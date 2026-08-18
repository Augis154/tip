#include "control.h"
#include "cpu.h"
#include <stdlib.h>

#define CONTROL_OP(opc, compare, val) \
case opc: \
    if (get_register(cpu, instr->operands.sType.r1) compare get_register(cpu, instr->operands.sType.r2)) \
        cpu->pc += (val << 2) - 4; \
    break;

int decode_control(struct cpu *cpu, struct Instruction *instr){
    if (instr->fn == CTRL_JAS || instr->fn  == CTRL_JASR){
        return decode_jump(cpu, instr);
    }
    decode_instr_operands(instr, S);
    switch (instr->fn) {
        CONTROL_OP(CTRL_EQ, ==, (int16_t)instr->operands.sType.imm)
        CONTROL_OP(CTRL_NE, !=, (int16_t)instr->operands.sType.imm)
        CONTROL_OP(CTRL_LT, <, (int16_t)instr->operands.sType.imm)
        CONTROL_OP(CTRL_GE, >=, (int16_t)instr->operands.sType.imm)
        CONTROL_OP(CTRL_LTU, <, instr->operands.sType.imm)
        CONTROL_OP(CTRL_GEU, >=, instr->operands.sType.imm)
        default:
            return ILL_INSTR;
            fprintf(stderr, "Unknown control function: %u\n", instr->fn);
            exit(1);            
    }
    return OK;
}

int decode_jump(struct cpu *cpu, struct Instruction *instr){
    switch (instr->fn) {
        case CTRL_JAS:
            {
            decode_instr_operands(instr, U);
            write_register(cpu, instr->operands.uType.rd, cpu->pc + 4);
            cpu->pc += (instr->operands.uType.imm18 << 2) - 4;
            }
            break;
        case CTRL_JASR:
            {
            decode_instr_operands(instr, I);
            uint32_t r1 = get_register(cpu, instr->operands.iType.r1);
            write_register(cpu, instr->operands.iType.rd, cpu->pc + 4);
            cpu->pc = r1 + instr->operands.iType.imm13 - 4;
            }
            break;
        default:
            fprintf(stderr, "Unknown jump function: %u\n", instr->fn);
            return ILL_INSTR;
    }
    return OK;
}
