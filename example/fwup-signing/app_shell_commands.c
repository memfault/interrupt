#include "shell/shell.h"
#include "shared_memory.h"

#include <libopencm3/cm3/scb.h>
#include <stddef.h>
#include <stdio.h>


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

int cli_command_dfu_mode(int argc, char *argv[]) {
    shell_put_line("Rebooting into DFU mode");
    shared_memory_set_dfu_requested(true);
    scb_reset_system();
    while (1) {}
    return 0;
}

int cli_command_mark_stable(int argc, char *argv[]) {
    shell_put_line("Marking app as stable");
    shared_memory_clear_boot_counter();
    return 0;
}

int cli_command_reboot(int argc, char *argv[]) {
    shell_put_line("Rebooting");
    scb_reset_system();
    while (1) {}
    return 0;
}

static const sShellCommand s_shell_commands[] = {
  {"mark-stable", cli_command_mark_stable, "Mark app as stable"},
  {"dfu-mode", cli_command_dfu_mode, "Reboot into DFU mode"},
  {"reboot", cli_command_reboot, "Reboot device"},
  {"help", shell_help_handler, "Lists all commands"},
};

const sShellCommand *const g_shell_commands = s_shell_commands;
const size_t g_num_shell_commands = ARRAY_SIZE(s_shell_commands);
