#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "str_table.h"

#include <stdio.h>

typedef struct {
  const char *filename;
  FILE *file;

  StrTable *instr_lut;
} AssemblerCtx;

#endif // ASSEMBLER_H