#include<stdio.h>
#include"cpu.h"
#include"spec.h"

#define R_TYPE(tag, fn4, rd, r1, r2, fn9) tag | (fn4 << POS_FN) | (rd << POS_RD) | (r1 << POS_R1) | (r2 << POS_R2) | (fn9 << POS_FN9)
#define I_TYPE(tag, fn4, rd, r1, imm) tag | (fn4 << POS_FN) | (rd << POS_RD) | (r1 << POS_R1) | (imm << POS_IMM14)
#define IS_TYPE(tag, fn4, rd, r1, shamt, fn9) tag | (fn4 << POS_FN) | (rd << POS_RD) | (r1 << POS_R1) | (shamt << POS_SHAMT) | (fn9 << POS_FN9)
#define S_TYPE(tag, fn4, imm, r1, r2) tag | (fn4 << POS_FN) | ((imm & MASK_IMM4_0) << POS_IMM4_0) | (r1 << POS_R1) | (r2 << POS_R2) | (((imm >> MASK_IMM4_0) & MASK_IMM13_5) << POS_IMM13_5)
#define U_TYPE(tag, fn4, rd, imm) tag | (fn4 << POS_FN) | (rd << POS_RD) | (imm << POS_IMM19)

uint32_t bootrom[0xFFC/4] = {
    I_TYPE(TAG_LOAD, MEM_BYTE, 1, 0, 15),
    I_TYPE(TAG_LOAD, MEM_HALF, 1, 0, 270),
    I_TYPE(TAG_LOAD, MEM_WORD, 1, 0, 270),
};

int main()
{
    struct cpu cpu;
    init_cpu(&cpu, 4096*2);
    FILE *file = fopen("../output.bin", "rb");
    fgets(cpu.memory, 4092, file);
    fclose(file);
    cpu.R[1] = 5;
    cpu.R[2] = 10;
    // *(uint32_t *)(cpu.memory + RESET_VECTOR) = 0b00000000000010000010001100000000;
    *(uint32_t *)(cpu.memory + RESET_VECTOR) = I_TYPE(TAG_CTRL, CTRL_JASR, 0, 0, 0);
    while(1)
        step(&cpu);
    // uint32_t memory_ptr = *(uint32_t *)(cpu.memory + RESET_VECTOR);
    // printf("Hello World!! %u\n", memory_ptr);
    printf("Hello World!! %u\n", cpu.pc);
    free_cpu(&cpu);
    return 0;
}
