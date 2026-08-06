#ifndef SPEC_H
#define SPEC_H

#include <stdint.h>

typedef enum {
  TAG_ALU_REG = 0b0000,
  TAG_ALU_IMM = 0b0001,
  TAG_LOAD = 0b0010,
  TAG_STORE = 0b0011,
  TAG_CTRL = 0b0100,
  TAG_UPPER = 0b0101,
  TAG_SYS = 0b1111,
} InstrTag;

typedef enum {
  ALU_ADD = 0b0000,
  ALU_AND = 0b0100,
  ALU_OR = 0b0101,
  ALU_XOR = 0b0110,
  ALU_SHIFT = 0b0111,
  ALU_SEQ = 0b1000,
  ALU_SNE = 0b1001,
  ALU_SLT = 0b1010,
  ALU_SLTU = 0b1011,
  ALU_EXT = 0b1100, // Only for ALU_REG
} AluOp;

typedef enum {
  // If ALU_ADD
  EXT_ADD = 0b000,
  EXT_SUB = 0b001,

  // If ALU_SHIFT
  SHIFT_SLL = 0b000,
  SHIFT_SRL = 0b010,
  SHIFT_SRA = 0b011,

  // If ALU_EXT
  EXT_MUL = 0b000,
  EXT_MULH = 0b010,
  EXT_MULHU = 0b011,
  EXT_DIV = 0b100,
  EXT_REM = 0b110,
  EXT_DIVU = 0b101,
  EXT_REMU = 0b111,
} AluSubOp;

typedef enum {
  MEM_BYTE = 0b0000,
  MEM_HALF = 0b0001,
  MEM_WORD = 0b0010,
  MEM_BYTE_U = 0b0100, // Only for loads
  MEM_HALF_U = 0b0101, // Only for loads
} MemOp;

typedef enum {
  CTRL_EQ = 0b0000,
  CTRL_NE = 0b0001,
  CTRL_LT = 0b0010,
  CTRL_GE = 0b0011,
  CTRL_LTU = 0b0110,
  CTRL_GEU = 0b0111,

  CTRL_JAS = 0b1000,
  CTRL_JASR = 0b1001,
} CtrlOp;

typedef enum {
  UPPER_LUI = 0b0000,
  UPPER_LUPC = 0b0001,
} UpperOp;

typedef enum {
  SYS_ECALL = 0b0000,
  SYS_EBREAK = 0b0001,
} SysOp;

#define RESET_VECTOR 0xFFC

#define POS_TAG 0
#define POS_FN 4
#define POS_RD 8
#define POS_R1 13
#define POS_R2 18
#define POS_IMM14 18
#define POS_IMM19 13
#define POS_SHAMT 18
#define POS_FN9 23

#define MASK_TAG 0xF // 4 bits
#define MASK_FN 0xF  // 4 bits
#define MASK_OP 0xFF // 8 bits

#define MASK_REG 0x1F      // 5 bits
#define MASK_IMM14 0x3FFF  // 14 bits
#define MASK_IMM19 0x7FFFF // 19 bits
#define MASK_SHAMT 0x1F    // 5 bits
#define MASK_FN9 0x1FF     // 9 bits

#endif // SPEC_H