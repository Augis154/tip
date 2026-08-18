#ifndef ISA_H
#define ISA_H

#include "assembler.h"
#include "ir.h"
#include "lib/str_table.h"

#include <stdint.h>

#define UNLIMITED_ARGS -1

typedef enum {
  FORMAT_R,
  FORMAT_I,
  FORMAT_IS, // For shift instructions
  FORMAT_S,
  FORMAT_U,

  FORMAT_NONE, // I type with no operands
  FORMAT_COUNT,
} Format;

typedef struct InstrDef {
  const char *mnemonic;
  Format format;
  uint8_t opcode;
  uint16_t ext; // Extension bits for R-type or Shift Type
} InstrDef;

typedef struct {
  uint8_t arg_count;
  uint8_t arg_types[3];
} FormatRule;

typedef void (*DirectiveLayoutFunc)(AssemblerCtx *ctx, TypedLine *line, uint32_t *current_address);
typedef void (*DirectiveEmitFunc)(AssemblerCtx *ctx, TypedLine *line);

typedef struct DirectiveDef {
  char *name;
  DirectiveLayoutFunc layout;
  DirectiveEmitFunc emit;

  int8_t arg_count;
  uint8_t arg_types[4];
} DirectiveDef;

StrTable *instr_create_lut();
void instr_free_lut(StrTable *instr_lut);

const InstrDef *instr_lookup(StrTable *instr_lut, const char *mnemonic);
const FormatRule *format_rule_lookup(Format format);

const DirectiveDef *find_directive(const char *name);

#endif // ISA_H