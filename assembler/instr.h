#ifndef INSTR_H
#define INSTR_H

#include "str_table.h"

#include <stdint.h>

typedef enum {
  FORMAT_R,
  FORMAT_I,
  FORMAT_S,
  FORMAT_U,
  FORMAT_IS, // For shift instructions
  FORMAT_ALIAS,
} Format;

typedef struct {
  const char *mnemonic;
  uint8_t tag;
  uint8_t opcode;
  Format format;
  uint16_t fn9; // Extension bits for R-type or Shift Type
} InstrDef;

void instr_init_lut(StrTable *instr_lut);
void instr_free_lut(StrTable *instr_lut);

InstrDef *instr_lookup(StrTable *instr_lut, const char *mnemonic);

#endif // INSTR_H