#ifndef PARSER_H
#define PARSER_H

#include "assembler.h"
#include "instr.h"
#include "lexer.h"

#include <stddef.h>
#include <stdint.h>
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

  const char *mnemonic;
  const char *label;

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

Lines parse_lines(AssemblerCtx *ctx, TokenizedLine *tokenized_lines, size_t line_count);
void free_lines(Lines *lines);

#endif // PARSER_H