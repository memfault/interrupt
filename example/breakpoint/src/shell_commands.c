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

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

static int prv_dump_fpb_config(int argc, char *argv[]) {
  fpb_dump_breakpoint_config();
  return 0;
}

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

static int prv_fpb_set_breakpoint(int argc, char *argv[]) {
  if (argc < 3) {
    EXAMPLE_LOG("Expected [Comp Id] [Address]");
    return -1;
  }

  size_t comp_id = strtoul(argv[1], NULL, 0x0);
  uint32_t addr = strtoul(argv[2], NULL, 0x0);

  bool success = fpb_set_breakpoint(comp_id, addr);
  EXAMPLE_LOG("Set breakpoint on address 0x%x in FP_COMP[%d] %s", addr,
              (int)comp_id, success ? "Succeeded" : "Failed");

  return success ? 0 : -1;
}

static int prv_issue_breakpoint(int argc, char *argv[]) {
  __asm("bkpt 1");
  return 0;
}

static const sShellCommand s_shell_commands[] = {
  {"bkpt", prv_issue_breakpoint, "Issue a Breakpoint Exception" },
  {"fpb_dump", prv_dump_fpb_config, "Dump Active FPB Settings"},
  {"fpb_set_breakpoint", prv_fpb_set_breakpoint, "Set Breakpoint [Comp Id] [Address]"},
  {"call_dummy_funcs", prv_call_dummy_funcs, "Invoke dummy functions"},
  {"dump_dummy_funcs", prv_dump_dummy_funcs, "Print first instruction of each dummy function"},
  {"help", shell_help_handler, "Lists all commands"},
};

const sShellCommand *const g_shell_commands = s_shell_commands;
const size_t g_num_shell_commands = ARRAY_SIZE(s_shell_commands);
