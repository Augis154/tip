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

typedef enum {
  SYM_LABEL,
  SYM_CONSTANT,
} SymbolType;

typedef struct {
  const char *filename;

  Arena *str_arena;

  StrTable *instr_lut;
  StrTable *symbol_table;
} AssemblerCtx;

typedef struct {
  OperandType type;

  union {
    uint8_t reg_num;
    int32_t imm_value;
    const char *label_ref;
  };
} Operand;

typedef struct {
  SymbolType type;

  union {
    uint32_t address;
    int32_t value;
  };

  uint32_t line_defined;
} Symbol;

typedef struct _TypedLine TypedLine;

typedef void (*DirectiveLayoutFunc)(AssemblerCtx *ctx, TypedLine *line, uint32_t *current_address);
typedef void (*DirectiveEmitFunc)(AssemblerCtx *ctx, TypedLine *line);

typedef struct {
  char *name;
  DirectiveLayoutFunc layout;
  DirectiveEmitFunc emit;

  int8_t arg_count;
  uint8_t arg_types[4];
} DirectiveDef;

typedef struct _TypedLine {
  LineType type;

  const char *label;
  union {
    const InstrDef *instr_def;
    const DirectiveDef *directive_def;
  };

  Operand args[MAX_ARGS];
  size_t arg_count;

  uint32_t line_num;
  uint32_t address;
  uint32_t instruction;

  uint8_t *bytes;
  size_t byte_count;
} TypedLine;

typedef struct {
  TypedLine *lines;
  size_t count;
  size_t capacity;
} Program;

void resolve_labels(AssemblerCtx *ctx, Program *program);
void assemble_lines(AssemblerCtx *ctx, Program *program);

void free_symbols(AssemblerCtx *ctx);

#endif // ASSEMBLER_H