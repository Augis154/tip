#include "assembler.h"
#include "instr.h"
#include "lexer.h"
#include "parser.h"

#include <ctype.h>
#include <stdint.h>
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

  FILE *input_file = fopen(filename, "rb");
  if (!input_file) {
    fprintf(stderr, "Error opening file: %s\n", filename);
    return 1;
  }

  AssemblerCtx ctx = {0};

  ctx.filename = filename;

  ctx.instr_lut = instr_create_lut();

  Arena str_arena = {0};

  ctx.str_arena = &str_arena;

  TokenizedFile tokenized_file = tokenize_file(input_file, &str_arena);

  fclose(input_file);

  // for (size_t i = 0; i < line_count; i++) {
  //   TokenizedLine *tokenized_line = &tokenized_lines[i];

  //   printf("Line %u: ", tokenized_line->line_num);
  //   for (size_t j = 0; j < tokenized_line->count; j++) {
  //     Token *token = &tokenized_line->tokens[j];
  //     printf("[%.*s] ", (int)token->lexeme_length, token->lexeme);
  //   }
  //   printf("\n");
  // }

  Program program = parse_lines(&ctx, &tokenized_file);

  resolve_labels(&ctx, &program);

  assemble_lines(&ctx, &program);

  FILE *lst_file = fopen("output.lst", "w");
  if (!lst_file) {
    fprintf(stderr, "Error creating listing file\n");
    return 1;
  }

  for (size_t i = 0; i < program.count; i++) {
    TypedLine line = program.lines[i];

    uint8_t *bytes = program.lines[i].bytes;
    size_t byte_count = program.lines[i].byte_count;

    switch (line.type) {
    case LINE_ERROR:
    case LINE_EMPTY:
      continue;

    case LINE_LABEL:
      // fprintf(lst_file, "\n%04X: ", line.address);
      fprintf(lst_file, "\n%s:\n", line.label);
      continue;

    case LINE_DIRECTIVE: {
      // fprintf(lst_file, "%04X: ", line.address);
      fprintf(lst_file, "\n.%s", line.directive_def->name);

      for (size_t j = 0; j < line.arg_count; j++) {
        Operand arg = line.args[j];
        switch (arg.type) {
        case OPT_REG:
          fprintf(lst_file, " r%u", arg.reg_num);
          break;
        case OPT_IMM:
          fprintf(lst_file, " %#x (%d)", (uint32_t)arg.imm_value, (int32_t)arg.imm_value);
          break;
        default:
          break;
        }
      }
      break;
    }

    case LINE_INSTR: {
      uint32_t instruction = program.lines[i].instruction;

      fprintf(lst_file, "%04X: ", line.address);
      fprintf(lst_file, "%08X    ", instruction);
      fprintf(lst_file, "%032b    ", instruction);
      fprintf(lst_file, "%s", line.instr_def->mnemonic);

      for (size_t j = 0; j < line.arg_count; j++) {
        Operand arg = line.args[j];
        switch (arg.type) {
        case OPT_REG:
          fprintf(lst_file, " r%u", arg.reg_num);
          break;
        case OPT_IMM:
          fprintf(lst_file, " %#x (%d)", (uint32_t)arg.imm_value, (int32_t)arg.imm_value);
          break;
        default:
          break;
        }
      }
      break;
    }
    }

    fprintf(lst_file, "\n");
  }

  fclose(lst_file);
  printf("Generated output.lst\n");

  FILE *output_file = fopen("output.bin", "wb");
  if (!output_file) {
    fprintf(stderr, "Error creating output file\n");
    return 1;
  }

  uint32_t bytes_written = 0;
  for (size_t i = 0; i < program.count; i++) {
    TypedLine line = program.lines[i];

    if (line.address > bytes_written) {
      uint32_t padding = line.address - bytes_written;
      uint8_t *zero_bytes = calloc(padding, sizeof(uint8_t));
      fwrite(zero_bytes, sizeof(uint8_t), padding, output_file);
      free(zero_bytes);
      bytes_written += padding;
    }

    uint8_t *bytes = line.bytes;
    size_t byte_count = line.byte_count;
    fwrite(bytes, sizeof(uint8_t), byte_count, output_file);
    bytes_written += byte_count;
  }

  fclose(output_file);
  printf("Output written to output.bin\n");

  // for (size_t i = 0; i < program.count; i++) {
  //   TypedLine line = program.lines[i];

  //   switch (line.type) {
  //   case LINE_ERROR:
  //     printf("Line %u: Error\n", line.line_num);
  //     break;

  //   case LINE_EMPTY:
  //     // printf("Line %u: Empty\n", line.line_num);
  //     break;

  //   case LINE_LABEL:
  //     printf("Line %u: Label: %s\n", line.line_num, line.label);
  //     break;

  //   case LINE_INSTR:
  //     printf("Line %u: Instruction: %s", line.line_num, line.instr_def->mnemonic);

  //     for (size_t j = 0; j < line.arg_count; j++) {
  //       Operand arg = line.args[j];
  //       switch (arg.type) {
  //       case OPT_REG:
  //         printf(" [R%u]", arg.reg_num);
  //         break;
  //       case OPT_IMM:
  //         printf(" [Immediate %#x]", arg.imm_value);
  //         break;
  //       case OPT_LABEL:
  //         printf(" [Label %s]", arg.label_ref);
  //         break;
  //       default:
  //         break;
  //       }
  //     }

  //     printf("\n");

  //     break;
  //   case LINE_DIRECTIVE:
  //     printf("Line %u: Directive: %s", line.line_num, line.directive_def->name);

  //     for (size_t j = 0; j < line.arg_count; j++) {
  //       Operand arg = line.args[j];
  //       switch (arg.type) {
  //       case OPT_REG:
  //         printf(" [R%u]", arg.reg_num);
  //         break;
  //       case OPT_IMM:
  //         printf(" [Immediate %#x]", arg.imm_value);
  //         break;
  //       case OPT_LABEL:
  //         printf(" [Label %s]", arg.label_ref);
  //         break;
  //       default:
  //         break;
  //       }
  //     }

  //     printf("\n");

  //     break;
  //   default:
  //     printf("Line %u: Unknown line type\n", line.line_num);
  //     break;
  //   }
  // }

  free_program(&program);

  arena_free(&str_arena);

  instr_free_lut(ctx.instr_lut);

  return 0;
}