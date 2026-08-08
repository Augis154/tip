#include<stdio.h>
#include"cpu.h"
int main()
{
    struct cpu cpu;
    init_cpu(&cpu, 4096*2);
    cpu.R[1] = 5;
    cpu.R[2] = 10;
    *(uint32_t *)(cpu.memory + RESET_VECTOR) = 0b00000000000010000010001100000000;
    step(&cpu);
    // uint32_t memory_ptr = *(uint32_t *)(cpu.memory + RESET_VECTOR);
    // printf("Hello World!! %u\n", memory_ptr);
    printf("Hello World!! %u\n", cpu.R[3]);
    free_cpu(&cpu);
    return 0;
}
