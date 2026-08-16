#ifndef DIRECTIVE_H
#define DIRECTIVE_H

#include "assembler.h"
#include <stdint.h>

#define UNLIMITED_ARGS -1

const DirectiveDef *find_directive(const char *name);

#endif // DIRECTIVE_H