#include "cpu.h"
#include "debug.h"
#include <stdio.h>

typedef int (*Disassembler)(struct cpu *cpu, struct Instruction *instr);

struct InstructionFormat {
    const char *name;
    OperandFormat fmt;
    uint8_t isSigned;
    uint16_t variant;
    struct InstructionFormat *subFunctions;
};

struct InstructionFormat instr_tags[0x10][0x10] = {
    [TAG_ALU_REG] = {
        [ALU_ADD] = {"n/a", R, 0, 0, (struct InstructionFormat[]) {
                {"add", R, 0, EXT_ADD, NULL},
                {"sub", R, 0, EXT_SUB, NULL},
                {0}
            }
        },
        [ALU_AND] = {"and", R, 0},
        [ALU_OR] = {"or", R, 0},
        [ALU_XOR] = {"or", R, 0},
        [ALU_SHIFT] = {"n/a", R, 0, 0, (struct InstructionFormat[]) {
            {"sll", R, 0, SHIFT_SLL, NULL},
            {"srl", R, 0, SHIFT_SRL, NULL},
            {"sra", R, 0, SHIFT_SRA, NULL},
            {0}
        }},
        [ALU_SEQ] = {"seq", R, 0},
        [ALU_SNE] = {"sne", R, 0},
        [ALU_SLT] = {"slt", R, 1},
        [ALU_SLTU] = {"sltu", R, 0},
        [ALU_EXT] = {"n/a", R, 0, 0, (struct InstructionFormat[]) {
            {"mul", R, 0, EXT_MUL, NULL},
            {"mulh", R, 1, EXT_MULH, NULL},
            {"mulhu", R, 0, EXT_MULHU, NULL},
            {"div", R, 0, EXT_DIV, NULL},
            {"rem", R, 0, EXT_REM, NULL},
            {"divu", R, 0, EXT_DIVU, NULL},
            {"remu", R, 0, EXT_REMU, NULL},
            {0}
        }},
    },
    [TAG_ALU_IMM] = {
        [ALU_ADD] = {"addi", I, 1},
        [ALU_AND] = {"andi", I, 0},
        [ALU_OR] = {"ori", I, 0},
        [ALU_XOR] = {"xori", I, 0},
        [ALU_SHIFT] = {"n/a", IS, 0, 3, (struct InstructionFormat[]) {
            {"slli", IS, 0, SHIFT_SLL, NULL},
            {"srli", IS, 0, SHIFT_SRL, NULL},
            {"srai", IS, 0, SHIFT_SRA, NULL},
            {0}
        }},
        [ALU_SEQ] = {"seqi", I, 1},
        [ALU_SNE] = {"snei", I, 1},
        [ALU_SLT] = {"slti", I, 1},
        [ALU_SLTU] = {"sltiu", I, 0},
    },
    [TAG_LOAD] = {
        [MEM_BYTE] = {"lb", I, 1},
        [MEM_HALF] = {"lh", I, 1},
        [MEM_WORD] = {"lw", I, 1},
        [MEM_BYTE_U] = {"lbu", I, 1},
        [MEM_HALF_U] = {"lhu", I, 1},
    },
    [TAG_STORE] = {
        [MEM_BYTE] = {"sb", S, 1},
        [MEM_HALF] = {"sh", S, 1},
        [MEM_WORD] = {"sw", S, 1},
    },
    [TAG_CTRL] = { 
        [CTRL_EQ] = {"beq", S, 1},
        [CTRL_NE] = {"bne", S, 1},
        [CTRL_LT] = {"blt", S, 1},
        [CTRL_GE] = {"bge", S, 1},
        [CTRL_LTU] = {"bltu", S, 0},
        [CTRL_GEU] = {"bgeu", S, 0},
        [CTRL_JAS] = {"jas", U, 1},
        [CTRL_JASR] = {"jasr", I, 1},
    },
    [TAG_UPPER] = {
        [UPPER_LUI] = {"lui", U, 0},
        [UPPER_LUPC] = {"lupc", U, 0},
    },
    [TAG_SYS] = {
        [SYS_SCALL] = {"scall", None, 0},
        [SYS_SBREAK] = {"sbreak", None, 0},
        [SYS_SRET] = {"sret", None, 0},
        [SYS_SRW] = {"srw", I, 0},
        [SYS_SRS] = {"srs", I, 0},
        [SYS_SRC] = {"src", I, 0},
    },
};

#define FETCH_SUBVAR(type) if(fmt->subFunctions) { \
                for (struct InstructionFormat *f = fmt->subFunctions; f->name != NULL; f++) { \
                    if (f->variant == instr->operands.type.fn9) \
                        fmt = f; \
                } \
            }

void print_instr(struct cpu *cpu, struct Instruction *instr){
    (void) cpu;
    struct InstructionFormat *fmt = &instr_tags[instr->tag][instr->fn];
    decode_instr_operands(instr, fmt->fmt);
    fprintf(stderr, "%08x: ", cpu->pc);
    // if(fmt->subFunctions) {
    //     for (struct InstructionFormat *f = fmt->subFunctions; f->name != NULL; f++) {
    //         struct Instruction temp = *instr;
    //         decode_instr_operands(&temp, f->fmt);
    //         if (f->variant == instr->operands.rType.fn9)
    //     }
    // }

    switch(fmt->fmt) {
        case R:
            FETCH_SUBVAR(rType)
            fprintf(stderr, "%s, r%i, r%i, r%i", 
                fmt->name, 
                instr->operands.rType.rd, 
                instr->operands.rType.r1, 
                instr->operands.rType.r2);
            break;
        case I:
            if(fmt->isSigned)
                fprintf(stderr, "%s, r%i, r%i, %hi", 
                    fmt->name, 
                    instr->operands.iType.rd, 
                    instr->operands.iType.r1,
                    instr->operands.iType.imm13);
            else
                fprintf(stderr, "%s, r%i, r%i, %u", 
                    fmt->name, 
                    instr->operands.iType.rd, 
                    instr->operands.iType.r1,
                    instr->operands.iType.imm13);
            break;
        case IS:
            FETCH_SUBVAR(isType)
            fprintf(stderr, "%s, r%i, r%i, %u", 
                fmt->name, 
                instr->operands.isType.rd, 
                instr->operands.isType.r1,
                instr->operands.isType.shamt5);
            break;
        case S:
            fprintf(stderr, "%s, r%i, r%i, %hi", 
                fmt->name, 
                instr->operands.sType.r2, 
                instr->operands.sType.r1,
                instr->operands.sType.imm);
            break;
        case U:
            fprintf(stderr, "%s, r%i, %i", 
                fmt->name, 
                instr->operands.uType.rd, 
                instr->operands.uType.imm18);
            break;
        default:
            break;
    }
    fprintf(stderr, "\n");
};
