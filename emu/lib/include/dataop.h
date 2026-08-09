#ifndef DATAOP_H
#define DATAOP_H
#include <stdlib.h>
#include "cpu.h"

void decode_load(struct cpu *cpu, uint32_t instruction, uint8_t fn4);
void decode_store(struct cpu *cpu, uint32_t instruction, uint8_t fn4);
void decode_upper(struct cpu *cpu, uint32_t instruction, uint8_t fn4);

#endif
