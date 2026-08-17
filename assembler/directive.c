#include "directive.h"
#include "assembler.h"

#include <stdlib.h>
#include <string.h>

static const DirectiveDef directive_table[];

static void layout_equ(AssemblerCtx *ctx, TypedLine *line, uint32_t *current_address) {
  const char *label = line->args[0].label_ref;
  int32_t value = line->args[1].imm_value;

  Symbol *existing_symbol = st_get(ctx->symbol_table, label);
  if (existing_symbol) {
    fprintf(stderr,
            "[Line %u] Error: Duplicate label '%s', Previously defined at line %u\n",
            line->line_num,
            label,
            existing_symbol->line_defined);
    return;
  }

  Symbol *symbol = malloc(sizeof(Symbol));
  symbol->type = SYM_CONSTANT;
  symbol->value = value;
  symbol->line_defined = line->line_num;

  st_put(ctx->symbol_table, label, symbol);
}

static void layout_org(AssemblerCtx *ctx, TypedLine *line, uint32_t *current_address) {
  int32_t new_address = line->args[0].imm_value;

  if (new_address < 0) {
    fprintf(stderr, "[Line %u] Error: .org address cannot be negative\n", line->line_num);
    return;
  }

  if (new_address < *current_address) {
    fprintf(stderr, "[Line %u] Error: .org address cannot be less than current address\n", line->line_num);
    return;
  }

  *current_address = (uint32_t)new_address;
}

static void layout_byte(AssemblerCtx *ctx, TypedLine *line, uint32_t *current_address) {
  *current_address += line->arg_count;
}
static void emit_byte(AssemblerCtx *ctx, TypedLine *line) {
  line->byte_count = line->arg_count;

  uint8_t *bytes = malloc(line->byte_count);
  for (size_t i = 0; i < line->arg_count; i++) {
    bytes[i] = (uint8_t)(line->args[i].imm_value & 0xFF);
  }
  line->bytes = bytes;
}

static void layout_word(AssemblerCtx *ctx, TypedLine *line, uint32_t *current_address) {
  *current_address += line->arg_count * 4;
}
static void emit_word(AssemblerCtx *ctx, TypedLine *line) {
  line->byte_count = line->arg_count * 4;

  uint8_t *bytes = malloc(line->byte_count);
  for (size_t i = 0; i < line->arg_count; i++) {
    int32_t value = line->args[i].imm_value;
    emit_uint32(&bytes[i * 4], value);
  }
  line->bytes = bytes;
}

const DirectiveDef *find_directive(const char *name) {
  for (size_t i = 0; directive_table[i].name != NULL; i++) {
    if (strcmp(directive_table[i].name, name) == 0) {
      return &directive_table[i];
    }
  }
  return NULL;
}

static const DirectiveDef directive_table[] = {
  {"equ", layout_equ, NULL, 2, {OPT_LABEL, OPT_IMM}},
  {"org", layout_org, NULL, 1, {OPT_IMM}},
  // {"align", layout_equ, NULL, 1, {OPT_IMM}},
  {"byte", layout_byte, emit_byte, UNLIMITED_ARGS, {OPT_IMM}},
  {"word", layout_word, emit_word, UNLIMITED_ARGS, {OPT_IMM}},
  // {"string", layout_string, emit_string, UNLIMITED_ARGS, {OPT_STRING}},
  // {"asciz", layout_asciz, emit_asciz, UNLIMITED_ARGS, {OPT_STRING}},
  // {"space", layout_space, emit_space, 1, {OPT_IMM}},
  {NULL, NULL, NULL, 0, {0}},
};