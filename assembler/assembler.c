#include "assembler.h"

#include "../spec.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_LABEL_TABLE_SIZE 256

static void emit_instr(InstrBuffer *buffer, uint32_t instr) {
  if (buffer->count >= buffer->capacity) {
    size_t new_capacity = buffer->capacity == 0 ? 256 : buffer->capacity * 2;
    buffer->instr = realloc(buffer->instr, new_capacity * sizeof(uint32_t));
    buffer->capacity = new_capacity;
  }
  buffer->instr[buffer->count++] = instr;
}

void resolve_labels(AssemblerCtx *ctx, Lines *lines) {
  StrTable *label_table = malloc(sizeof(StrTable));
  st_init(label_table, INITIAL_LABEL_TABLE_SIZE);

  uint32_t current_address = 0;
  for (size_t i = 0; i < lines->count; i++) {
    TypedLine line = lines->lines[i];

    if (line.type != LINE_LABEL) {
      lines->lines[i].address = current_address;
      current_address++;
      continue;
    }

    st_put(label_table, line.label, (void *)(uintptr_t)current_address);
  }

  for (size_t i = 0; i < lines->count; i++) {
    TypedLine *line = &lines->lines[i];

    if (line->type != LINE_INSTR) {
      continue;
    }

    for (size_t j = 0; j < line->arg_count; j++) {
      Operand *arg = &line->args[j];

      if (arg->type == OPT_LABEL) {
        void *label_addr_ptr = st_get(label_table, arg->label);
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

  st_free(label_table);
  free(label_table);
}

InstrBuffer assemble_lines(AssemblerCtx *ctx, Lines *lines) {
  InstrBuffer buffer = {0};

  for (size_t i = 0; i < lines->count; i++) {
    TypedLine line = lines->lines[i];

    if (line.type != LINE_INSTR) {
      continue;
    }

    char *normalized_mnemonic = arena_strdup(ctx->str_arena, line.mnemonic);

    for (size_t j = 0; j < strlen(normalized_mnemonic); j++) {
      normalized_mnemonic[j] = tolower(normalized_mnemonic[j]);
    }

    const InstrDef *instr_def = instr_lookup(ctx->instr_lut, normalized_mnemonic);
    assert(instr_def != NULL && "Instruction definition should exist after parsing");

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
      fprintf(stderr, "[Line %u] Error: Unsupported instruction format for '%s'\n", line.line_num, line.mnemonic);
      continue;
    }

    emit_instr(&buffer, instruction);
  }

  return buffer;
}