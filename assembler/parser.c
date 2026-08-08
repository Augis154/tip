#include "parser.h"
#include "instr.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void add_line(Lines *lines, TypedLine line) {
  if (lines->count >= lines->capacity) {
    size_t new_capacity = lines->capacity == 0 ? 256 : lines->capacity * 2;
    lines->lines = realloc(lines->lines, new_capacity * sizeof(TypedLine));
    lines->capacity = new_capacity;
  }
  lines->lines[lines->count++] = line;
}

static TypedLine error_line(uint32_t line_num, const char *fmt, ...) {
  TypedLine line = {0};
  line.line_num = line_num;
  line.type = LINE_ERROR;

  va_list args;
  va_start(args, fmt);

  fprintf(stderr, "[Line %u] Error: ", line_num);
  vfprintf(stderr, fmt, args);

  va_end(args);

  return line;
}

static bool is_token_allowed(TokenType type, uint8_t allowed_mask) {
  uint8_t actual = 0;

  switch (type) {
  case TOKEN_REGISTER:
    actual = OPT_REG;
    break;

  case TOKEN_IMMEDIATE:
    actual = OPT_IMM;
    break;

  case TOKEN_IDENTIFIER:
    actual = OPT_LABEL;
    break;

  default:
    actual = OPT_NONE;
    break;
  }

  return (actual & allowed_mask) != 0;
}

static TypedLine parse_instruction(AssemblerCtx *ctx, TokenizedLine *tokenized_line) {
  TypedLine typed_line = {0};
  typed_line.line_num = tokenized_line->line_num;

  size_t token_idx = 0;

  const char *mnemonic = tokenized_line->tokens[token_idx].lexeme;
  char *normalized_mnemonic = arena_strndup(ctx->str_arena, mnemonic, strlen(mnemonic));

  for (size_t i = 0; i < strlen(mnemonic); i++) {
    normalized_mnemonic[i] = tolower(normalized_mnemonic[i]);
  }

  const InstrDef *instr_def = instr_lookup(ctx->instr_lut, normalized_mnemonic);
  if (!instr_def) {
    return error_line(tokenized_line->line_num, "Unknown instruction '%s'\n", normalized_mnemonic);
  }

  typed_line.type = LINE_INSTR;
  typed_line.mnemonic = mnemonic;

  token_idx++;

  const FormatRule *format_rule = format_rule_lookup(instr_def->format);

  for (size_t i = 0; i < format_rule->op_count; i++) {
    if (token_idx >= tokenized_line->count) {
      return error_line(tokenized_line->line_num, "Missing operand %zu for instruction '%s'\n", i + 1, mnemonic);
    }

    Token *token = &tokenized_line->tokens[token_idx];

    if (!is_token_allowed(token->type, format_rule->types[i])) {
      return error_line(
        tokenized_line->line_num, "Invalid type for operand %zu of instruction '%s'\n", i + 1, mnemonic);
    }

    switch (token->type) {
    case TOKEN_REGISTER:
      typed_line.args[i] = (Operand){.type = OPT_REG, .reg_num = token->reg_num};
      break;

    case TOKEN_IMMEDIATE:
      typed_line.args[i] = (Operand){.type = OPT_IMM, .imm_value = token->imm_value};
      break;

    case TOKEN_IDENTIFIER:
      typed_line.args[i] = (Operand){.type = OPT_LABEL, .label = token->lexeme};
      break;

    default:
      break;
    }

    token_idx++;
    typed_line.arg_count++;

    if (i < format_rule->op_count - 1) {
      if (token_idx >= tokenized_line->count || tokenized_line->tokens[token_idx].type != TOKEN_COMMA) {
        return error_line(
          tokenized_line->line_num, "Missing comma after operand %zu for instruction '%s'\n", i + 1, mnemonic);
      }
      token_idx++; // Skip the comma
    }
  }

  if (token_idx < tokenized_line->count && tokenized_line->tokens[token_idx].type != TOKEN_EOL) {
    return error_line(tokenized_line->line_num, "Unexpected tokens after instruction '%s'\n", mnemonic);
  }

  return typed_line;
}

static TypedLine parse_line(AssemblerCtx *ctx, TokenizedLine *tokenized_line) {
  if (tokenized_line->count == 0 || (tokenized_line->count == 1 && tokenized_line->tokens[0].type == TOKEN_EOL)) {
    TypedLine empty_line = {
      .line_num = tokenized_line->line_num,
      .type = LINE_EMPTY,
    };
    return empty_line;
  }

  Token *tokens = tokenized_line->tokens;

  if (tokens[0].type == TOKEN_IDENTIFIER && tokenized_line->count > 1 && tokens[1].type == TOKEN_COLON) {
    TypedLine label_line = {
      .line_num = tokenized_line->line_num,
      .type = LINE_LABEL,
      .label = tokens[0].lexeme,
    };
    return label_line;
  }

  if (tokens[0].type == TOKEN_IDENTIFIER) {
    return parse_instruction(ctx, tokenized_line);
  }

  return error_line(tokenized_line->line_num, "Unexpected token type\n");
}

Lines parse_lines(AssemblerCtx *ctx, TokenizedLine *tokenized_lines, size_t line_count) {
  Lines lines = {0};

  for (size_t i = 0; i < line_count; i++) {
    TypedLine parsed_line = parse_line(ctx, &tokenized_lines[i]);

    add_line(&lines, parsed_line);
  }

  return lines;
}

void free_lines(Lines *lines) {
  free(lines->lines);
  lines->lines = NULL;
  lines->count = 0;
  lines->capacity = 0;
}
