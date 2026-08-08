#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "arena.h"
#include "str_table.h"

#include <stddef.h>
#include <stdio.h>

typedef struct {
  const char *filename;

  StrTable *instr_lut;
  Arena *str_arena;
} AssemblerCtx;

#endif // ASSEMBLER_H