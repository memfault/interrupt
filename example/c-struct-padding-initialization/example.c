//!
//! Example program demonstrating different C structure padding initialization
//! techniques.
//!

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//! Example structure. This structure is padded to 8 bytes.
struct foo {
  uint32_t i;
  uint8_t b;
  // 3 bytes of padding inserted here, UNLESS -fpack-struct is used!
};

//! Print hex dump of a buffer, and line where it's invoked.
#define HEX_PRINT(val_, len_) hex_print(__LINE__, val_, len_)
void hex_print(int line, void *val, size_t len) {
  printf("line: %-10d", line);
  for (size_t i = 0; i < len; i++) {
    printf(" %02x", ((uint8_t *)val)[i]);
  }
  printf("\n");
}

int main(int argc, char **argv) {
  (void)argc;
  printf("Running %s...\n", argv[0]);

  struct foo foo;

  // Use 4 different initialization strategies, printing out the result of each

  // 1. memset with all 0xa5
  memset(&foo, 0xa5, sizeof(foo));
  HEX_PRINT(&foo, sizeof(foo));

  // 2. individually set all members to 0
  foo = (struct foo){
      .i = 0,
      .b = 0,
  };
  HEX_PRINT(&foo, sizeof(foo));

  // 3. use { 0 } zero-initializer
  memset(&foo, 0xa5, sizeof(foo));
  foo = (struct foo){0};
  HEX_PRINT(&foo, sizeof(foo));

  // 4. use {} extension zero-initializer
  memset(&foo, 0xa5, sizeof(foo));
  foo = (struct foo){};
  HEX_PRINT(&foo, sizeof(foo));

  return 0;
}
