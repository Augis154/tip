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
  SYS_SCALL = 0b0000,
  SYS_SBREAK = 0b0001,
  SYS_SRET = 0b0010,
  SYS_SRW = 0b0100,
  SYS_SRS = 0b0101,
  SYS_SRC = 0b0110,
} SysOp;

typedef enum {
  SR_CYCLE = 0x10,
  SR_CYCLEH = 0x11,

  SR_STS = 0x40,
  SR_STH = 0x41,

  SR_STPC = 0x51,
  SR_STC = 0x52,
  SR_STV = 0x53,
} SysReg;

typedef enum {
  STC_NONE = 0b0000,
  STC_INSTR_MISALIGNED = 0b0001,
  STC_INSTR_FAULT = 0b0010,
  STC_DATA_MISALIGNED = 0b0011,
  STC_DATA_FAULT = 0b0100,
  STC_ILLEGAL_INSTR = 0b0101,
  STC_SCALL = 0b1000,
  STC_SBREAK = 0b1001,
  STC_ALU_ERROR = 0b1100,
} TrapCause;

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

// STS register bit positions
#define POS_STS_SIE 0
#define POS_STS_SMODE 1
#define POS_STS_PIE 2
#define POS_STS_PMODE 3

#define MASK_TAG 0xF // 4 bits
#define MASK_FN 0xF  // 4 bits
#define MASK_OP 0xFF // 8 bits

#define MASK_REG 0x1F      // 5 bits
#define MASK_IMM14 0x3FFF  // 14 bits
#define MASK_IMM19 0x7FFFF // 19 bits
#define MASK_SHAMT 0x1F    // 5 bits
#define MASK_FN9 0x1FF     // 9 bits

// For S-type instructions, the immediate is split into two parts: imm[4:0] and imm[13:5]
#define POS_IMM4_0 8
#define POS_IMM13_5 23
#define MASK_IMM4_0 0x1F   // 5 bits
#define MASK_IMM13_5 0x1FF // 9 bits

#define INT14_MIN (-16384)  // -2^14
#define INT14_MAX 16383     // 2^14 - 1
#define INT19_MIN (-524288) // -2^19
#define INT19_MAX 524287    // 2^19 - 1

#endif // SPEC_H