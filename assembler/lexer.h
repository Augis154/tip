#ifndef LEXER_H
#define LEXER_H

#include "arena.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_TOKENS_PER_LINE 32
#define MAX_LINE_LENGTH 256

typedef enum {
  TOKEN_ERROR,
  TOKEN_EOL,

  TOKEN_IDENTIFIER,
  TOKEN_REGISTER,
  TOKEN_IMMEDIATE,

  TOKEN_DOT,
  TOKEN_COMMA,
  TOKEN_COLON,
} TokenType;

typedef struct {
  TokenType type;

  const char *lexeme;
  size_t lexeme_length;

  union {
    uint8_t reg_num;   // For registers
    int32_t imm_value; // For immediates
  };
} Token;

typedef struct {
  Token tokens[MAX_TOKENS_PER_LINE];
  size_t count;

  uint32_t line_num;
} TokenizedLine;

void tokenize_file(FILE *file, Arena *str_arena, TokenizedLine *tokenized_lines, size_t *line_count);

#endif // LEXER_H