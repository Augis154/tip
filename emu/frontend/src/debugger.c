#include "debugger.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "cpu.h"

#define CMD(name) strstr(command, name) == command
#define CMD_ARGS(name, handler) cmd_with_args(command, cpu, name, handler)
#define BOOL_VARIANT(bool, orig, variant) if(bool) variant else orig

typedef int (*CmdHandler)(struct cpu *cpu, char *argv);

typedef enum {
    INVALID = -1,
    UINT, INT, HEX, BIN
} NumPrintFmt;

void format_number_long(char *dest, int size, uint64_t num, NumPrintFmt fmt, int long_mode) {
    switch (fmt)
    {
    case UINT:
        BOOL_VARIANT(long_mode, snprintf(dest, size, "%u", (uint32_t)num);, snprintf(dest, size, "%lu", num);)
        break;
    case INT:
        BOOL_VARIANT(long_mode, snprintf(dest, size, "%i", (uint32_t)num);, snprintf(dest, size, "%li", num);)
        break;
    case HEX:
        BOOL_VARIANT(long_mode, snprintf(dest, size, "%x", (uint32_t)num);, snprintf(dest, size, "%lx", num);)
        break;

    case BIN:
        BOOL_VARIANT(long_mode, snprintf(dest, size, "0b%032b", (uint32_t)num);, snprintf(dest, size, "0b%064lb", num);)
        break;
    default:
        fprintf(stderr, "Invalid format given for number formatting.");
        exit(1);
        break;
    }
}

void format_number(char *dest, int size, uint32_t num, NumPrintFmt fmt) {
    format_number_long(dest, size, (uint64_t) num, fmt, 0);
}

NumPrintFmt getFormat(char fmtLetter) {
    switch(fmtLetter) {
        case 'u':
            return UINT;
        case 'i':
            return INT;
        case 'x':
            return HEX;
        case 'b':
            return BIN;
        break;
        case '\0':
            return INT;
        default:
            return INVALID;
    }
};

int _write_register(struct cpu *cpu, char *argv){
    char *val = strstr(argv, " ");
    if (val) {
        val[0] = '\0';
        val++;
    } else {
        fprintf(stderr, "Not enough arguments. Usage: 'wr <register number> <register value>\n");
        return 1;
    }
    int regnum = atoi(argv);
    int regval = atoi(val);
    return write_register(cpu, regnum, regval);
}

int _get_register(struct cpu *cpu, char *argv){
    NumPrintFmt fmt = getFormat(argv[0]);
    if(fmt == INVALID) {
        fprintf(stderr, "Incorrect argument #1. Usage: 'gr <u|i|x|b> [register number]\n");
        return 1;
    }

    char *val = strstr(argv, " ");
    char strval[256];
    if (val) {
        int register_number = atoi(val+1);
        format_number(strval, 256, get_register(cpu, (uint8_t)register_number), fmt);
        fprintf(stderr, "R%i: %s\n", register_number, strval);
        return 1;
    }
    for(int i = 0; i < 32; i++) {
        format_number(strval, 256, cpu->R[i], fmt);
        fprintf(stderr, "R%i: %s\n", i, strval);
    }
    return 1;
}

#define ARG_MATCH(src, name, dst, val) else if(strcmp(src, name) == 0) dst = val

