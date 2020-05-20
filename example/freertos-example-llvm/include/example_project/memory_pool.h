#pragma once

#include <stddef.h>

#include "example_project/compiler.h"

EXAMPLE_PROJ_MALLOC_LIKE_FUNC
void *memory_pool_allocate(size_t size);

// warning: Memory allocated by memory_pool_allocate()
// should be deallocated by free(), not memory_pool_free()
#ifdef __clang_analyzer__
#define memory_pool_free free
#endif

EXAMPLE_PROJ_FREE_LIKE_FUNC
void memory_pool_free(void *ptr);
