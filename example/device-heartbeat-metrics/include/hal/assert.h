#pragma once

void example_assert(void);

#define EXAMPLE_ASSERT(expr)                        \
  do {                                              \
    if (!(expr)) {                                  \
      example_assert();                             \
    }                                               \
  } while (0)
