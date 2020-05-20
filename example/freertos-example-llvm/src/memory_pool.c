#include "example_project/memory_pool.h"

#include <stdbool.h>
#include <stdint.h>

#include "example_project/compiler.h"

static bool s_memory_allocated = false;

// NB: align on an 8 byte boundary for heap allocations
static uint8_t s_memory[256] EXAMPLE_PROJ_ALIGNED(8);

void *memory_pool_allocate(size_t size) {
  if (s_memory_allocated) {
    return NULL;
  }

  s_memory_allocated = true;
  return &s_memory[0];
}

void memory_pool_free(void *ptr) {
  s_memory_allocated = false;
}
