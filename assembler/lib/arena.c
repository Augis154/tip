#include "arena.h"

#include <stdlib.h>
#include <string.h>

void *arena_alloc(Arena *a, size_t size) {
  size = (size + sizeof(void *) - 1) & ~(sizeof(void *) - 1);

  if (!a->current || a->current->used + size > ARENA_BLOCK_SIZE) {
    ArenaBlock *new_block = malloc(sizeof(ArenaBlock) + ARENA_BLOCK_SIZE);
    new_block->next = NULL;
    new_block->used = 0;

    if (a->current) {
      a->current->next = new_block;
    } else {
      a->first = new_block;
    }
    a->current = new_block;
  }

  uintptr_t *ptr = a->current->data + a->current->used;
  a->current->used += size;
  return (void *)ptr;
}

void arena_free(Arena *a) {
  ArenaBlock *block = a->first;

  while (block) {
    ArenaBlock *next = block->next;
    free(block);
    block = next;
  }

  a->first = NULL;
  a->current = NULL;
}

ArenaMark arena_mark(Arena *a) {
  return (ArenaMark){.block = a->current, .used = a->current ? a->current->used : 0};
}

void arena_rollback(Arena *a, ArenaMark mark) {
  if (!a->first) {
    return;
  }

  ArenaBlock *curr = mark.block ? mark.block->next : a->first;
  while (curr) {
    ArenaBlock *next = curr->next;
    free(curr);
    curr = next;
  }

  a->current = mark.block;
  if (a->current) {
    a->current->used = mark.used;
    a->current->next = NULL;
  } else {
    a->first = NULL;
  }
}

char *arena_strdup(Arena *a, const char *str) {
  size_t len = strlen(str);
  char *copy = arena_alloc(a, len + 1);
  memcpy(copy, str, len);
  copy[len] = '\0';
  return copy;
}

char *arena_strndup(Arena *a, const char *str, size_t len) {
  char *copy = arena_alloc(a, len + 1);
  memcpy(copy, str, len);
  copy[len] = '\0';
  return copy;
}