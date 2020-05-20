#include "example_project/examples.h"

#include <string.h>
#include <stdlib.h>

#include "example_project/memory_pool.h"

uint32_t example_malloc_free(void) {
  const size_t size = 10;
  uint8_t *ptr = malloc(size);
  free(ptr);

  // access the data before setting it
  uint32_t sum = 0;
  for (size_t i = 0; i < size; i++) {
    sum += ptr[i];
  }
  return sum;
}

uint32_t example_access_garbage(void) {
  const size_t size = 10;
  uint8_t *ptr = memory_pool_allocate(size);
  if (ptr == NULL) {
    return 0;
  }

  // access the data before setting it
  uint32_t sum = 0;
  for (size_t i = 0; i < size; i++) {
    sum += ptr[i];
  }

  return sum;
}

uint32_t example_memory_leak(void) {
  const size_t size = 10;
  uint8_t *ptr = memory_pool_allocate(size);
  if (ptr == NULL) {
    return 0;
  }

  // ... do something with the pointer ...
  const uint8_t return_code = ((uint32_t)(uintptr_t)ptr) ? 0xa5 : 0xef;
  if (return_code == 0xa5) {
    return 1;
  }

  memory_pool_free(ptr);
  return 0;
}


uint32_t example_use_after_free(void) {
  const size_t size = 10;
  uint8_t *ptr = memory_pool_allocate(size);
  if (ptr == NULL) {
    return 0;
  }

  // ... do something with the pointer ...
  const uint8_t return_code = ((uint32_t)(uintptr_t)ptr) ? 0xa5 : 0xef;
  if (return_code == 0xa5) {
    memory_pool_free(ptr);
  }

  memset(ptr, 0x5e, size);
  return 0;
}

uint32_t example_project_run_memory_leak_examples(void) {
  return example_malloc_free() + example_access_garbage() + example_memory_leak() + example_use_after_free();
}
