#include "parser.h"
#include "arena.h"
#include "directive.h"
#include "instr.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static TypedLine error_line(uint32_t line_num, const char *fmt, ...) {
  TypedLine line = {
    .type = LINE_ERROR,
    .line_num = line_num,
  };

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
  TypedLine typed_line = {
    .type = LINE_INSTR,
    .line_num = tokenized_line->line_num,
  };

  size_t token_idx = 0;

  ArenaMark mark = arena_mark(ctx->str_arena);

  const char *mnemonic = tokenized_line->tokens[token_idx].lexeme;
  char *normalized_mnemonic = arena_strdup(ctx->str_arena, mnemonic);

  for (size_t i = 0; i < strlen(mnemonic); i++) {
    normalized_mnemonic[i] = tolower(normalized_mnemonic[i]);
  }

  const InstrDef *instr_def = instr_lookup(ctx->instr_lut, normalized_mnemonic);
  if (!instr_def) {
    return error_line(tokenized_line->line_num, "Unknown instruction '%s'\n", normalized_mnemonic);
  }

  arena_rollback(ctx->str_arena, mark);

  typed_line.instr_def = instr_def;

  token_idx++;

  const FormatRule *format_rule = format_rule_lookup(instr_def->format);

  for (size_t i = 0; i < format_rule->arg_count; i++) {
    if (token_idx >= tokenized_line->count) {
      return error_line(tokenized_line->line_num, "Missing operand %zu for instruction '%s'\n", i + 1, mnemonic);
    }

    Token *token = &tokenized_line->tokens[token_idx];

    if (!is_token_allowed(token->type, format_rule->arg_types[i])) {
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
      typed_line.args[i] = (Operand){.type = OPT_LABEL, .label_ref = token->lexeme};
      break;

    default:
      break;
    }

    token_idx++;
    typed_line.arg_count++;

    if (i < format_rule->arg_count - 1) {
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

static TypedLine parse_directive(AssemblerCtx *ctx, TokenizedLine *tokenized_line) {
  TypedLine typed_line = {
    .type = LINE_DIRECTIVE,
    .line_num = tokenized_line->line_num,
  };

  size_t token_idx = 1; // Skip the DOT token

  ArenaMark mark = arena_mark(ctx->str_arena);

  const char *dir_name = tokenized_line->tokens[token_idx].lexeme;
  char *normalized_name = arena_strdup(ctx->str_arena, dir_name);

  for (size_t i = 0; i < strlen(dir_name); i++) {
    normalized_name[i] = tolower(normalized_name[i]);
  }

  const DirectiveDef *directive = find_directive(normalized_name);
  if (!directive) {
    return error_line(tokenized_line->line_num, "Unknown directive '%s'\n", dir_name);
  }

  arena_rollback(ctx->str_arena, mark);

  typed_line.type = LINE_DIRECTIVE;
  typed_line.directive_def = directive;

  token_idx++;

  if (directive->arg_count == UNLIMITED_ARGS) {
    while (token_idx < tokenized_line->count && tokenized_line->tokens[token_idx].type != TOKEN_EOL) {
      Token *token = &tokenized_line->tokens[token_idx];

      if (!is_token_allowed(token->type, directive->arg_types[0])) {
        return error_line(tokenized_line->line_num,
                          "Invalid type for operand %zu of directive '%s'\n",
                          typed_line.arg_count + 1,
                          dir_name);
      }

      switch (token->type) {
      case TOKEN_REGISTER:
        typed_line.args[typed_line.arg_count] = (Operand){.type = OPT_REG, .reg_num = token->reg_num};
        break;

      case TOKEN_IMMEDIATE:
        typed_line.args[typed_line.arg_count] = (Operand){.type = OPT_IMM, .imm_value = token->imm_value};
        break;

      case TOKEN_IDENTIFIER:
        typed_line.args[typed_line.arg_count] = (Operand){.type = OPT_LABEL, .label_ref = token->lexeme};
        break;

      default:
        break;
      }

      typed_line.arg_count++;
      token_idx++;

      if (token_idx < tokenized_line->count && tokenized_line->tokens[token_idx].type == TOKEN_COMMA) {
        token_idx++; // Skip the comma
      }
    }
  } else {
    for (size_t i = 0; i < directive->arg_count; i++) {
      if (token_idx >= tokenized_line->count || tokenized_line->tokens[token_idx].type == TOKEN_EOL) {
        return error_line(tokenized_line->line_num, "Missing operand %zu for directive '%s'\n", i + 1, dir_name);
      }

      Token *token = &tokenized_line->tokens[token_idx];

      if (!is_token_allowed(token->type, directive->arg_types[i])) {
        return error_line(
          tokenized_line->line_num, "Invalid type for operand %zu of directive '%s'\n", i + 1, dir_name);
      }

      switch (token->type) {
      case TOKEN_REGISTER:
        typed_line.args[i] = (Operand){.type = OPT_REG, .reg_num = token->reg_num};
        break;

      case TOKEN_IMMEDIATE:
        typed_line.args[i] = (Operand){.type = OPT_IMM, .imm_value = token->imm_value};
        break;
      case TOKEN_IDENTIFIER:
        typed_line.args[i] = (Operand){.type = OPT_LABEL, .label_ref = token->lexeme};
        break;
      default:
        break;
      }

      typed_line.arg_count++;
      token_idx++;

      if (token_idx < tokenized_line->count && tokenized_line->tokens[token_idx].type == TOKEN_COMMA) {
        token_idx++; // Skip the comma
      }
    }
  }

  return typed_line;
}

static TypedLine parse_label(AssemblerCtx *ctx, TokenizedLine *tokenized_line) {
  TypedLine typed_line = {
    .type = LINE_LABEL,
    .line_num = tokenized_line->line_num,
  };

  size_t token_idx = 0;

  Token *label_token = &tokenized_line->tokens[token_idx];
  typed_line.type = LINE_LABEL;
  typed_line.label = label_token->lexeme;

  token_idx += 2; // Skip the label and colon

  if (token_idx < tokenized_line->count && tokenized_line->tokens[token_idx].type != TOKEN_EOL) {
    return error_line(tokenized_line->line_num, "Unexpected tokens after label '%s'\n", typed_line.label);
  }

  return typed_line;
}

static TypedLine parse_line(AssemblerCtx *ctx, TokenizedLine *tokenized_line) {
  TypedLine typed_line = {
    .line_num = tokenized_line->line_num,
    .type = LINE_EMPTY,
  };

  if (tokenized_line->count == 0) {
    return typed_line;
  }

  Token *tokens = tokenized_line->tokens;
  size_t token_idx = 0;

  switch (tokens[token_idx].type) {
  case TOKEN_EOL:
    return typed_line;

  case TOKEN_ERROR:
    return error_line(tokenized_line->line_num, "Lexical error: %s\n", tokens[token_idx].lexeme);

  case TOKEN_DOT:
    if (tokens[token_idx + 1].type != TOKEN_IDENTIFIER) {
      return error_line(tokenized_line->line_num, "Expected directive name after '.'\n");
    }

    return parse_directive(ctx, tokenized_line);

  case TOKEN_IDENTIFIER:
    if (tokens[token_idx + 1].type == TOKEN_COLON) {
      return parse_label(ctx, tokenized_line);
    }
    return parse_instruction(ctx, tokenized_line);

  default:
    return error_line(tokenized_line->line_num, "Unexpected token type\n");
  }
}

static void add_line(Program *program, TypedLine line) {
  if (program->count >= program->capacity) {
    size_t new_capacity = program->capacity == 0 ? 256 : program->capacity * 2;
    program->lines = realloc(program->lines, new_capacity * sizeof(TypedLine));
    program->capacity = new_capacity;
  }
  program->lines[program->count++] = line;
}

Program parse_lines(AssemblerCtx *ctx, TokenizedFile *tokenized_file) {
  Program program = {0};

  for (size_t i = 0; i < tokenized_file->count; i++) {
    TypedLine parsed_line = parse_line(ctx, &tokenized_file->lines[i]);

    add_line(&program, parsed_line);
  }

  return program;
}

void free_program(Program *program) {
  for (size_t i = 0; i < program->count; i++) {
    TypedLine *line = &program->lines[i];
    if (line->type == LINE_DIRECTIVE || line->type == LINE_INSTR) {
      free(line->bytes);
      line->bytes = NULL;
      line->byte_count = 0;
    }
  }

  free(program->lines);
  program->lines = NULL;
  program->count = 0;
  program->capacity = 0;
}
