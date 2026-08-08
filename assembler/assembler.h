#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "arena.h"
#include "instr.h"
#include "str_table.h"

#include <stddef.h>
#include <stdio.h>

#define MAX_ARGS 8

typedef enum {
  LINE_ERROR,
  LINE_EMPTY,
  LINE_LABEL,
  LINE_INSTR,
  LINE_DIRECTIVE,
} LineType;

typedef struct {
  OperandType type;

  union {
    uint8_t reg_num;
    int32_t imm_value;
    const char *label;
  };
} Operand;

typedef struct {
  LineType type;

  union {
    const char *mnemonic;
    const char *label;
  };

  Operand args[MAX_ARGS];
  size_t arg_count;

  uint32_t line_num;
  uint32_t address;
} TypedLine;

typedef struct {
  TypedLine *lines;
  size_t count;
  size_t capacity;
} Lines;

typedef struct {
  uint32_t *instr;
  size_t count;
  size_t capacity;
} InstrBuffer;

typedef struct {
  const char *filename;

  Arena *str_arena;

  StrTable *instr_lut;
} AssemblerCtx;

void resolve_labels(AssemblerCtx *ctx, Lines *lines);
InstrBuffer assemble_lines(AssemblerCtx *ctx, Lines *lines);

#endif // ASSEMBLER_H