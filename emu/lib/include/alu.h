#ifndef ALU_H
#define ALU_H
#include "cpu.h"

int decode_alu_reg(struct cpu *cpu, struct Instruction *instr);
int decode_alu_imm(struct cpu *cpu, struct Instruction *instr);
#endif
