#ifndef PARSER_H
#define PARSER_H

#include "assembler.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
  LINE_ERROR,
  LINE_EMPTY,
  LINE_LABEL,
  LINE_INSTR,
  LINE_DIRECTIVE,
} LineType;

typedef struct {
  LineType type;
  uint32_t line_num;

  const char *mnemonic;
  const char *label;

  uint8_t rd, r1, r2;
  int32_t imm;
  const char *target;
} Line;

typedef struct {
  Line *lines;
  size_t count;
  size_t capacity;
} Lines;

Lines parse_file(AssemblerCtx *ctx);
void free_lines(Lines *lines);

#endif // PARSER_H