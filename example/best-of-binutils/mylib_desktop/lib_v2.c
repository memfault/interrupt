#include <stdio.h>

#include "library.h"

void my_library_function(void) {
  printf("%s  %s:%d: Version 1\n", __func__, __FILE__, __LINE__);
}
