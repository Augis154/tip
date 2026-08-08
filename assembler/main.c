#include "assembler.h"
#include "instr.h"
#include "lexer.h"
#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define shift(argc, argv) ((argc)--, (argv)++[0])

static char *to_lower_len(char *s, size_t len) {
  for (char *p = s; p < s + len; p++) {
    *p = tolower(*p);
  }
  return s;
}

int main(int argc, char *argv[]) {
  char *prg_name = shift(argc, argv);

  if (argc < 1) {
    fprintf(stderr, "Usage: %s <file>\n", prg_name);
    return 1;
  }

  char *filename = shift(argc, argv);

  FILE *file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Error opening file: %s\n", filename);
    return 1;
  }

  AssemblerCtx ctx;

  ctx.filename = filename;

  ctx.instr_lut = instr_create_lut();

  TokenizedLine tokenized_lines[MAX_TOKENS_PER_LINE];
  size_t line_count = 0;

  Arena str_arena;

  ctx.str_arena = &str_arena;

  tokenize_file(file, &str_arena, tokenized_lines, &line_count);

  fclose(file);

  // for (size_t i = 0; i < line_count; i++) {
  //   TokenizedLine *tokenized_line = &tokenized_lines[i];

  //   printf("Line %u: ", tokenized_line->line_num);
  //   for (size_t j = 0; j < tokenized_line->count; j++) {
  //     Token *token = &tokenized_line->tokens[j];
  //     printf("[%.*s] ", (int)token->lexeme_length, token->lexeme);
  //   }
  //   printf("\n");
  // }

  Lines lines = parse_lines(&ctx, tokenized_lines, line_count);

  for (size_t i = 0; i < lines.count; i++) {
    TypedLine line = lines.lines[i];

    switch (line.type) {
    case LINE_ERROR:
      printf("Line %u: Error\n", line.line_num);
      break;

    case LINE_EMPTY:
      printf("Line %u: Empty\n", line.line_num);
      break;

    case LINE_LABEL:
      printf("Line %u: Label: %s\n", line.line_num, line.label);
      break;
    case LINE_INSTR:
      printf("Line %u: Instruction: %s", line.line_num, line.mnemonic);

      for (size_t j = 0; j < line.arg_count; j++) {
        Operand arg = line.args[j];
        switch (arg.type) {
        case OPT_REG:
          printf(" [R%u]", arg.reg_num);
          break;
        case OPT_IMM:
          printf(" [Immediate %d]", arg.imm_value);
          break;
        case OPT_LABEL:
          printf(" [Label %s]", arg.label);
          break;
        default:
          break;
        }
      }

      printf("\n");

      break;
    case LINE_DIRECTIVE:
      printf("Line %u: Directive: %s\n", line.line_num, line.mnemonic);
      break;
    default:
      printf("Line %u: Unknown line type\n", line.line_num);
      break;
    }
  }

  free_lines(&lines);

  arena_free(&str_arena);

  instr_free_lut(ctx.instr_lut);

  return 0;
}