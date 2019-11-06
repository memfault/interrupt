#pragma once

#include "impl_common.h"

#include <assert.h>

extern void my_assert(const uint32_t *pc, const uint32_t *lr);

#define MY_ASSERT_RECORD()     \
  do {                         \
    void *pc;                  \
    GET_PC_ASM(pc);            \
    const void *lr = GET_LR(); \
    my_assert(pc, lr);         \
  } while (0)

#define MY_ASSERT(exp)         \
  do {                         \
    if (!(exp)) {              \
      MY_ASSERT_RECORD();      \
    }                          \
  } while (0)
