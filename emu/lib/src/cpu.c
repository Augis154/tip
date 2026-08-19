#include "spec.h"
#include "alu.h"
#include "dataop.h"
#include "control.h"
#include "sys.h"
#include "cpu.h"
#include "debug.h"
#include "formats.h"
#include <stdlib.h>
#include <string.h>

void init_cpu(struct cpu *cpu, uint32_t memory_size)
{
    if (memory_size <= 0) {
        fprintf(stderr, "Memory size must be greater than 0\n");
        exit(1);
    }
    if (memory_size < RESET_VECTOR) {
        fprintf(stderr, "Memory size must be greater than or equal to %d\n", RESET_VECTOR);
        exit(1);
    }
    cpu->memory = malloc(sizeof(uint8_t) * memory_size + 4); // +4 to avoid out of bounds access when fetching instructions

    if (cpu->memory == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }
    
    cpu->memory_size = memory_size;
    memset(cpu->system_register, 0, __SR_MAX*sizeof(uint32_t));
    memset(cpu->R, 0, 32*sizeof(uint32_t));
    cpu->pc = RESET_VECTOR;
    cpu->system_register[SR_STS] = 0x2;
    cpu->trap_occured = 0;
}

void free_cpu(struct cpu *cpu)
{
    free(cpu->memory);
}

uint8_t get_byte(struct cpu *cpu, uint32_t address)
{
    if (address >= cpu->memory_size) {
        fprintf(stderr, "Address out of bounds: %u\n", address);
        exit(1);
    }
    return *(uint8_t *)(cpu->memory + address);
}

uint16_t get_halfword(struct cpu *cpu, uint32_t address)
{
    if (address >= cpu->memory_size - 1) {
        fprintf(stderr, "Address out of bounds: %u\n", address);
        exit(1);
    }
    return  ((uint16_t)*(cpu->memory + address + 0))       |
            ((uint16_t)*(cpu->memory + address + 1) << 8);
}

uint32_t get_word(struct cpu *cpu, uint32_t address)
{
    if (address > cpu->memory_size - 4) {
        fprintf(stderr, "Address out of bounds: %u\n", address);
        exit(1);
    }
    return  ((uint32_t)*(cpu->memory + address + 0))       |
            ((uint32_t)*(cpu->memory + address + 1) << 8)  |
            ((uint32_t)*(cpu->memory + address + 2) << 16) |
            ((uint32_t)*(cpu->memory + address + 3) << 24);
}

uint32_t get_register(struct cpu *cpu, uint8_t reg_num)
{
    if (reg_num >= 32) {
        fprintf(stderr, "Register number out of bounds: %u\n", reg_num);
        exit(1);
    }
    if (reg_num == 0) {
        return 0; // Register x0 is always zero
    }
    return cpu->R[reg_num];
}

int write_register(struct cpu *cpu, uint8_t reg_num, uint32_t value)
{
    if (reg_num >= 32) {
        fprintf(stderr, "Register number out of bounds: %u\n", reg_num);
        return 0;
    }
    cpu->R[reg_num] = value;
    return 1;
}

static InstructionDecoder instruction_decoders[16] = {
    [TAG_ALU_REG] = decode_alu_reg,
    [TAG_ALU_IMM] = decode_alu_imm,
    [TAG_LOAD] = decode_load,
    [TAG_STORE] = decode_store,
    [TAG_CTRL] = decode_control,
    [TAG_UPPER] = decode_upper,
    [TAG_SYS] = decode_sys,
    // Placeholder for instruction decoders
};

void trap(struct cpu *cpu, uint32_t cause, struct Instruction *instr)
{
    if (!((cpu->system_register[SR_STS] >> POS_STS_SIE) & 1))
        return;
    cpu->system_register[SR_STPC] = cpu->pc;
    uint32_t *STS = &cpu->system_register[SR_STS];
    *STS &= ~((1 << POS_STS_PIE) | (1 << POS_STS_PMODE));
    *STS |= (*STS & (1 << POS_STS_SIE)) << (POS_STS_PIE - POS_STS_SIE);
    *STS |= (*STS & (1 << POS_STS_SMODE)) << (POS_STS_PMODE - POS_STS_SMODE);
    cpu->system_register[SR_STC] = cause;
    switch(cause) {
        case STC_INSTR_MISALIGNED:
        case STC_INSTR_FAULT:
            cpu->system_register[SR_STV] = cpu->pc;
            break;
        case STC_DATA_MISALIGNED:
        case STC_DATA_FAULT:
            switch(instr->fmt) {
                case I:
                    cpu->system_register[SR_STV] = get_register(cpu, instr->operands.iType.r1) + instr->operands.iType.imm13;
                    break;
                case S:
                    cpu->system_register[SR_STV] = get_register(cpu, instr->operands.sType.r1) + instr->operands.sType.imm;
                    break;
                default:
                    fprintf(stderr, "STC_DATA_FAULT in unsupported instruction\n");
                    exit(1);
                    break;
            }
            break;
        case STC_ILLEGAL_INSTR:
            cpu->system_register[SR_STV] = instr->raw;
            break;
        case STC_SCALL:
        case STC_SBREAK:
            cpu->system_register[SR_STV] = 0;
            break;
        case STC_ALU_ERROR:
            cpu->system_register[SR_STV] = instr->raw;
            break;
        default:
            fprintf(stderr, "Encountered trap with unknown cause %i\n", cause);
            break;
    }
    cpu->pc = cpu->system_register[SR_STH]-4;
    cpu->trap_occured = 1;
}

void step(struct cpu *cpu)
{
    struct Instruction instr = {0};
    cpu->trap_occured = 0;
    if(cpu->pc % 4)
        trap(cpu, STC_INSTR_MISALIGNED, &instr);

    // Fetch instruction
    instr.raw = get_word(cpu, cpu->pc);
    instr.tag = (instr.raw >> POS_TAG) & MASK_TAG;
    instr.fn = (instr.raw >> POS_FN) & MASK_FN;
    
    if (cpu->debug)
        print_instr(cpu, &instr);

    int (*decoder)(struct cpu *, struct Instruction *) = instruction_decoders[instr.tag];
    if (decoder) {
        decoder(cpu, &instr);
    } else {
        trap(cpu, STC_ILLEGAL_INSTR, &instr);
    }
    
    cpu->pc += 4; // Increment PC by 4 for the next instruction
    uint64_t cycles = (((uint64_t)cpu->system_register[SR_CYCLEH] << 32) | cpu->system_register[SR_CYCLE])+1;
    cpu->system_register[SR_CYCLE] = (uint32_t)cycles;
    cpu->system_register[SR_CYCLEH] = (uint32_t)(cycles >> 32);
}
