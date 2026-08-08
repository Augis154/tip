#include "../spec.h"
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

  StrTable *labels = malloc(sizeof(StrTable));
  st_init(labels, 16);

  uint32_t current_address = 0;
  for (size_t i = 0; i < lines.count; i++) {
    TypedLine line = lines.lines[i];

    if (line.type != LINE_LABEL) {
      lines.lines[i].address = current_address;
      current_address++;
      continue;
    }

    st_put(labels, line.label, (void *)(uintptr_t)current_address);
  }

  for (size_t i = 0; i < lines.count; i++) {
    TypedLine *line = &lines.lines[i];

    if (line->type != LINE_INSTR) {
      continue;
    }

    for (size_t j = 0; j < line->arg_count; j++) {
      Operand *arg = &line->args[j];

      if (arg->type == OPT_LABEL) {
        void *label_addr_ptr = st_get(labels, arg->label);
        if (!label_addr_ptr) {
          fprintf(stderr, "[Line %u] Error: Undefined label '%s'\n", line->line_num, arg->label);
          continue;
        }

        size_t label_addr = (size_t)(uintptr_t)label_addr_ptr;
        int32_t offset = (int32_t)label_addr - (int32_t)line->address;

        if (offset < INT14_MIN || offset > INT14_MAX) {
          fprintf(stderr, "[Line %u] Error: Label '%s' address out of range\n", line->line_num, arg->label);
          continue;
        }

        arg->type = OPT_IMM;
        arg->imm_value = offset;
      }
    }
  }

  FILE *output_file = fopen("output.bin", "wb");
  if (!output_file) {
    fprintf(stderr, "Error creating output file\n");
    return 1;
  }

  for (size_t i = 0; i < lines.count; i++) {
    TypedLine line = lines.lines[i];

    if (line.type != LINE_INSTR) {
      continue;
    }

    char *normalized_mnemonic = arena_strdup(&str_arena, line.mnemonic);

    for (size_t j = 0; j < strlen(normalized_mnemonic); j++) {
      normalized_mnemonic[j] = tolower(normalized_mnemonic[j]);
    }

    const InstrDef *instr_def = instr_lookup(ctx.instr_lut, normalized_mnemonic);

    if (!instr_def) {
      fprintf(stderr, "[Line %u] Error: Unknown instruction '%s'\n", line.line_num, line.mnemonic);
      continue;
    }

    uint32_t instruction = 0;

    switch (instr_def->format) {
    case FORMAT_R: {
      uint8_t opcode = instr_def->opcode & MASK_OP;

      uint8_t rd = line.args[0].reg_num & MASK_REG;
      uint8_t r1 = line.args[1].reg_num & MASK_REG;
      uint8_t r2 = line.args[2].reg_num & MASK_REG;

      uint8_t ext = instr_def->ext & MASK_FN9;

      instruction = (opcode) | (rd << POS_RD) | (r1 << POS_R1) | (r2 << POS_R2) | (ext << POS_FN9);
      break;
    }
    case FORMAT_I: {
      uint8_t opcode = instr_def->opcode & MASK_OP;

      uint8_t rd = line.args[0].reg_num & MASK_REG;
      uint8_t r1 = line.args[1].reg_num & MASK_REG;
      int32_t imm = line.args[2].imm_value & MASK_IMM14;

      instruction = (opcode) | (rd << POS_RD) | (r1 << POS_R1) | (imm << POS_IMM14);
      break;
    }
    case FORMAT_IS: {
      uint8_t opcode = instr_def->opcode & MASK_OP;

      uint8_t rd = line.args[0].reg_num & MASK_REG;
      uint8_t r1 = line.args[1].reg_num & MASK_REG;
      int32_t shamt = line.args[2].imm_value & MASK_SHAMT;

      uint16_t ext = instr_def->ext & MASK_FN9;

      instruction = (opcode) | (rd << POS_RD) | (r1 << POS_R1) | (shamt << POS_SHAMT) | (ext << POS_FN9);
      break;
    }
    case FORMAT_S: {
      uint8_t opcode = instr_def->opcode & MASK_OP;

      uint8_t r1 = line.args[0].reg_num & MASK_REG;
      uint8_t r2 = line.args[1].reg_num & MASK_REG;
      int32_t imm = line.args[2].imm_value & MASK_IMM14;

      uint16_t imm4_0 = imm & MASK_IMM4_0;
      uint16_t imm13_5 = (imm >> 5) & MASK_IMM13_5;

      instruction = (opcode) | (imm4_0 << POS_IMM4_0) | (r1 << POS_R1) | (r2 << POS_R2) | (imm13_5 << POS_IMM13_5);
      break;
    }
    case FORMAT_U: {
      uint8_t opcode = instr_def->opcode & MASK_OP;

      uint8_t rd = line.args[0].reg_num & MASK_REG;
      int32_t imm = line.args[1].imm_value & MASK_IMM19;

      instruction = (opcode) | (rd << POS_RD) | (imm << POS_IMM19);
      break;
    }
    default:
      fprintf(stderr, "[Line %u] Error: Unsupported instruction format for '%s'\n", line.line_num, normalized_mnemonic);
      continue;
    }

    fwrite(&instruction, sizeof(instruction), 1, output_file);
  }

  fclose(output_file);
  printf("Output written to output.bin\n");

  // for (size_t i = 0; i < lines.count; i++) {
  //   TypedLine line = lines.lines[i];

  //   switch (line.type) {
  //   case LINE_ERROR:
  //     printf("Line %u: Error\n", line.line_num);
  //     break;

  //   case LINE_EMPTY:
  //     printf("Line %u: Empty\n", line.line_num);
  //     break;

  //   case LINE_LABEL:
  //     printf("Line %u: Label: %s\n", line.line_num, line.label);
  //     break;
  //   case LINE_INSTR:
  //     printf("Line %u: Instruction: %s", line.line_num, line.mnemonic);

  //     for (size_t j = 0; j < line.arg_count; j++) {
  //       Operand arg = line.args[j];
  //       switch (arg.type) {
  //       case OPT_REG:
  //         printf(" [R%u]", arg.reg_num);
  //         break;
  //       case OPT_IMM:
  //         printf(" [Immediate %d]", arg.imm_value);
  //         break;
  //       case OPT_LABEL:
  //         printf(" [Label %s]", arg.label);
  //         break;
  //       default:
  //         break;
  //       }
  //     }

  //     printf("\n");

  //     break;
  //   case LINE_DIRECTIVE:
  //     printf("Line %u: Directive: %s\n", line.line_num, line.mnemonic);
  //     break;
  //   default:
  //     printf("Line %u: Unknown line type\n", line.line_num);
  //     break;
  //   }
  // }

  free_lines(&lines);

  arena_free(&str_arena);

  instr_free_lut(ctx.instr_lut);

  return 0;
}