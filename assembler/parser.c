#include "parser.h"
#include "instr.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256
#define MAX_TOKENS_PER_LINE 16

// Lexer logic

typedef enum {
  TOKEN_ERROR,
  TOKEN_EOL,

  TOKEN_IDENTIFIER,
  TOKEN_MNEMONIC,
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

  int32_t value;
} Token;

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

static char *to_lower_len(char *s, size_t len) {
  for (char *p = s; p < s + len; p++) {
    *p = tolower(*p);
  }
  return s;
}

static size_t tokenize_line(AssemblerCtx *ctx, Token *tokens, const char *line) {
  size_t token_count = 0;
  const char *current = line;

  while (current && token_count < MAX_TOKENS_PER_LINE) {
    while (is_space(*current)) {
      current++;
    }

    Token *token = &tokens[token_count];
    token_count++;

    if (is_eol(*current)) {
      token->type = TOKEN_EOL;
      break;
    }

    token->lexeme = current;
    token->lexeme_length = 1;

    switch (*current) {
    case '.':
      token->type = TOKEN_DOT;
      current++;
      continue;
    case ',':
      token->type = TOKEN_COMMA;
      current++;
      continue;
    case ':':
      token->type = TOKEN_COLON;
      current++;
      continue;

    case 'R':
    case 'r':
      if (is_digit(*(current + 1))) {
        char *endptr;
        token->value = (int32_t)strtol(current + 1, &endptr, 10);

        token->type = TOKEN_REGISTER;
        token->lexeme_length = endptr - current;

        current = endptr;
        continue;
      }
      break;
    }

    if (is_digit(*current) || (*current == '-' && is_digit(*(current + 1)))) {
      char *endptr;
      token->value = (int32_t)strtol(current, &endptr, 0);

      token->type = TOKEN_IMMEDIATE;
      token->lexeme_length = endptr - current;
      current = endptr;
      continue;
    }

    if (is_alpha(*current)) {
      const char *start = current;
      while (is_alnum(*current)) {
        current++;
      }

      size_t length = current - start;

      // Normalize to lowercase
      char norm[16];
      size_t norm_len = (length < sizeof(norm) - 1) ? length : sizeof(norm) - 1;

      for (size_t i = 0; i < norm_len; i++) {
        norm[i] = tolower(start[i]);
      }
      norm[norm_len] = '\0';

      InstrDef *instr_def = instr_lookup(ctx->instr_lut, norm);
      if (instr_def) {
        token->type = TOKEN_MNEMONIC;
        token->lexeme = instr_def->mnemonic;
      } else {
        token->type = TOKEN_IDENTIFIER;
        token->lexeme = start;
      }

      token->lexeme_length = length;
      continue;
    }
  }

  return token_count;
}

// Parser logic

static void add_line(Lines *lines, Line line) {
  if (lines->count >= lines->capacity) {
    size_t new_capacity = lines->capacity == 0 ? 256 : lines->capacity * 2;
    lines->lines = realloc(lines->lines, new_capacity * sizeof(Line));
    lines->capacity = new_capacity;
  }
  lines->lines[lines->count++] = line;
}

static bool match_tokens(Token *tokens, uint32_t line_num, const char *pattern, ...) {
  va_list args;
  va_start(args, pattern);

  size_t token_idx = 1; // Start from the second token, as the first is the mnemonic

  while (*pattern) {
    Token *token = &tokens[token_idx];

    switch (*pattern) {
    case 'r':
      if (token->type != TOKEN_REGISTER) {
        goto error;
      }
      *va_arg(args, uint8_t *) = (uint8_t)token->value;
      break;

    case 'i':
      if (token->type != TOKEN_IMMEDIATE) {
        goto error;
      }
      *va_arg(args, int32_t *) = token->value;
      break;

    case 'l':
      if (token->type != TOKEN_IDENTIFIER) {
        goto error;
      }
      *va_arg(args, const char **) = token->lexeme;
      break;

    case ',':
      if (token->type != TOKEN_COMMA) {
        goto error;
      }
      break;

    default:
      goto error;
    }

    token_idx++;
    pattern++;
  }

  if (tokens[token_idx].type != TOKEN_EOL) {
    goto error;
  }

  va_end(args);
  return true;

error:
  va_end(args);

  const char *pattern_str = pattern;
  switch (*pattern_str) {
  case 'r':
    pattern_str = "register";
    break;
  case 'i':
    pattern_str = "immediate";
    break;
  case 'l':
    pattern_str = "label";
    break;
  case ',':
    pattern_str = "','";
    break;
  case '\0':
    pattern_str = "end of line";
    break;
  }

  fprintf(stderr,
          "[Line %u] Syntax error: Expected %s near '%.*s'\n",
          line_num,
          pattern_str,
          (int)tokens[token_idx].lexeme_length,
          tokens[token_idx].lexeme);

  return false;
}

static Line parse_line(AssemblerCtx *ctx, Token *tokens, size_t token_count, uint32_t line_num) {
  Line parsed_line = {
    .line_num = line_num,
    .type = LINE_ERROR,
  };

  if (token_count == 0) {
    parsed_line.type = LINE_EMPTY;
    return parsed_line;
  }

  switch (tokens[0].type) {
  case TOKEN_IDENTIFIER:
    if (token_count > 1 && tokens[1].type == TOKEN_COLON) {
      parsed_line.type = LINE_LABEL;
      parsed_line.label = strndup(tokens[0].lexeme, tokens[0].lexeme_length);
      return parsed_line;
    }
    break;

  case TOKEN_MNEMONIC: {
    parsed_line.type = LINE_INSTR;
    parsed_line.mnemonic = tokens[0].lexeme;

    InstrDef *instr_def = instr_lookup(ctx->instr_lut, tokens[0].lexeme);

    switch (instr_def->format) {
    case FORMAT_R:
      match_tokens(tokens, line_num, "r,r,r", &parsed_line.rd, &parsed_line.r1, &parsed_line.r2);
      break;

    case FORMAT_I:
    case FORMAT_IS:
      match_tokens(tokens, line_num, "r,r,i", &parsed_line.rd, &parsed_line.r1, &parsed_line.imm);
      break;

    case FORMAT_S:
      match_tokens(tokens, line_num, "r,r,i", &parsed_line.r2, &parsed_line.r1, &parsed_line.imm);
      break;

    case FORMAT_U:
      match_tokens(tokens, line_num, "r,i", &parsed_line.rd, &parsed_line.imm);
      break;

    default:
      fprintf(stderr, "[Line %u] Error: Unsupported instruction format\n", line_num);
      break;
    }
    break;
  }

  default:
    fprintf(stderr, "[Line %u] Error: Unexpected token type\n", line_num);
    return parsed_line;
  }

  return parsed_line;
}

Lines parse_file(AssemblerCtx *ctx) {
  Lines lines = {0};
  Token tokens[MAX_TOKENS_PER_LINE];

  uint32_t line_num = 0;
  char src_line[MAX_LINE_LENGTH];

  while (fgets(src_line, MAX_LINE_LENGTH, ctx->file)) {
    line_num++;

    size_t token_count = tokenize_line(ctx, tokens, src_line);
    Line parsed_line = parse_line(ctx, tokens, token_count, line_num);

    add_line(&lines, parsed_line);
  }

  return lines;
}

void free_lines(Lines *lines) {
  for (size_t i = 0; i < lines->count; i++) {
    if (lines->lines[i].type == LINE_LABEL) {
      free((void *)lines->lines[i].label);
    }
  }
  free(lines->lines);
  lines->lines = NULL;
  lines->count = 0;
  lines->capacity = 0;
}
