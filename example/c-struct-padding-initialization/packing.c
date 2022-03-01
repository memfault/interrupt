//!
//! Difference between packed and unpacked for trailing padding in arrays.
//!

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct foo {
  uint8_t b;
  uint16_t a;
};

struct __attribute__((packed)) foo_packed {
  uint8_t b;
  uint16_t a;
};

int main(int argc, char **argv) {
  (void)argc;

  {
    struct foo foo[2];
    printf("foo (unpacked):\n");
    printf("  size: %zu / %zu\n", sizeof(foo), sizeof(*foo));
    printf("  address: %p , %p\n", &foo[0], &foo[1]);
    printf("  address Δ: %" PRIuPTR "\n",
           (uintptr_t)&foo[1] - (uintptr_t)&foo[0]);
  }
  {
    struct foo_packed foo[2];
    printf("foo packed:\n");
    printf("  size: %zu / %zu\n", sizeof(foo), sizeof(*foo));
    printf("  address: %p , %p\n", &foo[0], &foo[1]);
    printf("  address Δ: %" PRIuPTR "\n",
           (uintptr_t)&foo[1] - (uintptr_t)&foo[0]);
  }

  return 0;
}
