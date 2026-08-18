#include "frontend.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256

static bool is_space(char c) {
  return c == ' ' || c == '\t';
}

static bool is_eol(char c) {
  return c == '\0' || c == '\n' || c == '\r' || c == ';';
}

static bool is_digit(char c) {
  return c >= '0' && c <= '9';
}

static bool is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_alnum(char c) {
  return is_alpha(c) || is_digit(c);
}

static TokenizedLine tokenize_line(const char *line, Arena *str_arena) {
  TokenizedLine tokenized_line;
  tokenized_line.count = 0;

  const char *current = line;

  while (current && tokenized_line.count < MAX_TOKENS_PER_LINE) {
    while (is_space(*current)) {
      current++;
    }

    Token *token = &tokenized_line.tokens[tokenized_line.count];
    tokenized_line.count++;

    if (is_eol(*current)) {
      token->type = TOKEN_EOL;
      token->lexeme = "<EOL>";

      break;
    }

    switch (*current) {
    case '.':
      token->type = TOKEN_DOT;
      token->lexeme = ".";

      current++;
      continue;
    case ',':
      token->type = TOKEN_COMMA;
      token->lexeme = ",";
      current++;
      continue;
    case ':':
      token->type = TOKEN_COLON;
      token->lexeme = ":";
      current++;
      continue;

    case 'R':
    case 'r':
      if (is_digit(*(current + 1))) {
        char *endptr;
        token->reg_num = (uint8_t)strtol(current + 1, &endptr, 10);

        token->type = TOKEN_REGISTER;
        token->lexeme = arena_strndup(str_arena, current, endptr - current);

        current = endptr;
        continue;
      }
      break;
    }

    if (is_digit(*current) || (*current == '-' && is_digit(*(current + 1)))) {
      char *endptr;
      token->imm_value = (int32_t)strtol(current, &endptr, 0);

      token->type = TOKEN_IMMEDIATE;
      token->lexeme = arena_strndup(str_arena, current, endptr - current);

      current = endptr;
      continue;
    }

    if (is_alpha(*current)) {
      const char *start = current;
      while (is_alnum(*current)) {
        current++;
      }

      token->type = TOKEN_IDENTIFIER;
      token->lexeme = arena_strndup(str_arena, start, current - start);

      continue;
    }

    char *err_msg = arena_alloc(str_arena, 32);
    sprintf(err_msg, "Unrecognized character: %c", *current);

    token->type = TOKEN_ERROR;
    token->lexeme = err_msg;

    break;
  }

  return tokenized_line;
}

static void add_line(TokenizedFile *tokenized_file, TokenizedLine line) {
  if (tokenized_file->count >= tokenized_file->capacity) {
    size_t new_capacity = tokenized_file->capacity == 0 ? 256 : tokenized_file->capacity * 2;
    tokenized_file->lines = realloc(tokenized_file->lines, new_capacity * sizeof(TokenizedLine));
    tokenized_file->capacity = new_capacity;
  }
  tokenized_file->lines[tokenized_file->count++] = line;
}

TokenizedFile tokenize_file(AssemblerCtx *ctx, FILE *file) {
  TokenizedFile tokenized_file = {0};

  char line[MAX_LINE_LENGTH];
  uint32_t count = 0;

  while (fgets(line, sizeof(line), file)) {
    TokenizedLine tokenized_line = tokenize_line(line, ctx->str_arena);
    tokenized_line.line_num = count + 1;
    add_line(&tokenized_file, tokenized_line);
    count++;
  }

  return tokenized_file;
}