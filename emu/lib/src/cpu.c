#include "spec.h"
#include "cpu.h"
#include <stdlib.h>

void init_cpu(struct cpu *cpu, uint32_t memory_size)
{
    if (memory_size <= 0) {
        fprintf(stderr, "Memory size must be greater than 0\n");
        exit(1);
    }
    if (memory_size < RESET_VECTOR) {
        fprintf(stderr, "Memory size must be greater than or equal to %d\n", RESET_VECTOR);
        exit(1);
    }
    cpu->pc = RESET_VECTOR;
    cpu->memory = malloc(sizeof(uint8_t) * memory_size + 4); // +4 to avoid out of bounds access when fetching instructions

    if (cpu->memory == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }
    
    cpu->memory_size = memory_size;
}

void free_cpu(struct cpu *cpu)
{
    free(cpu->memory);
}

static uint8_t get_byte(struct cpu *cpu, uint32_t address)
{
    if (address >= cpu->memory_size || address < 0) {
        fprintf(stderr, "Address out of bounds: %u\n", address);
        exit(1);
    }
    return *(uint8_t *)(cpu->memory + address);
}

static uint16_t get_halfword(struct cpu *cpu, uint32_t address)
{
    if (address >= cpu->memory_size - 1 || address < 0) {
        fprintf(stderr, "Address out of bounds: %u\n", address);
        exit(1);
    }
    return *(uint16_t *)(cpu->memory + address);
}

static uint32_t get_word(struct cpu *cpu, uint32_t address)
{
    if (address >= cpu->memory_size - 3 || address < 0) {
        fprintf(stderr, "Address out of bounds: %u\n", address);
        exit(1);
    }
    return *(uint32_t *)(cpu->memory + address);
}

static uint32_t get_register(struct cpu *cpu, uint8_t reg_num)
{
    if (reg_num >= 32 || reg_num < 0) {
        fprintf(stderr, "Register number out of bounds: %u\n", reg_num);
        exit(1);
    }
    if (reg_num == 0) {
        return 0; // Register x0 is always zero
    }
    return cpu->R[reg_num];
}

static void write_register(struct cpu *cpu, uint8_t reg_num, uint32_t value)
{
    if (reg_num >= 32 || reg_num < 0) {
        fprintf(stderr, "Register number out of bounds: %u\n", reg_num);
        exit(1);
    }
    cpu->R[reg_num] = value;
}

static int decode_alu_common(struct cpu *cpu, uint32_t instruction, uint8_t *rd, uint8_t *r1)
{
    *rd = (instruction >> POS_RD) & MASK_REG;
    if (*rd == 0) {
        return 0;
    }
    *r1 = (instruction >> POS_R1) & MASK_REG;
    return 1;
}

static void decode_alu_reg(struct cpu *cpu, uint32_t instruction, uint8_t fn4)
{
    uint8_t rd, r1;
    if (!decode_alu_common(cpu, instruction, &rd, &r1)) {
        return;
    }
    uint8_t r2 = (instruction >> POS_R2) & MASK_REG;
    uint16_t fn9 = (instruction >> POS_FN9) & MASK_FN9;
    
    switch (fn4)
    {
    case ALU_ADD:
        if (fn9 == EXT_SUB) {
            write_register(cpu, rd, get_register(cpu, r1) - get_register(cpu, r2));
        } else {
            write_register(cpu, rd, get_register(cpu, r1) + get_register(cpu, r2));
        }
        break;
    case ALU_AND:
        write_register(cpu, rd, get_register(cpu, r1) & get_register(cpu, r2));
        break;
    default:
        break;
    }
}

static void (*instruction_decoders[0xF])(struct cpu *cpu, uint32_t instruction, uint8_t fn4) = {
    [TAG_ALU_REG] = decode_alu_reg,
    // Placeholder for instruction decoders
};



void step(struct cpu *cpu)
{
    // Fetch instruction
    uint32_t instruction = get_word(cpu, cpu->pc);
    uint8_t tag4 = (instruction >> POS_TAG) & MASK_TAG;
    uint8_t fn4 = (instruction >> POS_FN) & MASK_FN;
    cpu->pc += 4; // Increment PC by 4 for the next instruction
    
    void (*decoder)(struct cpu *, uint32_t, uint8_t) = instruction_decoders[tag4];
    if (decoder) {
        decoder(cpu, instruction, fn4);
    }
    // Decode and execute instruction
    // This is a placeholder for the actual implementation of the CPU step function.
    // The actual implementation would involve decoding the instruction and performing the appropriate operation.

}