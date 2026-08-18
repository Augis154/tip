#include "../spec.h"
#include "ir.h"
#include "isa.h"

#include <stddef.h>
#include <stdlib.h>

#define INSTR_TABLE_SIZE 128

#define OPCODE(tag, fn) ((uint8_t)(((tag) | (fn) << POS_FN) & MASK_OP))

#define ALU_REG(mnemonic, fn, ext) {mnemonic, FORMAT_R, OPCODE(TAG_ALU_REG, fn), ext}
#define ALU_IMM(mnemonic, fn) {mnemonic, FORMAT_I, OPCODE(TAG_ALU_IMM, fn), 0}
#define ALU_IMM_SHIFT(mnemonic, fn, ext) {mnemonic, FORMAT_IS, OPCODE(TAG_ALU_IMM, fn), ext}

#define LOAD(mnemonic, fn) {mnemonic, FORMAT_I, OPCODE(TAG_LOAD, fn), 0}
#define STORE(mnemonic, fn) {mnemonic, FORMAT_S, OPCODE(TAG_STORE, fn), 0}

#define CTRL(mnemonic, fn, format) {mnemonic, format, OPCODE(TAG_CTRL, fn), 0}
#define UPPER(mnemonic, fn) {mnemonic, FORMAT_U, OPCODE(TAG_UPPER, fn), 0}

#define SYS(mnemonic, fn, format) {mnemonic, format, OPCODE(TAG_SYS, fn), 0}

static const FormatRule format_rules[];
static const InstrDef instr_table[];

StrTable *instr_create_lut() {
  StrTable *instr_lut = malloc(sizeof(StrTable));
  st_init(instr_lut, INSTR_TABLE_SIZE);

  for (size_t i = 0; instr_table[i].mnemonic != NULL; i++) {
    st_put(instr_lut, instr_table[i].mnemonic, (void *)&instr_table[i]);
  }

  return instr_lut;
}

void instr_free_lut(StrTable *instr_lut) {
  st_free(instr_lut);
  free(instr_lut);
}

const InstrDef *instr_lookup(StrTable *instr_lut, const char *mnemonic) {
  return (const InstrDef *)st_get(instr_lut, mnemonic);
}

const FormatRule *format_rule_lookup(Format format) {
  if (format < FORMAT_R || format >= FORMAT_COUNT) {
    return NULL;
  }
  return &format_rules[format];
}

static const FormatRule format_rules[] = {
  [FORMAT_R] = {3, {OPT_REG, OPT_REG, OPT_REG}},
  [FORMAT_I] = {3, {OPT_REG, OPT_REG, OPT_IMM | OPT_LABEL}},
  [FORMAT_IS] = {3, {OPT_REG, OPT_REG, OPT_IMM}},
  [FORMAT_S] = {3, {OPT_REG, OPT_REG, OPT_IMM | OPT_LABEL}},
  [FORMAT_U] = {2, {OPT_REG, OPT_IMM | OPT_LABEL}},

  [FORMAT_NONE] = {0, {0}},
};

static const InstrDef instr_table[] = {
  // ALU-Reg Instructions
  ALU_REG("add", ALU_ADD, EXT_ADD),
  ALU_REG("sub", ALU_ADD, EXT_SUB),

  ALU_REG("and", ALU_AND, 0),
  ALU_REG("or", ALU_OR, 0),
  ALU_REG("xor", ALU_XOR, 0),

  ALU_REG("sll", ALU_SHIFT, SHIFT_SLL),
  ALU_REG("srl", ALU_SHIFT, SHIFT_SRL),
  ALU_REG("sra", ALU_SHIFT, SHIFT_SRA),

  ALU_REG("seq", ALU_SEQ, 0),
  ALU_REG("sne", ALU_SNE, 0),
  ALU_REG("slt", ALU_SLT, 0),
  ALU_REG("sltu", ALU_SLTU, 0),

  ALU_REG("mul", ALU_EXT, EXT_MUL),
  ALU_REG("mulh", ALU_EXT, EXT_MULH),
  ALU_REG("mulhu", ALU_EXT, EXT_MULHU),
  ALU_REG("div", ALU_EXT, EXT_DIV),
  ALU_REG("rem", ALU_EXT, EXT_REM),
  ALU_REG("divu", ALU_EXT, EXT_DIVU),
  ALU_REG("remu", ALU_EXT, EXT_REMU),

  // ALU-Imm Instructions
  ALU_IMM("addi", ALU_ADD),
  ALU_IMM("andi", ALU_AND),
  ALU_IMM("ori", ALU_OR),
  ALU_IMM("xori", ALU_XOR),

  ALU_IMM_SHIFT("slli", ALU_SHIFT, SHIFT_SLL),
  ALU_IMM_SHIFT("srli", ALU_SHIFT, SHIFT_SRL),
  ALU_IMM_SHIFT("srai", ALU_SHIFT, SHIFT_SRA),

  ALU_IMM("seqi", ALU_SEQ),
  ALU_IMM("snei", ALU_SNE),
  ALU_IMM("slti", ALU_SLT),
  ALU_IMM("sltui", ALU_SLTU),

  // Load Instructions
  LOAD("lb", MEM_BYTE),
  LOAD("lh", MEM_HALF),
  LOAD("lw", MEM_WORD),
  LOAD("lbu", MEM_BYTE_U),
  LOAD("lhu", MEM_HALF_U),

  // Store Instructions
  STORE("sb", MEM_BYTE),
  STORE("sh", MEM_HALF),
  STORE("sw", MEM_WORD),

  // Control Instructions
  CTRL("beq", CTRL_EQ, FORMAT_S),
  CTRL("bne", CTRL_NE, FORMAT_S),
  CTRL("blt", CTRL_LT, FORMAT_S),
  CTRL("bge", CTRL_GE, FORMAT_S),
  CTRL("bltu", CTRL_LTU, FORMAT_S),
  CTRL("bgeu", CTRL_GEU, FORMAT_S),
  CTRL("jas", CTRL_JAS, FORMAT_U),
  CTRL("jasr", CTRL_JASR, FORMAT_I),

  // Upper Instructions
  UPPER("lui", UPPER_LUI),
  UPPER("lupc", UPPER_LUPC),

  // System Instructions
  SYS("scall", SYS_SCALL, FORMAT_NONE),
  SYS("sbreak", SYS_SBREAK, FORMAT_NONE),
  SYS("sret", SYS_SRET, FORMAT_NONE),
  SYS("srw", SYS_SRW, FORMAT_I),
  SYS("srs", SYS_SRS, FORMAT_I),
  SYS("src", SYS_SRC, FORMAT_I),

  {NULL} // Sentinel value to mark the end of the table
};
