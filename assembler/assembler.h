#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "arena.h"
#include "instr.h"
#include "str_table.h"

#include <stddef.h>
#include <stdio.h>

#define MAX_ARGS 8

#define UNREACHABLE(msg)                                                  \
  fprintf(stderr, "UNREACHABLE: %s at %s:%d\n", msg, __FILE__, __LINE__); \
  abort()

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
    const char *label_ref;
  };
} Operand;

typedef struct {
  LineType type;

  const char *label;
  union {
    const InstrDef *instr_def;
    const char *directive;
  };

  Operand args[MAX_ARGS];
  size_t arg_count;

  uint32_t line_num;

  uint32_t address;
  uint32_t instr;
} TypedLine;

typedef struct {
  TypedLine *lines;
  size_t count;
  size_t capacity;
} Program;

typedef struct {
  const char *filename;

  Arena *str_arena;

  StrTable *instr_lut;
} AssemblerCtx;

void resolve_labels(AssemblerCtx *ctx, Program *program);
void assemble_lines(AssemblerCtx *ctx, Program *program);

#endif // ASSEMBLER_H