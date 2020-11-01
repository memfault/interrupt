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

#include "hal/logging.h"
#include "bootutil/bootutil.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

static void prv_reboot(void) {
  // NVIC_SystemReset
  *(volatile uint32_t*)0xE000ED0C = 0x5FAUL << 16 | 0x4;

  __builtin_unreachable();
}

static int prv_reboot_cli(int argc, char *argv[]) {
  prv_reboot();
  return 0;
}

static int prv_swap_images(int argc, char *argv[]) {
  EXAMPLE_LOG("Triggering Image Swap");

  const int permanent = 0;
  boot_set_pending(permanent);
  prv_reboot();
  return 0;
}

static const sShellCommand s_shell_commands[] = {
  {"swap_images", prv_swap_images, "Swap images"},
  {"reboot", prv_reboot_cli, "Reboot System"},
  {"help", shell_help_handler, "Lists all commands"},
};

const sShellCommand *const g_shell_commands = s_shell_commands;
const size_t g_num_shell_commands = ARRAY_SIZE(s_shell_commands);
