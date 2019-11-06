#pragma once

#include <inttypes.h>

#define GET_LR() __builtin_return_address(0)
#define GET_PC_ASM(_a) __asm volatile ("mov %0, pc" : "=r" (_a)) 
#if defined(__GNUC__)
#define GET_PC(_a) ({ void *pc; GET_PC_ASM(pc); pc; })
#endif

// Convenience structure that we can store items in
//  to print out later when we add logging.
typedef struct sAssertInfo {
  uint32_t pc;
  uint32_t lr;
  uint32_t line;
  // I don't suggest actually using these, but they
  // are included for the examples.
  char file[256];
  char msg[256];
} sAssertInfo;

// Data structure which stores assert information
extern sAssertInfo g_assert_info;
// Our interal "print" buffer since we aren't printing
//  to serial output
extern char g_assert_str_buf[512];

void assert_path_A(void);

void assert_path_B(void);
