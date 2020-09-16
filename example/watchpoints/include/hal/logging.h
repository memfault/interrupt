#pragma once

void example_log(const char *fmt, ...);

#define EXAMPLE_LOG(...)                            \
  do {                                              \
    example_log(__VA_ARGS__);                       \
  } while (0)
