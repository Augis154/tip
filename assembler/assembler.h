#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "ir.h"
#include "lib/arena.h"
#include "lib/str_table.h"

#include <stddef.h>
#include <stdio.h>

#define MAX_ARGS 8

#define UNREACHABLE(msg)                                                  \
  fprintf(stderr, "UNREACHABLE: %s at %s:%d\n", msg, __FILE__, __LINE__); \
  abort()

typedef enum {
  SYM_LABEL,
  SYM_CONSTANT,
} SymbolType;

typedef struct {
  SymbolType type;

  union {
    uint32_t address;
    int32_t value;
  };

  uint32_t line_defined;
} Symbol;

typedef struct {
  const char *filename;

  Arena *str_arena;

  StrTable *instr_lut;
  StrTable *symbol_table;
} AssemblerCtx;

typedef struct {
  TypedLine *lines;
  size_t count;
  size_t capacity;
} Program;

void emit_uint32(uint8_t *buffer, uint32_t value);

void resolve_symbols(AssemblerCtx *ctx, Program *program);
void assemble_lines(AssemblerCtx *ctx, Program *program);

void free_symbols(AssemblerCtx *ctx);

#endif // ASSEMBLER_H