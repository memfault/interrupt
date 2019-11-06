#pragma once

extern void my_assert(const char *msg);

#define MY_ASSERT(expr, msg)   \
  do {                         \
    if (!(expr)) {             \
      my_assert(msg);          \
    }                          \
  } while (0)
