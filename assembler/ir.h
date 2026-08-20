#ifndef IR_H
#define IR_H

#include <stddef.h>
#include <stdint.h>

#define MAX_ARGS 8

typedef struct DirectiveDef DirectiveDef;
typedef struct InstrDef InstrDef;

typedef enum {
  LINE_ERROR,
  LINE_EMPTY,
  LINE_LABEL,
  LINE_INSTR,
  LINE_DIRECTIVE,
} LineType;

typedef enum {
  OPT_NONE = 0b0000,
  OPT_REG = 0b0001,
  OPT_IMM = 0b0010,
  OPT_LABEL = 0b0100,
  OPT_STRING = 0b1000,
} OperandType;

typedef struct {
  OperandType type;

  union {
    uint8_t reg_num;
    int32_t imm_value;
    const char *label_ref;
    const char *string_value;
  };
} Operand;

typedef struct {
  LineType type;

  const char *label;
  union {
    const InstrDef *instr_def;
    const DirectiveDef *directive_def;
  };

  Operand args[MAX_ARGS];
  size_t arg_count;

  uint32_t line_num;
  uint32_t address;
  uint32_t instruction;

  uint8_t *bytes;
  size_t byte_count;
} TypedLine;

#endif // IR_H