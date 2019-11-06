#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "impl_common.h"

void __assert_func(const char *file, 
                   int line, 
                   const char *func, 
                   const char *failedexpr) {
  snprintf(g_assert_info.msg, sizeof(g_assert_info.msg), 
      "ASSERT: %s at %s\n", failedexpr, func);
  strncpy(g_assert_info.file, file, sizeof(g_assert_info.file));
  g_assert_info.line = line;

  __asm("bkpt 1");
  while (1);
  __builtin_unreachable();
}

void assert_path_A(void) {
  assert(0);
}

void assert_path_B(void) {
  assert(0);
}

