#include<stdio.h>
#include"cpu.h"
int main()
{
    struct cpu cpu;
    init_cpu(&cpu, 4096*2);
    cpu.memory[0] = 0b00000001;
    cpu.memory[1] = 0b00000010;
    uint16_t memory_ptr = *(uint16_t *)(cpu.memory);
    printf("Hello World!! %u\n", memory_ptr);
    free_cpu(&cpu);
    return 0;
}