#include <stdint.h>
#ifndef FORMATS_H
#define FORMATS_H

struct R_Type {
    uint8_t rd;
    uint8_t r1;
    uint8_t r2;
    uint16_t fn9;
};

struct I_Type {
    uint8_t rd;
    uint8_t r1;
    uint16_t imm13;
    uint8_t _;
};

struct IS_Type {
    uint8_t rd;
    uint8_t r1;
    uint8_t shamt5;
    uint16_t fn9;
};

struct S_Type {
    uint8_t _;
    uint8_t r1;
    uint8_t r2;
    uint16_t imm;
};

struct U_Type {
    uint8_t rd;
    uint32_t imm18;
};

union Operands {
    struct R_Type rType;
    struct I_Type iType;
    struct IS_Type isType;
    struct S_Type sType;
    struct U_Type uType;
};

typedef enum {
    R,
    I,
    IS,
    S,
    U
} OperandFormat;

struct Instruction {
    uint32_t raw;
    OperandFormat fmt;
    uint8_t tag;
    uint8_t fn;
    union Operands operands;
};

void decode_instr_operands(struct Instruction *instr, OperandFormat fmt);

#endif
