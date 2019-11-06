#include "assert3.h"
#include "impl_common.h"
#include "string.h"

void my_assert(const char *file, uint32_t line) {
  strncpy(g_assert_info.file, file, sizeof(g_assert_info.file));
  g_assert_info.line = line;
  g_assert_info.pc = (uint32_t)GET_PC();
  g_assert_info.lr = (uint32_t)GET_LR();
  __asm("bkpt 3");
}

void assert_path_A(void) {
  MY_ASSERT(0);
}

void assert_path_B(void) {
  MY_ASSERT(0);
}