int _get_sysreg(struct cpu *cpu, char *argv) {
    NumPrintFmt fmt = getFormat(argv[0]);
    if(fmt == INVALID) {
        fprintf(stderr, "Incorrect argument #1. Usage: 'sgr <u|i|x|b> [register name]\n");
_get_sysreg_help:
        fprintf(stderr, "Register names:\n");
        fprintf(stderr, "- cycle\n");
        fprintf(stderr, "- cycleh\n");
        fprintf(stderr, "- sts\n");
        fprintf(stderr, "- sth\n");
        fprintf(stderr, "- stpc\n");
        fprintf(stderr, "- stc\n");
        fprintf(stderr, "- stv\n");
        return 1;
    }
    char *reg = strstr(argv, " ");
    if (reg)
        reg++;
    else {
        fprintf(stderr, "No register provided. Usage: 'sgr <u|i|x|b> [register name]\n");
        goto _get_sysreg_help;
    }
    char *next = strstr(reg, " ");
    if(next)
        *next = 0;
    int position = -1;
    if(1 == 0) {}
    ARG_MATCH(reg, "cycle", position, SR_CYCLE);
    ARG_MATCH(reg, "cycleh", position, SR_CYCLEH);
    ARG_MATCH(reg, "sts", position, SR_STS);
    ARG_MATCH(reg, "sth", position, SR_STH);
    ARG_MATCH(reg, "stpc", position, SR_STPC);
    ARG_MATCH(reg, "stc", position, SR_STC);
    ARG_MATCH(reg, "stv", position, SR_STV);
    else {
        goto _get_sysreg_help;
    }
    char strval[256];
    if(position == SR_CYCLE) {
        uint64_t cycles = ((uint64_t)cpu->system_register[SR_CYCLEH] << 32) | cpu->system_register[SR_CYCLE];
        format_number_long(strval, 256, cycles, fmt, 1);
    } else {
        format_number(strval, 256, cpu->system_register[position], fmt);
    }
    fprintf(stderr, "%s: %s\n", reg, strval);
    return 1;
}

int _read_mem(struct cpu *cpu, char *argv) {
    uint32_t count, start_addr, width = 8;
    uint32_t *params[3] = { &start_addr, &count, &width };
    char *pos = argv;
    for(int i = 0; i < 3 && pos; i++){
        char *end = strstr(pos, " ");
        if(end) *end = 0;
        *params[i] = (uint32_t)strtoul(pos, NULL, 0);
        if(end)
            pos = end + 1;
        else
            pos = 0;
    }   
    if(start_addr > cpu->memory_size) {
        fprintf(stderr, "Memory read start out of bounds. Start address: %x, memory size: %x", start_addr, cpu->memory_size);
        return 1;
    }
    if(start_addr + (count - 1) > cpu->memory_size) {
        fprintf(stderr, "Memory read out of bounds. End address: %x, memory size: %x", start_addr + count, cpu->memory_size);
        return 1;
    }

    for(uint32_t i = 0; i < count; i++) {
        if(i % width == 0)
            fprintf(stderr, "\n%08x: ", start_addr + i);

        fprintf(stderr, "%02hx", cpu->memory[(start_addr) + i]);
        if(i % 2)
            fprintf(stderr, " ");
    }
    fprintf(stderr, "\n");
    return 1;
}

int cmd_with_args(char *input, struct cpu *cpu, char *name, CmdHandler handler){
    if (strstr(input, name) == input){
        int n = strlen(name);
        switch(input[n]) {
            case ' ':
                return handler(cpu, input + n + 1);
            case '\0':
            case '\n':
                return handler(cpu, input + n);
            default:
                return 0;

        }
        return 0;
    }
    return 0;
}

void handle_debug(struct cpu *cpu, int *state) {
    cpu->debug = 1;
    char command[256];
    memset(command, 0, 256);
    fprintf(stderr, "> ");
    fgets(command, 256, stdin);
    for(int i = 0; command[i]; i++){
        command[i] = tolower(command[i]);
        if(command[i] == '\n')
            command[i] = 0;
    }
    if(CMD("step")) {
        step(cpu);
    }
    else if(CMD("help")){
        fprintf(stderr, "Supported commands: \n");
        fprintf(stderr, "- step \n");
        fprintf(stderr, "- run \n");
        fprintf(stderr, "- gr <u|i|x|b> [register number] \n");
        fprintf(stderr, "- wr [register number] [register value] \n");
        fprintf(stderr, "- sgr <u|i|x|b> [register name] \n");
        fprintf(stderr, "- pc\n");
        fprintf(stderr, "- rm <memory start> <byte count> [display width]\n");
    }
    else if (CMD("run"))
    {
        *state = 0;
        cpu->trap_occured = 0;
    }
    else if (CMD_ARGS("wr", _write_register)) {}
    else if (CMD_ARGS("gr", _get_register)) {}
    else if (CMD_ARGS("sgr", _get_sysreg)) {}
    else if (CMD_ARGS("rm", _read_mem)) {}
    else if (CMD("pc")) {
        fprintf(stderr, "PC: 0x%x\n", cpu->pc);
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", command);
    }
}
