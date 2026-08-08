#include "cpu.h"
#include "spec.h"
#include "alu.h"
#include <stdlib.h>
#include <stdio.h>

static int decode_alu_common(uint32_t instruction, uint8_t *rd, uint8_t *r1)
{
    *rd = (instruction >> POS_RD) & MASK_REG;
    if (*rd == 0) {
        return 0;
    }
    *r1 = (instruction >> POS_R1) & MASK_REG;
    return 1;
}

#define ALU_SIMPLE_MATCH(alu_op, maths_op) \
    case alu_op: \
        write_register(cpu, rd, maths_op); \
        break;

#define ALU_DIV_MATCH(alu_op, maths_op) \
    case alu_op: \
        if (rhs == 0) { \
            fprintf(stderr, "Division by zero in ALU function: "#alu_op); \
            exit(1); \
        } \
        write_register(cpu, rd, maths_op); \
        break;

static int alu_op(struct cpu *cpu, uint8_t fn4, uint16_t fn9, uint32_t lhs, uint32_t rhs, uint8_t rd)
{
    switch (fn4)
    {
    case ALU_ADD:
        if (fn9 == EXT_SUB) {
            write_register(cpu, rd, lhs - rhs);
        } else {
            write_register(cpu, rd, lhs + rhs);
        }
        break;
    ALU_SIMPLE_MATCH(ALU_AND, lhs & rhs)
    ALU_SIMPLE_MATCH(ALU_OR, lhs | rhs)
    ALU_SIMPLE_MATCH(ALU_XOR, lhs ^ rhs)
    
    case ALU_SHIFT:
        switch (fn9)
        {
        ALU_SIMPLE_MATCH(SHIFT_SLL, lhs << (rhs & 0x1F))
        ALU_SIMPLE_MATCH(SHIFT_SRL, lhs >> (rhs & 0x1F))
        ALU_SIMPLE_MATCH(SHIFT_SRA, (int32_t)lhs >> (rhs & 0x1F))
        default:
            fprintf(stderr, "Unknown ALU shift function: %u\n", fn9);
            exit(1);
        }
        break;
    
    ALU_SIMPLE_MATCH(ALU_SEQ, lhs == rhs ? 1 : 0)
    ALU_SIMPLE_MATCH(ALU_SNE, lhs != rhs ? 1 : 0)
    ALU_SIMPLE_MATCH(ALU_SLT, (int32_t)lhs < (int32_t)rhs ? 1 : 0)
    ALU_SIMPLE_MATCH(ALU_SLTU, lhs < rhs ? 1 : 0)
    
    case ALU_EXT:
        switch (fn9)
        {
        ALU_SIMPLE_MATCH(EXT_MUL, ((int32_t)lhs * (int32_t)rhs) & 0xFFFFFFFF);
        ALU_SIMPLE_MATCH(EXT_MULH, (((int64_t)lhs * (int64_t)rhs) >> 32) & 0xFFFFFFFF);
        ALU_SIMPLE_MATCH(EXT_MULHU, (((uint64_t)lhs * (uint64_t)rhs) >> 32) & 0xFFFFFFFF);

        ALU_DIV_MATCH(EXT_DIV, (int32_t)lhs / (int32_t)rhs)
        ALU_DIV_MATCH(EXT_DIVU, lhs / rhs)
        ALU_DIV_MATCH(EXT_REM, (int32_t)lhs % (int32_t)rhs)
        ALU_DIV_MATCH(EXT_REMU, lhs % rhs)

        default:
            fprintf(stderr, "Unknown ALU extension function: %u\n", fn9);
            exit(1);
        }
        break;
    default:
        fprintf(stderr, "Unknown ALU function: %u\n", fn4);
        exit(1);
    }
    return 0;
}

void decode_alu_reg(struct cpu *cpu, uint32_t instruction, uint8_t fn4)
{
    uint8_t rd, r1;
    if (!decode_alu_common(instruction, &rd, &r1)) {
        return;
    }
    uint8_t r2 = (instruction >> POS_R2) & MASK_REG;
    uint16_t fn9 = (instruction >> POS_FN9) & MASK_FN9;

    uint32_t r1val = get_register(cpu, r1);
    uint32_t r2val = get_register(cpu, r2);

    alu_op(cpu, fn4, fn9, r1val, r2val, rd);
}

void decode_alu_imm(struct cpu *cpu, uint32_t instruction, uint8_t fn4)
{
    uint8_t rd, r1;
    if (!decode_alu_common(instruction, &rd, &r1)) {
        return;
    }
    uint32_t imm = 0;
    uint16_t fn9 = 0;
    if(fn4 == ALU_SHIFT) {
        fn9 = (instruction >> POS_FN9) & MASK_FN9;
        imm = (instruction >> POS_SHAMT) & MASK_SHAMT;
    } else {
        imm = (instruction >> POS_IMM14) & MASK_IMM14;
    }
    uint32_t r1val = get_register(cpu, r1);

    alu_op(cpu, fn4, fn9, r1val, imm, rd);
}
