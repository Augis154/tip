#include "control.h"
#include "cpu.h"
#include <stdlib.h>

#define CONTROL_OP(opc, compare, val) \
case opc: \
    if (get_register(cpu, r1) compare get_register(cpu, r2)) \
        cpu->pc += val << 2; \
    break;

void decode_control(struct cpu *cpu, uint32_t instruction, uint8_t fn4){
    if (fn4 == CTRL_JAS || fn4 == CTRL_JASR){
        decode_jump(cpu, instruction, fn4);
        return;
    }
    uint8_t r1 = (instruction >> POS_R1) & MASK_REG;
    uint8_t r2 = (instruction >> POS_R2) & MASK_REG;
    uint8_t imm4_0 = (instruction >> POS_IMM4_0) & MASK_IMM4_0;
    uint16_t imm13_5 = (instruction >> POS_IMM13_5) & MASK_IMM13_5;
    uint16_t imm = (imm13_5 << 5) | imm4_0;
    int16_t simm = imm & 0x200 ? 0xFE00 | imm : imm;
    switch (fn4) {
        CONTROL_OP(CTRL_EQ, ==, simm)
        CONTROL_OP(CTRL_NE, !=, simm)
        CONTROL_OP(CTRL_LT, <, simm)
        CONTROL_OP(CTRL_GE, >=, simm)
        CONTROL_OP(CTRL_LTU, <, imm)
        CONTROL_OP(CTRL_GEU, >=, imm)
        default:
            fprintf(stderr, "Unknown control function: %u\n", fn4);
            exit(1);            
    }
}

void decode_jump(struct cpu *cpu, uint32_t instruction, uint8_t fn4){
    switch (fn4) {
        case CTRL_JAS:
            {
            uint8_t rd = (instruction >> POS_RD) & MASK_REG;
            uint32_t imm = (instruction >> POS_IMM19) & MASK_IMM19;
            write_register(cpu, rd, cpu->pc + 4);
            cpu->pc += (imm << 2) - 4;
            }
            break;
        case CTRL_JASR:
            {
            uint8_t rd = (instruction >> POS_RD) & MASK_REG;
            uint32_t r1 = get_register(cpu, (instruction >> POS_R1) & MASK_REG);
            uint16_t imm = (instruction >> POS_IMM14) & MASK_IMM14;
            write_register(cpu, rd, cpu->pc + 4);
            cpu->pc = r1 + imm - 4;
            }
            break;
        default:
            fprintf(stderr, "Unknown jump function: %u\n", fn4);
            exit(1);            
    }
}
