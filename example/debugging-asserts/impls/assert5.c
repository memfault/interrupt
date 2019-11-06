#include "assert5.h"
#include "impl_common.h"
#include "string.h"

void my_assert(const uint32_t *pc, const uint32_t *lr) {
  // File and line deliberately left empty
  g_assert_info.pc = (uint32_t)pc;
  g_assert_info.lr = (uint32_t)lr;
  __asm("bkpt 5");
}

void assert_path_A(void) {
  MY_ASSERT(0);
}

void assert_path_B(void) {
  MY_ASSERT(0);
}
