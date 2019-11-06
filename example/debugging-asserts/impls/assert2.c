#include "assert2.h"
#include "impl_common.h"
#include "string.h"

void my_assert(const char *msg) {
  strncpy(g_assert_info.msg, msg, sizeof(g_assert_info.msg)); 
  strncpy(g_assert_info.file, __FILE__, sizeof(g_assert_info.file));
  g_assert_info.line = __LINE__;
  g_assert_info.pc = (uint32_t)GET_PC();
  g_assert_info.lr = (uint32_t)GET_LR();
  __asm("bkpt 2");
}

void assert_path_A(void) {
  MY_ASSERT(0, "Assert in `assert2.c::assert_path_A`");
}

void assert_path_B(void) {
  MY_ASSERT(0, "Assert in `assert2.c::assert_path_B`");
}
