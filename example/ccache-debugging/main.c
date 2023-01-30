#include <stdio.h>

#include "generated_header.h"

int main(int argc, char **argv) {
  (void)argc, (void)argv;

  printf("%s\n", BUILD_UUID);
  printf("%s : %s\n", __DATE__, __TIME__);

  return 0;
}
