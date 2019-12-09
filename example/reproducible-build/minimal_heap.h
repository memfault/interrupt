#pragma once

#include <stddef.h>

void *minimal_heap_malloc(size_t size);
void minimal_heap_free(void);
