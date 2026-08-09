#include <stdlib.h>
#include "dataop.h"
#include "cpu.h"

#define LOAD_OP(match, size, check) \
case match: \
    { \
    uint32_t value = get_##size(cpu, address); \
    write_register(cpu, rd, value & check ? value | ~((uint32_t)(check | ~(check))) : value); \
    } \
    break;

void decode_load(struct cpu *cpu, uint32_t instruction, uint8_t fn4)
{
    uint8_t rd = (instruction >> POS_RD) & MASK_REG;
    uint8_t r1 = (instruction >> POS_R1) & MASK_REG;
    int16_t simm = (instruction >> POS_IMM14) & MASK_IMM14;
    uint32_t address = get_register(cpu, r1) + simm;
    switch (fn4) {
        LOAD_OP(MEM_BYTE, byte, (uint8_t)0x80)
        LOAD_OP(MEM_HALF, halfword, (uint16_t)0x8000)
        LOAD_OP(MEM_WORD, word, 0)
        LOAD_OP(MEM_BYTE_U, byte, 0)
        LOAD_OP(MEM_HALF_U, halfword, 0)
        default:
            fprintf(stderr, "Unknown load function: %u\n", fn4);
            exit(1);
    }
}

void decode_store(struct cpu *cpu, uint32_t instruction, uint8_t fn4)
{
    uint8_t imm4_0 = (instruction >> POS_IMM4_0) & MASK_IMM4_0;
    uint8_t r1 = (instruction >> POS_R1) & MASK_REG;
    uint8_t r2 = (instruction >> POS_R2) & MASK_REG;
    uint16_t imm13_5 = (instruction >> POS_IMM13_5) & MASK_IMM13_5;
    int16_t simm = (imm13_5 << 5) | imm4_0;
    simm = simm & 0x200 ? 0xFE00 | simm : simm;
    switch (fn4) {
        case MEM_BYTE:
            *(uint8_t *)(cpu->memory + get_register(cpu, r1) + simm) = (uint8_t)get_register(cpu, r2);
            break;
        case MEM_HALF:
            *(uint16_t *)(cpu->memory + get_register(cpu, r1) + simm) = (uint16_t)get_register(cpu, r2);
            break;
        case MEM_WORD:
            *(uint32_t *)(cpu->memory + get_register(cpu, r1) + simm) = (uint32_t)get_register(cpu, r2);
            break;
        default:
            fprintf(stderr, "Unknown store function: %u\n", fn4);
            exit(1);
    }
}

void decode_upper(struct cpu *cpu, uint32_t instruction, uint8_t fn4)
{
    uint8_t rd = (instruction >> POS_RD) & MASK_REG;
    uint32_t imm = (instruction >> POS_IMM19) & MASK_IMM19;
    switch (fn4) {
        case UPPER_LUI:
            write_register(cpu, rd, imm << 13);
            break;
        case UPPER_LUPC:
            write_register(cpu, rd, cpu->pc + (imm << 13));
            break;
        default:
            fprintf(stderr, "Unknown upper function: %u\n", fn4);
            exit(1);
    }
}
