#include "spec.h"
#include "alu.h"
#include "dataop.h"
#include "control.h"
#include "cpu.h"
#ifdef DEBUG
#include "debug.h"
#endif
#include <stdlib.h>

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
    cpu->pc = RESET_VECTOR;
    cpu->memory = malloc(sizeof(uint8_t) * memory_size + 4); // +4 to avoid out of bounds access when fetching instructions

    if (cpu->memory == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }
    
    cpu->memory_size = memory_size;
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
    return *(uint16_t *)(cpu->memory + address);
}

uint32_t get_word(struct cpu *cpu, uint32_t address)
{
    if (address >= cpu->memory_size - 3) {
        fprintf(stderr, "Address out of bounds: %u\n", address);
        exit(1);
    }
    return *(uint32_t *)(cpu->memory + address);
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

void write_register(struct cpu *cpu, uint8_t reg_num, uint32_t value)
{
    if (reg_num >= 32) {
        fprintf(stderr, "Register number out of bounds: %u\n", reg_num);
        exit(1);
    }
    cpu->R[reg_num] = value;
}

static void (*instruction_decoders[0xF])(struct cpu *cpu, uint32_t instruction, uint8_t fn4) = {
    [TAG_ALU_REG] = decode_alu_reg,
    [TAG_ALU_IMM] = decode_alu_imm,
    [TAG_LOAD] = decode_load,
    [TAG_STORE] = decode_store,
    [TAG_CTRL] = decode_control,
    [TAG_UPPER] = decode_upper,
    // Placeholder for instruction decoders
};

void step(struct cpu *cpu)
{
    // Fetch instruction
    uint32_t instruction = get_word(cpu, cpu->pc);
    uint8_t tag4 = (instruction >> POS_TAG) & MASK_TAG;
    uint8_t fn4 = (instruction >> POS_FN) & MASK_FN;
    
    #ifdef DEBUG
    print_instr(cpu, instruction);
    #endif

    void (*decoder)(struct cpu *, uint32_t, uint8_t) = instruction_decoders[tag4];
    if (decoder) {
        decoder(cpu, instruction, fn4);
    }
    
    cpu->pc += 4; // Increment PC by 4 for the next instruction
}
