#ifndef ALU_H
#define ALU_H
#include "cpu.h"

void decode_alu_reg(struct cpu *cpu, uint32_t instruction, uint8_t fn4);
void decode_alu_imm(struct cpu *cpu, uint32_t instruction, uint8_t fn4);

#endif
