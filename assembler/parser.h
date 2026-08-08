#ifndef PARSER_H
#define PARSER_H

#include "assembler.h"
#include "instr.h"
#include "lexer.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

Lines parse_lines(AssemblerCtx *ctx, TokenizedFile *tokenized_file);
void free_lines(Lines *lines);

#endif // PARSER_H