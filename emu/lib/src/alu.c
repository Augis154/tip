#include "cpu.h"
#include "spec.h"
#include "alu.h"
#include <stdlib.h>
#include <stdio.h>

#define ALU_SIMPLE_MATCH(alu_op, maths_op) \
    case alu_op: \
        write_register(cpu, rd, maths_op); \
        break;

#define ALU_DIV_MATCH(alu_op, maths_op) \
    case alu_op: \
        if (rhs == 0) { \
            fprintf(stderr, "Division by zero in ALU function: "#alu_op"\n"); \
            return ERROR; \
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
            return ILL_INSTR;
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
            return ILL_INSTR;
        }
        break;
    default:
        fprintf(stderr, "Unknown ALU function: %u\n", fn4);
        return ILL_INSTR;
    }
    return OK;
}

int decode_alu_reg(struct cpu *cpu, struct Instruction *instr)
{
    decode_instr_operands(instr, R);
    if(instr->operands.rType.rd == 0)
        return OK;
    int retval = alu_op(cpu, instr->fn, instr->operands.rType.fn9, get_register(cpu, instr->operands.rType.r1), get_register(cpu, instr->operands.rType.r2), instr->operands.rType.rd);
    if(retval)
        trap(cpu, STC_ALU_ERROR, instr);
    return retval;
}

int decode_alu_imm(struct cpu *cpu, struct Instruction *instr)
{
    if(instr->fn == ALU_SHIFT) {
        decode_instr_operands(instr, IS);
        if(instr->operands.iType.rd == 0)
            return OK;
        int retval = alu_op(cpu, instr->fn, instr->operands.isType.fn9, get_register(cpu, instr->operands.isType.r1), get_register(cpu, instr->operands.isType.shamt5), instr->operands.isType.rd);
        if(retval)
            trap(cpu, STC_ALU_ERROR, instr);
        return retval;
    }
    else {
        decode_instr_operands(instr, I);
        if(instr->operands.iType.rd == 0)
            return OK;
        int retval = alu_op(cpu, instr->fn, 0, get_register(cpu, instr->operands.iType.r1), instr->operands.iType.imm13, instr->operands.iType.rd);
        if(retval)
            trap(cpu, STC_ALU_ERROR, instr);
        return retval;
    }
}
