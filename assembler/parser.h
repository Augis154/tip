#ifndef PARSER_H
#define PARSER_H

#include "assembler.h"
#include "instr.h"
#include "lexer.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

Program parse_lines(AssemblerCtx *ctx, TokenizedFile *tokenized_file);
void free_program(Program *program);

#endif // PARSER_H