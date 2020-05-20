#include "example_project/examples.h"

#include <stddef.h>


int example_operate_on_pointer(uint8_t *pointer) {
  int result = 0;
  if (pointer == NULL) {
    result = -1;
  }
  result += *pointer;
  return result;
}

int example_divide_by_zero(int denominator) {
  int rv = 5;
  if (denominator == 0) {
    rv = 1 / denominator;
  }
  return rv;
}
