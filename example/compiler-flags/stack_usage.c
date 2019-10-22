#include <stdint.h>

int stack_usage_example(int num) {
  uint8_t tmp[256];
  tmp[0] = 0;

  for (int i = 1;  i < sizeof(tmp); i++) {
    tmp[i] = (uint8_t)(tmp[i-1] + num);
  }

  return tmp[255];
}

int vla_stack_usage(int num) {
  if (num <= 0) {
    return -1;
  }

  uint8_t tmp[num];
  tmp[0] = (uint8_t)0;

  for (int i = 1;  i < sizeof(tmp); i++) {
    tmp[i] = (uint8_t)(tmp[i-1] + num);
  }

  return tmp[num - 1];
}
