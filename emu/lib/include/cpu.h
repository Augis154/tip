#ifndef CPU_H
#define CPU_H
#include <stdio.h>
#include "spec.h"
#include "formats.h"

typedef enum {
    OK,
    ERROR,
    ILL_INSTR
} RetVal;
struct cpu {
    uint8_t *memory;
    uint32_t memory_size;
    uint32_t pc;
    uint32_t R[32];
    uint8_t debug;
    uint8_t trap_occured;
    uint32_t system_register[__SR_MAX];
};

typedef int (*InstructionDecoder)(struct cpu *cpu, struct Instruction *instr);

void init_cpu(struct cpu *cpu, uint32_t memory_size);
void free_cpu(struct cpu *cpu);
void trap(struct cpu *cpu, uint32_t cause, struct Instruction *instr);
void step(struct cpu *cpu);
uint8_t get_byte(struct cpu *cpu, uint32_t address);
uint16_t get_halfword(struct cpu *cpu, uint32_t address);
uint32_t get_word(struct cpu *cpu, uint32_t address);
uint32_t get_register(struct cpu *cpu, uint8_t reg_num);
int write_register(struct cpu *cpu, uint8_t reg_num, uint32_t value);
uint32_t get_register(struct cpu *cpu, uint8_t reg_num);

#endif
