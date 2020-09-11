//! @file
//!
//! @brief
//! Shell commands for experimenting with breakpoints

#include "shell/shell.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dummy_functions.h"
#include "fpb.h"
#include "hal/logging.h"

#include "accel.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

__attribute__((aligned(32)))
volatile uint8_t g_array[17] = { 0 };


typedef struct {
  const char *name;
  void (*func)(void);
} sDummyFunction;

#define DUMMY_FUNC_ENTRY(_f) \
  { .name = #_f, .func = _f}

static const sDummyFunction s_dummy_funcs[] = {
  DUMMY_FUNC_ENTRY(dummy_function_1),
  DUMMY_FUNC_ENTRY(dummy_function_2),
  DUMMY_FUNC_ENTRY(dummy_function_3),
  DUMMY_FUNC_ENTRY(dummy_function_4),
  DUMMY_FUNC_ENTRY(dummy_function_5),
  DUMMY_FUNC_ENTRY(dummy_function_6),
  DUMMY_FUNC_ENTRY(dummy_function_7),
  DUMMY_FUNC_ENTRY(dummy_function_8),
  DUMMY_FUNC_ENTRY(dummy_function_9),
  DUMMY_FUNC_ENTRY(dummy_function_ram),
};

static int prv_call_dummy_funcs(int argc, char *argv[]) {
  for (size_t i = 0; i < ARRAY_SIZE(s_dummy_funcs); i++) {
    s_dummy_funcs[i].func();
  }
  return 0;
}

static int prv_dump_dummy_funcs(int argc, char *argv[]) {
  for (size_t i = 0; i < ARRAY_SIZE(s_dummy_funcs); i++) {
    const sDummyFunction *d = &s_dummy_funcs[i];
    // physical address is function address with thumb bit removed
    volatile uint32_t *addr = (uint32_t *)(((uint32_t)d->func) & ~0x1);
    EXAMPLE_LOG("%s: Starts at 0x%x. First Instruction = 0x%x", d->name, addr, *addr);
  }

  return 0;
}

// disable optimizations to guarantee recursion
__attribute__((optimize("O0")))
int math_function(int n) {
  if (n == 0) {
    return 0;
  }

  uint32_t work_buf[3];
  memset(work_buf, n & 0xff, sizeof(work_buf));

  if (n == 10) {
    work_buf[0] = 0xbadcafe;
  }

  int sum = 0;
  for (size_t i = 0; i < sizeof(work_buf)/sizeof(work_buf[0]); i++) {
    sum += work_buf[i];
  }

  return math_function(n - 1) + sum;
}

static int prv_dwt_dump(int argc, char *argv[]) {
  extern void dwt_dump(void);
  dwt_dump();
  return 0;
}

static int prv_dwt_reset(int argc, char *argv[]) {
  extern void dwt_reset(void);
  dwt_reset();
  return 0;
}

static int prv_accel_example(int argc, char *argv[]) {
  accel_process_reading(7, 1, 1);
  return 0;
}

static int prv_math_example(int argc, char *argv[]) {
  int n = argc < 2 ? 1000: atoi(argv[1]);
  EXAMPLE_LOG("Running math_function()");
  int res = math_function(n);
  EXAMPLE_LOG("Result = %d\n", res);
  return 0;
}

static int prv_arr_read(int argc, char *argv[]) {
  if (argc < 2) {
    EXAMPLE_LOG("Expected [idx] arg");
    return -1;
  }

  int idx = atoi(argv[1]);
  if (idx > ARRAY_SIZE(g_array)) {
    EXAMPLE_LOG("Index %d out of range", idx);
    return -1;
  }

  EXAMPLE_LOG("Read - Addr: 0x%08x, Index: %d, Value: 0x%08x",
              &g_array[idx], (int)idx, g_array[idx]);
  return 0;
}


static int prv_arr_write(int argc, char *argv[]) {
  if (argc < 3) {
    EXAMPLE_LOG("Expected [idx] [val] args");
    return -1;
  }

  int idx = atoi(argv[1]);
  if (idx > ARRAY_SIZE(g_array)) {
    EXAMPLE_LOG("Index %d out of range", idx);
    return -1;
  }

  int val = atoi(argv[2]);

  EXAMPLE_LOG("Write - Addr: 0x%08x, Index: %d, Value: 0x%08x",
              &g_array[idx], (int)idx, val);
  g_array[idx] = val & 0xff;

  return 0;
}

static int prv_watchpoint_set(int argc, char *argv[]) {
  if (argc < 5) {
    EXAMPLE_LOG("Expected [idx] [FUNC] [COMP] [MASK] args");
    return -1;
  }


  int comp_id = strtoul(argv[1], NULL, 0x0);
  uint32_t func = strtoul(argv[2], NULL, 0x0);
  uint32_t comp = strtoul(argv[3], NULL, 0x0);
  uint32_t mask = strtoul(argv[4], NULL, 0x0);
  EXAMPLE_LOG("Configuring COMP%d", comp_id);
  EXAMPLE_LOG("  Set DWT_FUNC=0x%08x, DWT_COMP=0x%08x, DWT_MASK=0x%08x",
              func, comp, mask);

  extern void dwt_install_watchpoint(int comp_id, uint32_t func, uint32_t comp, uint32_t mask);
  dwt_install_watchpoint(comp_id, func, comp, mask);

  return 0;
}

static const sShellCommand s_shell_commands[] = {
  {"arr_write", prv_arr_write, "arr write [idx] [val]"},
  {"arr_read", prv_arr_read, "arr read [idx]"},

  {"accel_example", prv_accel_example, "Feed data into accel driver for processing" },
  {"math_example", prv_math_example, "Call a (recursive) computational function" },

  {"dwt_dump", prv_dwt_dump, "Dump DWT state"},
  {"dwt_reset", prv_dwt_reset, "Reset DWT Comparator state to POR state"},
  {"watchpoint_set", prv_watchpoint_set, "watchpoint_set ID FUNC COMP MASK"},

  {"call_dummy_funcs", prv_call_dummy_funcs, "Invoke dummy functions"},
  {"dump_dummy_funcs", prv_dump_dummy_funcs, "Print first instruction of each dummy function"},
  {"help", shell_help_handler, "Lists all commands"},
};

const sShellCommand *const g_shell_commands = s_shell_commands;
const size_t g_num_shell_commands = ARRAY_SIZE(s_shell_commands);
