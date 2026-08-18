#include <stdlib.h>
#include "dataop.h"
#include "cpu.h"

#define LOAD_OP(match, size, check) \
case match: \
    { \
    uint32_t value = get_##size(cpu, address); \
    write_register(cpu, instr->operands.iType.rd, value & check ? value | ~((uint32_t)(check | ~(check))) : value); \
    } \
    break;

int decode_load(struct cpu *cpu, struct Instruction *instr)
{
    decode_instr_operands(instr, I);
    int16_t simm = (int16_t)instr->operands.iType.imm13;
    uint32_t address = get_register(cpu, instr->operands.iType.r1) + simm;
    switch (instr->fn) {
        LOAD_OP(MEM_BYTE, byte, (uint8_t)0x80)
        LOAD_OP(MEM_HALF, halfword, (uint16_t)0x8000)
        LOAD_OP(MEM_WORD, word, 0)
        LOAD_OP(MEM_BYTE_U, byte, 0)
        LOAD_OP(MEM_HALF_U, halfword, 0)
        default:
            fprintf(stderr, "Unknown load function: %u\n", instr->fn);
            return ILL_INSTR;
    }
    return OK;
}

int decode_store(struct cpu *cpu, struct Instruction *instr)
{
    decode_instr_operands(instr, S);
    int16_t simm = instr->operands.sType.imm;
    switch (instr->fn) {
        case MEM_BYTE:
            *(uint8_t *)(cpu->memory + get_register(cpu, instr->operands.sType.r1) + simm) = (uint8_t)get_register(cpu, instr->operands.sType.r2);
            break;
        case MEM_HALF:
            *(uint16_t *)(cpu->memory + get_register(cpu, instr->operands.sType.r1) + simm) = (uint16_t)get_register(cpu, instr->operands.sType.r2);
            break;
        case MEM_WORD:
            *(uint32_t *)(cpu->memory + get_register(cpu, instr->operands.sType.r1) + simm) = (uint32_t)get_register(cpu, instr->operands.sType.r2);
            break;
        default:
            fprintf(stderr, "Unknown store function: %u\n", instr->fn);
            return ILL_INSTR;
    }
    return OK;
}

int decode_upper(struct cpu *cpu, struct Instruction *instr)
{
    decode_instr_operands(instr, U);
    switch (instr->fn) {
        case UPPER_LUI:
            write_register(cpu, instr->operands.uType.rd, instr->operands.uType.imm18 << 13);
            break;
        case UPPER_LUPC:
            write_register(cpu, instr->operands.uType.rd, cpu->pc + (instr->operands.uType.imm18 << 13));
            break;
        default:
            fprintf(stderr, "Unknown upper function: %u\n", instr->fn);
            return ILL_INSTR;
    }
    return OK;
}
