#include "assembler.h"
#include "instr.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define shift(argc, argv) ((argc)--, (argv)++[0])

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
  ctx.file = file;

  ctx.instr_lut = instr_create_lut();

  Lines lines = parse_file(&ctx);

  for (size_t i = 0; i < lines.count; i++) {
    Line line = lines.lines[i];

    switch (line.type) {
    case LINE_EMPTY:
      printf("Line %u: Empty\n", line.line_num);
      break;
    case LINE_LABEL:
      printf("Line %u: Label: %s\n", line.line_num, line.label);
      break;
    case LINE_INSTR:
      printf("Line %u: Instruction: %s\n", line.line_num, line.mnemonic);
      printf("  rd: %u, r1: %u, r2: %u, imm: %d\n", line.rd, line.r1, line.r2, line.imm);
      break;
    case LINE_DIRECTIVE:
      printf("Line %u: Directive: %s\n", line.line_num, line.mnemonic);
      break;
    default:
      printf("Line %u: Unknown type\n", line.line_num);
      break;
    }
  }

  free_lines(&lines);
  instr_free_lut(ctx.instr_lut);

  fclose(file);

  return 0;
}