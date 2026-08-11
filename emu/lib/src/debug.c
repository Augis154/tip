#include "cpu.h"
#include "debug.h"
#include <stdio.h>

#define TAG_DECODE(tag) case TAG_##tag: return #tag;
#define FN_DECODE(fn) case fn: return #fn;
#define FN_VAR_DECODE(fn, var) case fn: var = #fn; break;

static char *decode_tag(uint8_t tag4){
    switch(tag4) {
        TAG_DECODE(ALU_REG)
        TAG_DECODE(ALU_IMM)
        TAG_DECODE(LOAD)
        TAG_DECODE(STORE)
        TAG_DECODE(CTRL)
        TAG_DECODE(UPPER)
        TAG_DECODE(SYS)
        default: 
            return "N/A";
    }
}

static char *decode_fn(uint8_t fn4){
    switch(fn4) {
        FN_DECODE(ALU_ADD)
        FN_DECODE(ALU_AND)
        FN_DECODE(ALU_OR)
        FN_DECODE(ALU_XOR)
        FN_DECODE(ALU_SHIFT)
        FN_DECODE(ALU_SEQ)
        FN_DECODE(ALU_SNE)
        FN_DECODE(ALU_SLT)
        FN_DECODE(ALU_SLTU)
        FN_DECODE(ALU_EXT)
        default:
            return "N/A";
    }
}

static char *decode_fn_ctrl(uint8_t fn4){
    switch(fn4) {
        FN_DECODE(CTRL_EQ)
        FN_DECODE(CTRL_GE)
        FN_DECODE(CTRL_GEU)
        FN_DECODE(CTRL_JAS)
        FN_DECODE(CTRL_JASR)
        FN_DECODE(CTRL_LT)
        FN_DECODE(CTRL_LTU)
        FN_DECODE(CTRL_NE)
        default:
            return "N/A";
    }
}

static void decode_common(uint8_t tag4, uint8_t fn4) {
    char *tagname = decode_tag(tag4);
    char *fn_name = decode_fn(fn4);
    fprintf(stderr, "%s %s ", tagname, fn_name);
};

static void decode_common_control(uint8_t tag4, uint8_t fn4) {
    char *tagname = decode_tag(tag4);
    char *fn_name = decode_fn_ctrl(fn4);
    fprintf(stderr, "%s %s ", tagname, fn_name);
};

static void decode_r(struct cpu *cpu, uint32_t instruction, uint8_t tag4, uint8_t fn4) {
    (void)cpu;
    (void)instruction;
    decode_common(tag4, fn4);
    uint8_t rd = (instruction >> POS_RD) & MASK_REG;
    uint8_t r1 = (instruction >> POS_R1) & MASK_REG;
    uint8_t r2 = (instruction >> POS_R2) & MASK_REG;
    uint16_t fn9 = (instruction >> POS_FN9) & MASK_FN9;
    char *fn_name;
    switch(fn9){
        FN_VAR_DECODE(0, fn_name)
        FN_VAR_DECODE(EXT_DIV, fn_name)
        FN_VAR_DECODE(EXT_DIVU, fn_name)
        FN_VAR_DECODE(EXT_MULH, fn_name)
        FN_VAR_DECODE(EXT_MULHU, fn_name)
        FN_VAR_DECODE(EXT_REM, fn_name)
        FN_VAR_DECODE(EXT_REMU, fn_name)
        FN_VAR_DECODE(EXT_SUB, fn_name)
        default:
            fn_name = "N/A";
            break;
    }
    fprintf(stderr, "rd: %d, r1: %d, r2: %d, fn9: %s\n", rd, r1, r2, fn_name);
};

static void decode_i(struct cpu *cpu, uint32_t instruction, uint8_t tag4, uint8_t fn4) {
    (void)cpu;
    (void)instruction;
    decode_common(tag4, fn4);
    uint8_t rd = (instruction >> POS_RD) & MASK_REG;
    uint8_t r1 = (instruction >> POS_R1) & MASK_REG;
    uint16_t imm = (instruction >> POS_IMM14) & MASK_IMM14;
    fprintf(stderr, "rd: %d, r1: %d, imm: %d\n", rd, r1, imm);
};

static void decode_is(struct cpu *cpu, uint32_t instruction, uint8_t tag4, uint8_t fn4) {
    (void)cpu;
    (void)instruction;
    decode_common(tag4, fn4);
    uint8_t rd = (instruction >> POS_RD) & MASK_REG;
    uint8_t r1 = (instruction >> POS_R1) & MASK_REG;
    uint8_t shamt = (instruction >> POS_SHAMT) & MASK_SHAMT;
    uint8_t fn9 = (instruction >> POS_FN9) & MASK_FN9;
    char *fn_name;
    switch (fn9) {
        FN_VAR_DECODE(SHIFT_SLL, fn_name)
        FN_VAR_DECODE(SHIFT_SRL, fn_name)
        FN_VAR_DECODE(SHIFT_SRA, fn_name)
        default:
            fn_name = "N/A";
            break;
    }
    fprintf(stderr, "rd: %d, r1: %d, shamt: %d, fn9: %s\n", rd, r1, shamt, fn_name);

};

