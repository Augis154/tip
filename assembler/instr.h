#ifndef INSTR_H
#define INSTR_H

#include "str_table.h"

#include <stdint.h>

typedef enum {
  FORMAT_R,
  FORMAT_I,
  FORMAT_IS, // For shift instructions
  FORMAT_S,
  FORMAT_U,

  FORMAT_NONE, // I type with no operands
  FORMAT_COUNT,
} Format;

typedef enum {
  OPT_NONE = 0b000,
  OPT_REG = 0b001,
  OPT_IMM = 0b010,
  OPT_LABEL = 0b100,
} OperandType;

typedef struct {
  const char *mnemonic;
  Format format;
  uint8_t opcode;
  uint16_t ext; // Extension bits for R-type or Shift Type
} InstrDef;

typedef struct {
  uint8_t arg_count;
  uint8_t arg_types[3];
} FormatRule;

StrTable *instr_create_lut();
void instr_free_lut(StrTable *instr_lut);

const InstrDef *instr_lookup(StrTable *instr_lut, const char *mnemonic);
const FormatRule *format_rule_lookup(Format format);

#endif // INSTR_H