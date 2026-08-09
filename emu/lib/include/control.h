#ifndef CONTROL_H
#define CONTROL_H

#include "cpu.h"

void decode_control(struct cpu *cpu, uint32_t instruction, uint8_t fn4);
void decode_jump(struct cpu *cpu, uint32_t instruction, uint8_t fn4);

#endif
