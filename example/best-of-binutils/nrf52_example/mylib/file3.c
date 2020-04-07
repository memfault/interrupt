#include <stdio.h>

void mylib_func3(void) {
  printf("Hello World\n");
  __asm("bkpt 3");
}
