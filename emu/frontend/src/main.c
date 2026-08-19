#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "debugger.h"
#include "cpu.h"
#include "spec.h"

#define R_TYPE(tag, fn4, rd, r1, r2, fn9) tag | (fn4 << POS_FN) | (rd << POS_RD) | (r1 << POS_R1) | (r2 << POS_R2) | (fn9 << POS_FN9)
#define I_TYPE(tag, fn4, rd, r1, imm) tag | (fn4 << POS_FN) | (rd << POS_RD) | (r1 << POS_R1) | (imm << POS_IMM14)
#define IS_TYPE(tag, fn4, rd, r1, shamt, fn9) tag | (fn4 << POS_FN) | (rd << POS_RD) | (r1 << POS_R1) | (shamt << POS_SHAMT) | (fn9 << POS_FN9)
#define S_TYPE(tag, fn4, imm, r1, r2) tag | (fn4 << POS_FN) | ((imm & MASK_IMM4_0) << POS_IMM4_0) | (r1 << POS_R1) | (r2 << POS_R2) | (((imm >> 5) & MASK_IMM13_5) << POS_IMM13_5)
#define U_TYPE(tag, fn4, rd, imm) tag | (fn4 << POS_FN) | (rd << POS_RD) | (imm << POS_IMM19)

uint32_t bootrom[0xFFC/4] = {
    I_TYPE(TAG_ALU_IMM, ALU_ADD, 1, 0, 1),
    I_TYPE(TAG_SYS, SYS_SRS, 0, 1, SR_STS),
    I_TYPE(TAG_ALU_IMM, ALU_ADD, 1, 0, 5*sizeof(uint32_t)),
    I_TYPE(TAG_SYS, SYS_SRW, 0, 1, SR_STH),

    I_TYPE(TAG_CTRL, CTRL_JASR, 0, 0, 4096),

    S_TYPE(TAG_STORE, MEM_WORD, 8192, 0, 31),

    // S_TYPE(TAG_STORE, MEM_WORD, 8192, 0, 31),
    // S_TYPE(TAG_STORE, MEM_WORD, 8192, 0, 31),
    // S_TYPE(TAG_STORE, MEM_WORD, 8192, 0, 31),
    // S_TYPE(TAG_STORE, MEM_WORD, 8192, 0, 31),
    // S_TYPE(TAG_STORE, MEM_WORD, 8192, 0, 31),
    // S_TYPE(TAG_STORE, MEM_WORD, 8192, 0, 31),
};

int execState = 0;

void sigHandler(int signum){
    if(signum == SIGINT) {
        if(execState < 1)
            execState++;
        else
            exit(1);
        fprintf(stderr, "Execution paused.\n");
    }
}

static void load_file_into_memory(const char* filename, struct cpu *cpu, uint32_t size, uint32_t offset) {
    FILE *file = fopen(filename, "rb");
    if(file){
        fread(cpu->memory + offset, 1, size, file);
        fclose(file);
    }
}

const int MEM_MAX = 0x1030;

int main()
{
    struct cpu cpu;
    init_cpu(&cpu, MEM_MAX);
    #ifdef DEBUG
    cpu.debug = 1;
    #endif
    char *dbgval;
    if ((dbgval = getenv("DEBUG"))) {
        cpu.debug = 1;
        execState = atoi(dbgval);
    }
    load_file_into_memory("../rom.bin", &cpu, MEM_MAX, 0x0);
    load_file_into_memory("../bootrom.bin", &cpu, 4096, 0x0);
    // *(uint32_t *)(cpu.memory + RESET_VECTOR) = I_TYPE(TAG_CTRL, CTRL_JASR, 0, 0, 0);
    signal(SIGINT, sigHandler); 
    while(1) {
        if(execState == 0 && !cpu.trap_occured) {
            step(&cpu);
            continue;            
        }
        if(cpu.trap_occured)
            execState = 1;
        handle_debug(&cpu, &execState);
    }
    // uint32_t memory_ptr = *(uint32_t *)(cpu.memory + RESET_VECTOR);
    // printf("Hello World!! %u\n", memory_ptr);
    printf("Hello World!! %u\n", cpu.pc);
    free_cpu(&cpu);
    return 0;
}
