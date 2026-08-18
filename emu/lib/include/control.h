#ifndef CONTROL_H
#define CONTROL_H

#include "cpu.h"

int decode_control(struct cpu *cpu, struct Instruction *instr);
int decode_jump(struct cpu *cpu, struct Instruction *instr);

#endif
