#ifndef CPU_H
#define CPU_H
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
uint8_t get_byte(struct cpu *cpu, uint32_t address);
uint16_t get_halfword(struct cpu *cpu, uint32_t address);
uint32_t get_word(struct cpu *cpu, uint32_t address);
uint32_t get_register(struct cpu *cpu, uint8_t reg_num);
void write_register(struct cpu *cpu, uint8_t reg_num, uint32_t value);
uint32_t get_register(struct cpu *cpu, uint8_t reg_num);

#endif
