#include "lexer.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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
      token->lexeme_length = 5;

      break;
    }

    switch (*current) {
    case '.':
      token->type = TOKEN_DOT;
      token->lexeme = ".";
      token->lexeme_length = 1;

      current++;
      continue;
    case ',':
      token->type = TOKEN_COMMA;
      token->lexeme = ",";
      token->lexeme_length = 1;

      current++;
      continue;
    case ':':
      token->type = TOKEN_COLON;
      token->lexeme = ":";
      token->lexeme_length = 1;

      current++;
      continue;

    case 'R':
    case 'r':
      if (is_digit(*(current + 1))) {
        char *endptr;
        token->reg_num = (uint8_t)strtol(current + 1, &endptr, 10);

        token->type = TOKEN_REGISTER;
        token->lexeme = arena_strndup(str_arena, current, endptr - current);
        token->lexeme_length = endptr - current;

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
      token->lexeme_length = endptr - current;

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
      token->lexeme_length = current - start;

      continue;
    }

    token->type = TOKEN_ERROR;
    token->lexeme = arena_strndup(str_arena, current, 1);
    token->lexeme_length = 1;

    break;
  }

  return tokenized_line;
}

void tokenize_file(FILE *file, Arena *str_arena, TokenizedLine *tokenized_lines, size_t *line_count) {
  char line[MAX_LINE_LENGTH];
  uint32_t count = 0;

  while (fgets(line, sizeof(line), file)) {
    tokenized_lines[count] = tokenize_line(line, str_arena);
    tokenized_lines[count].line_num = count + 1;
    count++;
  }

  *line_count = count;
}