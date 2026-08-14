#include "assembler.h"

#include "../spec.h"
#include "instr.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SYMBOL_TABLE_SIZE 256

void resolve_labels(AssemblerCtx *ctx, Program *program) {
  ctx->symbol_table = malloc(sizeof(StrTable));
  st_init(ctx->symbol_table, INITIAL_SYMBOL_TABLE_SIZE);

  uint32_t current_address = 0;
  for (size_t i = 0; i < program->count; i++) {
    TypedLine line = program->lines[i];

    if (line.type != LINE_LABEL) {
      program->lines[i].address = current_address;
      current_address++;
      continue;
    }

    Symbol *existing_symbol = st_get(ctx->symbol_table, line.label);
    if (existing_symbol) {
      fprintf(stderr,
              "[Line %u] Error: Duplicate label '%s', Previously defined at line %u\n",
              line.line_num,
              line.label,
              existing_symbol->line_defined);
      continue;
    }

    Symbol *symbol = malloc(sizeof(Symbol));
    symbol->address = current_address;
    symbol->line_defined = line.line_num;

    st_put(ctx->symbol_table, line.label, symbol);
  }

  for (size_t i = 0; i < program->count; i++) {
    TypedLine *line = &program->lines[i];

    if (line->type != LINE_INSTR) {
      continue;
    }

    for (size_t j = 0; j < line->arg_count; j++) {
      Operand *arg = &line->args[j];

      if (arg->type == OPT_LABEL) {
        Symbol *symbol = st_get(ctx->symbol_table, arg->label_ref);
        if (!symbol) {
          fprintf(stderr, "[Line %u] Error: Undefined label '%s'\n", line->line_num, arg->label_ref);
          continue;
        }

        int32_t offset = symbol->address - (int32_t)line->address;

        if (offset < INT14_MIN || offset > INT14_MAX) {
          fprintf(stderr, "[Line %u] Error: Label '%s' address out of range\n", line->line_num, arg->label_ref);
          continue;
        }

        arg->type = OPT_IMM;
        arg->imm_value = offset;
      }
    }
  }
}

void assemble_lines(AssemblerCtx *ctx, Program *program) {
  for (size_t i = 0; i < program->count; i++) {
    TypedLine *line = &program->lines[i];

    if (line->type != LINE_INSTR) {
      continue;
    }

    const InstrDef *instr_def = line->instr_def;

    uint32_t instruction = 0;
    uint8_t opcode = instr_def->opcode & MASK_OP;

    switch (instr_def->format) {
    case FORMAT_R: {
      uint8_t rd = line->args[0].reg_num & MASK_REG;
      uint8_t r1 = line->args[1].reg_num & MASK_REG;
      uint8_t r2 = line->args[2].reg_num & MASK_REG;

      uint8_t ext = instr_def->ext & MASK_FN9;

      instruction = (opcode) | (rd << POS_RD) | (r1 << POS_R1) | (r2 << POS_R2) | (ext << POS_FN9);
      break;
    }
    case FORMAT_I: {
      uint8_t rd = line->args[0].reg_num & MASK_REG;
      uint8_t r1 = line->args[1].reg_num & MASK_REG;
      int32_t imm = line->args[2].imm_value & MASK_IMM14;

      instruction = (opcode) | (rd << POS_RD) | (r1 << POS_R1) | (imm << POS_IMM14);
      break;
    }
    case FORMAT_IS: {
      uint8_t rd = line->args[0].reg_num & MASK_REG;
      uint8_t r1 = line->args[1].reg_num & MASK_REG;
      int32_t shamt = line->args[2].imm_value & MASK_SHAMT;

      uint16_t ext = instr_def->ext & MASK_FN9;

      instruction = (opcode) | (rd << POS_RD) | (r1 << POS_R1) | (shamt << POS_SHAMT) | (ext << POS_FN9);
      break;
    }
    case FORMAT_S: {
      uint8_t r1 = line->args[0].reg_num & MASK_REG;
      uint8_t r2 = line->args[1].reg_num & MASK_REG;
      int32_t imm = line->args[2].imm_value & MASK_IMM14;

      uint16_t imm4_0 = imm & MASK_IMM4_0;
      uint16_t imm13_5 = (imm >> 5) & MASK_IMM13_5;

      instruction = (opcode) | (imm4_0 << POS_IMM4_0) | (r1 << POS_R1) | (r2 << POS_R2) | (imm13_5 << POS_IMM13_5);
      break;
    }
    case FORMAT_U: {
      uint8_t rd = line->args[0].reg_num & MASK_REG;
      int32_t imm = line->args[1].imm_value & MASK_IMM19;

      instruction = (opcode) | (rd << POS_RD) | (imm << POS_IMM19);
      break;
    }
    default:
      UNREACHABLE("Unknown instruction format");
      break;
    }

    program->lines[i].instr = instruction;
  }
}

void free_symbols(AssemblerCtx *ctx) {
  StrTable *symbol_table = ctx->symbol_table;

  for (size_t i = 0; i < symbol_table->size; i++) {
    STNode *node = symbol_table->buckets[i];
    while (node) {
      Symbol *symbol = (Symbol *)node->value;
      free(symbol);
      node = node->next;
    }
  }

  st_free(symbol_table);
  free(symbol_table);
}