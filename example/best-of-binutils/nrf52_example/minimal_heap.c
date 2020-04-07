#include "minimal_heap.h"

#include <stdbool.h>
#include <stdint.h>

#define MINIMAL_HEAP_TOTAL_SIZE 16

typedef struct {
  uint8_t heap[MINIMAL_HEAP_TOTAL_SIZE];
  bool space_free;
} sMinimalHeapContext;

static sMinimalHeapContext s_heap = {
  .space_free = true,
};

void *minimal_heap_malloc(size_t size) {
  if (!s_heap.space_free || size > MINIMAL_HEAP_TOTAL_SIZE) {
    return NULL;
  }
  s_heap.space_free = false;
  return &s_heap.heap[0];
}

void minimal_heap_free(void) {
  s_heap.space_free = true;
}
