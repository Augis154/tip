#include "instr.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define shift(argc, argv) ((argc)--, (argv)++[0])

static char *to_lower(char *s) {
  for (char *p = s; *p; p++) {
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

  instr_init_lut();

  char line[256];

  while (fgets(line, sizeof(line), file)) {
    if (line[0] == '\n' || line[0] == ';') {
      continue;
    }

    char *token = strtok(line, ", ");
    token = to_lower(token);

    InstrDef *instr_def = instr_get_def(token);

    if (!instr_def) {
      fprintf(stderr, "Unknown instruction: %s\n", token);
      continue;
    }

    printf("Instruction: %s\n", instr_def->mnemonic);
  }

  instr_free_lut();

  fclose(file);

  return 0;
}