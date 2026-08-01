#include "instr.h"

#include "../spec.h"

#include <stddef.h>
#include <stdlib.h>

static InstrDef instr_table[INSTR_TABLE_SIZE];
static StrTable *instr_lut;

void instr_init_lut() {
  instr_lut = malloc(sizeof(StrTable));
  st_init(instr_lut, INSTR_TABLE_SIZE);

  for (size_t i = 0; instr_table[i].mnemonic != NULL; i++) {
    st_put(instr_lut, instr_table[i].mnemonic, &instr_table[i]);
  }
}

void instr_free_lut() {
  st_free(instr_lut);
  free(instr_lut);
}

InstrDef *instr_get_def(const char *mnemonic) {
  return (InstrDef *)st_get(instr_lut, mnemonic);
}

static InstrDef instr_table[INSTR_TABLE_SIZE] = {
  // ALU-Reg Instructions
  {"add", TAG_ALU_REG, ALU_ADD, FORMAT_R, EXT_ADD},
  {"sub", TAG_ALU_REG, ALU_ADD, FORMAT_R, EXT_SUB},

  {"and", TAG_ALU_REG, ALU_AND, FORMAT_R, 0},
  {"or", TAG_ALU_REG, ALU_OR, FORMAT_R, 0},
  {"xor", TAG_ALU_REG, ALU_XOR, FORMAT_R, 0},

  {"sll", TAG_ALU_REG, ALU_SHIFT, FORMAT_R, SHIFT_SLL},
  {"srl", TAG_ALU_REG, ALU_SHIFT, FORMAT_R, SHIFT_SRL},
  {"sra", TAG_ALU_REG, ALU_SHIFT, FORMAT_R, SHIFT_SRA},

  {"seq", TAG_ALU_REG, ALU_SEQ, FORMAT_R, 0},
  {"sne", TAG_ALU_REG, ALU_SNE, FORMAT_R, 0},
  {"slt", TAG_ALU_REG, ALU_SLT, FORMAT_R, 0},
  {"sltu", TAG_ALU_REG, ALU_SLTU, FORMAT_R, 0},

  {"mul", TAG_ALU_REG, ALU_EXT, FORMAT_R, EXT_MUL},
  {"mulh", TAG_ALU_REG, ALU_EXT, FORMAT_R, EXT_MULH},
  {"mulhu", TAG_ALU_REG, ALU_EXT, FORMAT_R, EXT_MULHU},
  {"div", TAG_ALU_REG, ALU_EXT, FORMAT_R, EXT_DIV},
  {"rem", TAG_ALU_REG, ALU_EXT, FORMAT_R, EXT_REM},
  {"divu", TAG_ALU_REG, ALU_EXT, FORMAT_R, EXT_DIVU},
  {"remu", TAG_ALU_REG, ALU_EXT, FORMAT_R, EXT_REMU},

  // ALU-Imm Instructions
  {"addi", TAG_ALU_IMM, ALU_ADD, FORMAT_I, 0},

  {"andi", TAG_ALU_IMM, ALU_AND, FORMAT_I, 0},
  {"ori", TAG_ALU_IMM, ALU_OR, FORMAT_I, 0},
  {"xori", TAG_ALU_IMM, ALU_XOR, FORMAT_I, 0},

  {"slli", TAG_ALU_IMM, ALU_SHIFT, FORMAT_IS, SHIFT_SLL},
  {"srli", TAG_ALU_IMM, ALU_SHIFT, FORMAT_IS, SHIFT_SRL},
  {"srai", TAG_ALU_IMM, ALU_SHIFT, FORMAT_IS, SHIFT_SRA},

  {"seqi", TAG_ALU_IMM, ALU_SEQ, FORMAT_I, 0},
  {"snei", TAG_ALU_IMM, ALU_SNE, FORMAT_I, 0},
  {"slti", TAG_ALU_IMM, ALU_SLT, FORMAT_I, 0},
  {"sltui", TAG_ALU_IMM, ALU_SLTU, FORMAT_I, 0},

  // Load Instructions
  {"lb", TAG_LOAD, MEM_BYTE, FORMAT_I, 0},
  {"lh", TAG_LOAD, MEM_HALF, FORMAT_I, 0},
  {"lw", TAG_LOAD, MEM_WORD, FORMAT_I, 0},
  {"lbu", TAG_LOAD, MEM_BYTE_U, FORMAT_I, 0},
  {"lhu", TAG_LOAD, MEM_HALF_U, FORMAT_I, 0},

  // Store Instructions
  {"sb", TAG_STORE, MEM_BYTE, FORMAT_S, 0},
  {"sh", TAG_STORE, MEM_HALF, FORMAT_S, 0},
  {"sw", TAG_STORE, MEM_WORD, FORMAT_S, 0},

  // Control Instructions
  {"beq", TAG_CTRL, CTRL_EQ, FORMAT_S, 0},
  {"bne", TAG_CTRL, CTRL_NE, FORMAT_S, 0},
  {"blt", TAG_CTRL, CTRL_LT, FORMAT_S, 0},
  {"bge", TAG_CTRL, CTRL_GE, FORMAT_S, 0},
  {"bltu", TAG_CTRL, CTRL_LTU, FORMAT_S, 0},

  {"jas", TAG_CTRL, CTRL_JAS, FORMAT_U, 0},
  {"jasr", TAG_CTRL, CTRL_JASR, FORMAT_I, 0},

  // Upper Instructions
  {"lui", TAG_UPPER, UPPER_LDUI, FORMAT_U, 0},
  {"lupc", TAG_UPPER, UPPER_LDUPC, FORMAT_U, 0},

  {NULL, 0, 0, 0, 0} // Sentinel value to mark the end of the table
};