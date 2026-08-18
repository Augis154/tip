#include "spec.h"
#include "formats.h"

void decode_instr_operands(struct Instruction *instr, OperandFormat fmt) {
    instr->fmt = fmt;
    switch(fmt) {
        case R:
        {
            struct R_Type *ops = &instr->operands.rType;
            ops->rd = (instr->raw >> POS_RD) & MASK_REG;
            ops->r1 = (instr->raw >> POS_R1) & MASK_REG;
            ops->r2 = (instr->raw >> POS_R2) & MASK_REG;
            ops->fn9 = (instr->raw >> POS_FN9) & MASK_FN9;
        }
            break;
        case I:
        {
            struct I_Type *ops = &instr->operands.iType;
            ops->rd = (instr->raw >> POS_RD) & MASK_REG;
            ops->r1 = (instr->raw >> POS_R1) & MASK_REG;
            ops->imm13 = (instr->raw >> POS_IMM14) & MASK_IMM14;
        }
            break;
        case IS:
        {
            struct IS_Type *ops = &instr->operands.isType;
            ops->rd = (instr->raw >> POS_RD) & MASK_REG;
            ops->r1 = (instr->raw >> POS_R1) & MASK_REG;
            ops->shamt5 = (instr->raw >> POS_SHAMT) & MASK_SHAMT;
            ops->fn9 = (instr->raw >> POS_FN9) & MASK_FN9;
        }
            break;
        case S:
        {
            struct S_Type *ops = &instr->operands.sType;
            ops->r1 = (instr->raw >> POS_R1) & MASK_REG;
            ops->r2 = (instr->raw >> POS_R2) & MASK_REG;
            uint8_t imm4_0 = (instr->raw >> POS_IMM4_0) & MASK_IMM4_0;
            uint16_t imm13_5 = (instr->raw >> POS_IMM13_5) & MASK_IMM13_5;
            uint16_t imm = (imm13_5 << 5) | imm4_0;
            ops->imm = imm & 0x200 ? 0xFE00 | imm : imm;

        }
            break;
        case U:
        {
            struct U_Type *ops = &instr->operands.uType;
            ops->rd = (instr->raw >> POS_RD) & MASK_REG;
            ops->imm18 = (instr->raw >> POS_IMM19) & MASK_IMM19;
        }
            break;
    }
}
