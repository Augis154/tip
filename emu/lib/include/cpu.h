#include <stdio.h>
#include "spec.h"

struct cpu {
    uint8_t *memory;
    uint32_t memory_size;
    uint32_t pc;
    uint32_t R[32];
};

void init_cpu(struct cpu *cpu, uint32_t memory_size);
void free_cpu(struct cpu *cpu);
void step(struct cpu *cpu);