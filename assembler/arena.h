#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>
#include <stdint.h>

#define ARENA_BLOCK_SIZE 65536 // 64KB chunks

typedef struct ArenaBlock {
  struct ArenaBlock *next;
  size_t used;
  uintptr_t data[];
} ArenaBlock;

typedef struct {
  ArenaBlock *first;
  ArenaBlock *current;
} Arena;

void *arena_alloc(Arena *a, size_t size);
void arena_free(Arena *a);

char *arena_strndup(Arena *a, const char *str, size_t len);

#endif // ARENA_H