#include "assembler.h"

#include "../spec.h"
#include "instr.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SYMBOL_TABLE_SIZE 256

void emit_uint32(uint8_t *buffer, uint32_t value) {
  buffer[0] = (value >> 0) & 0xFF;
  buffer[1] = (value >> 8) & 0xFF;
  buffer[2] = (value >> 16) & 0xFF;
  buffer[3] = (value >> 24) & 0xFF;
}

void resolve_labels(AssemblerCtx *ctx, Program *program) {
  ctx->symbol_table = malloc(sizeof(StrTable));
  st_init(ctx->symbol_table, INITIAL_SYMBOL_TABLE_SIZE);

  uint32_t current_address = 0;

  for (size_t i = 0; i < program->count; i++) {
    TypedLine *line = &program->lines[i];

    line->address = current_address;

    switch (line->type) {
    case LINE_DIRECTIVE: {
      if (line->directive_def->layout) {
        line->directive_def->layout(ctx, line, &current_address);
      }
      break;
    }
    case LINE_LABEL: {
      Symbol *existing_symbol = st_get(ctx->symbol_table, line->label);
      if (existing_symbol) {
        fprintf(stderr,
                "[Line %u] Error: Duplicate label '%s', Previously defined at line %u\n",
                line->line_num,
                line->label,
                existing_symbol->line_defined);
        continue;
      }

      Symbol *symbol = malloc(sizeof(Symbol));
      symbol->type = SYM_LABEL;
      symbol->address = current_address;
      symbol->line_defined = line->line_num;

      st_put(ctx->symbol_table, line->label, symbol);
      break;
    }
    case LINE_INSTR:
      current_address += 4; // Assuming each instruction is 4 bytes
      break;
    default:
      break;
    }
  }

  for (size_t i = 0; i < program->count; i++) {
    TypedLine *line = &program->lines[i];

    if (line->type != LINE_INSTR) {
      continue;
    }

    const InstrDef *instr_def = line->instr_def;

    for (size_t j = 0; j < line->arg_count; j++) {
      Operand *arg = &line->args[j];

      if (arg->type == OPT_LABEL) {
        Symbol *symbol = st_get(ctx->symbol_table, arg->label_ref);
        if (!symbol) {
          fprintf(stderr, "[Line %u] Error: Undefined label '%s'\n", line->line_num, arg->label_ref);
          continue;
        }

        arg->type = OPT_IMM;

        if (symbol->type == SYM_CONSTANT) {
          arg->imm_value = symbol->value;
          continue;
        }

        uint8_t opcode = instr_def->opcode;
        uint8_t tag = opcode & MASK_TAG;
        uint8_t fn = (opcode >> POS_FN) & MASK_FN;

        int32_t imm = 0;

        if (tag == TAG_CTRL && fn <= CTRL_JAS) {
          int32_t offset = (int32_t)symbol->address - (int32_t)line->address;
          imm = offset >> 2;
        } else {
          imm = symbol->address;
        }

        if (instr_def->format == FORMAT_U) {
          if (imm < INT19_MIN || imm > INT19_MAX) {
            fprintf(stderr, "[Line %u] Error: Label '%s' address out of range\n", line->line_num, arg->label_ref);
            continue;
          }
        } else {
          if (imm < INT14_MIN || imm > INT14_MAX) {
            fprintf(stderr, "[Line %u] Error: Label '%s' address out of range\n", line->line_num, arg->label_ref);
            continue;
          }
        }

        arg->imm_value = imm;
      }
    }
  }
}

void assemble_lines(AssemblerCtx *ctx, Program *program) {
  for (size_t i = 0; i < program->count; i++) {
    TypedLine *line = &program->lines[i];

    switch (line->type) {
    case LINE_ERROR:
    case LINE_EMPTY:
    case LINE_LABEL:
      break;

    case LINE_DIRECTIVE: {
      if (line->directive_def->emit) {
        line->directive_def->emit(ctx, line);
      }
      break;
    }
    case LINE_INSTR: {
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
      case FORMAT_NONE: {
        instruction = (opcode);
        break;
      }
      default:
        UNREACHABLE("Unknown instruction format");
        break;
      }

      line->instruction = instruction;
      line->byte_count = 4; // Each instruction is 4 bytes

      uint8_t *bytes = malloc(line->byte_count);
      emit_uint32(bytes, instruction);

      line->bytes = bytes;
      break;
    }
    default:
      fprintf(stderr, "[Line %u] Error: Cannot assemble line of type %d\n", line->line_num, line->type);
      break;
    }
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