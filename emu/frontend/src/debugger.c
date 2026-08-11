#include "debugger.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cpu.h"

#define CMD(name) strstr(command, name) == command
#define CMD_ARGS(name, handler) cmd_with_args(command, cpu, name, handler)
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
    char mode = argv[0];
    switch(argv[0]) {
        case 'u':
        case 'i':
        case 'x':
        break;
        case '\0':
            mode = 'i';
            break;
        default:
            fprintf(stderr, "Incorrect argument #1. Usage: 'gr u|i|x [register number]\n");
            return 1;
    }

    char *val = strstr(argv, " ");
    char format[256];
    sprintf(format, "R%%i: %%%c\n", mode);
    if (val) {
        int register_number = atoi(val+1);
        fprintf(stderr, format, register_number, get_register(cpu, (uint8_t)register_number));
        return 1;
    }
    for(int i = 0; i < 32; i++) {
        fprintf(stderr, format, i, cpu->R[i]);
    }
    return 1;
}

int cmd_with_args(char *input, struct cpu *cpu, char *name, int (*handler)(struct cpu *, char *)){
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
    fgets(command, 256, stdin);
    if(CMD("step")) {
        step(cpu);
    }
    else if (CMD("run"))
    {
        if (!getenv("DEBUG"))
            cpu->debug = 0;
        *state = 0;
    }
    else if (CMD_ARGS("wr", _write_register)) {}
    else if (CMD_ARGS("gr", _get_register)) {}
    else
    {
        fprintf(stderr, "Unknown command: %s", command);
    }
}