static void decode_s(struct cpu *cpu, uint32_t instruction, uint8_t tag4, uint8_t fn4) {
    (void)cpu;
    (void)instruction;
    decode_common_control(tag4, fn4);
    uint8_t r1 = (instruction >> POS_R1) & MASK_REG;
    uint8_t r2 = (instruction >> POS_R2) & MASK_REG;
    uint8_t imm4_0 = (instruction >> POS_IMM4_0) & MASK_IMM4_0;
    uint16_t imm13_5 = (instruction >> POS_IMM13_5) & MASK_IMM13_5;
    uint16_t imm = (imm13_5 << 5) | imm4_0;
    int16_t simm = imm & 0x200 ? 0xFE00 | imm : imm;

    fprintf(stderr, "r1: %d, r2: %d, uimm: %d simm: %d\n", r1, r2, imm, simm);
};

static void decode_u(struct cpu *cpu, uint32_t instruction, uint8_t tag4, uint8_t fn4) {
    (void)cpu;
    (void)instruction;
    decode_common(tag4, fn4);
    uint8_t rd = (instruction >> POS_RD) & MASK_REG;
    uint32_t imm = (instruction >> POS_IMM19) & MASK_IMM19;
    fprintf(stderr, "rd: %d, imm: %d\n", rd, imm);
};

static void (*ALU_REG[0xF])(struct cpu *cpu, uint32_t instruction, uint8_t tag4, uint8_t fn4) = {
    [ALU_ADD] = decode_r,
    [ALU_AND] = decode_r,
    [ALU_OR] = decode_r,
    [ALU_XOR] = decode_r,
    [ALU_SHIFT] = decode_r,
    [ALU_SEQ] = decode_r,
    [ALU_SNE] = decode_r,
    [ALU_SLT] = decode_r,
    [ALU_SLTU] = decode_r,
    [ALU_EXT] = decode_r,
};

static void (*ALU_IMM[0xF])(struct cpu *cpu, uint32_t instruction, uint8_t tag4, uint8_t fn4) = {
    [ALU_ADD] = decode_i,
    [ALU_AND] = decode_i,
    [ALU_OR] = decode_i,
    [ALU_XOR] = decode_i,
    [ALU_SHIFT] = decode_is,
    [ALU_SEQ] = decode_i,
    [ALU_SNE] = decode_i,
    [ALU_SLT] = decode_i,
    [ALU_SLTU] = decode_i,
};

static void (*LOADS[0xF])(struct cpu *cpu, uint32_t instruction, uint8_t tag4, uint8_t fn4) = {
    [MEM_BYTE] = decode_i,
    [MEM_HALF] = decode_i,
    [MEM_WORD] = decode_i,
    [MEM_BYTE_U] = decode_i,
    [MEM_HALF_U] = decode_i,
};

static void (*STORES[0xF])(struct cpu *cpu, uint32_t instruction, uint8_t tag4, uint8_t fn4) = {
    [MEM_BYTE] = decode_s,
    [MEM_HALF] = decode_s,
    [MEM_WORD] = decode_s,
};

static void (*CONTROL[0xF])(struct cpu *cpu, uint32_t instruction, uint8_t tag4, uint8_t fn4) = {
    [CTRL_EQ] = decode_s,
    [CTRL_NE] = decode_s,
    [CTRL_LT] = decode_s,
    [CTRL_GE] = decode_s,
    [CTRL_LTU] = decode_s,
    [CTRL_GEU] = decode_s,
    [CTRL_JAS] = decode_u,
    [CTRL_JASR] = decode_i,
};

static void (**tags[0xF])(struct cpu *cpu, uint32_t instruction, uint8_t tag4, uint8_t fn4) = {
    [TAG_ALU_REG] = ALU_REG,
    [TAG_ALU_IMM] = ALU_IMM,
    [TAG_LOAD] = LOADS,
    [TAG_STORE] = STORES,
    [TAG_CTRL] = CONTROL,
};

void print_instr(struct cpu *cpu, uint32_t instr){
    uint8_t tag4 = (instr >> POS_TAG) & MASK_TAG;
    uint8_t fn4 = (instr >> POS_FN) & MASK_FN;
    
    void (*decoder)(struct cpu *, uint32_t, uint8_t, uint8_t) = tags[tag4][fn4];
    if (decoder) {
        fprintf(stderr, "0x%x: ", cpu->pc);
        decoder(cpu, instr, tag4, fn4);
    }
};
