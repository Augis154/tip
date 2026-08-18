#ifndef DATAOP_H
#define DATAOP_H
#include <stdlib.h>
#include "cpu.h"

int decode_load(struct cpu *cpu, struct Instruction *instr);
int decode_store(struct cpu *cpu, struct Instruction *instr);
int decode_upper(struct cpu *cpu, struct Instruction *instr);

#endif
