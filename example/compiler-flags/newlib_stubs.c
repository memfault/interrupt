//! Stub functions for standard library routines since we are compiling with -nostdlib

#include <stdio.h>

int printf(const char * restrict format, ...) {
  // dummy printf function since we aren't linking in stdlib
  return 0;
}

int snprintf(char *buf, size_t buf_len, const char *fmt, ...) {
  // dummy snprintf function since we aren't linking stdlib
  return 0;
}
