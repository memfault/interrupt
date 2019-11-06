#pragma once

#include <assert.h>
#include <stdint.h>

extern void my_assert(const char *file, uint32_t line);

#define MY_ASSERT(expr)                   \
  do {                                    \
    if (!(expr)) {                        \
      my_assert(__FILENAME__, __LINE__);  \
    }                                     \
  } while (0)
