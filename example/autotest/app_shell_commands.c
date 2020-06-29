#include "shell/shell.h"

#include <libopencm3/cm3/scb.h>
#include <stddef.h>
#include <stdio.h>

#include "memfault/panics/assert.h"


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

int cli_command_coredump(int argc, char *argv[]) {
    shell_put_line("coredump: abcdefghijklmnopqrstuvwxyz");
    return 0;
}

int cli_command_ping(int argc, char *argv[]) {
    shell_put_line("PONG");
    return 0;
}

int cli_command_assert(int argc, char *argv[]) {
    void (*g_bad_func_call)(void) = (void (*)(void))0x20000100;
    g_bad_func_call();
    return 0;
}

static const sShellCommand s_shell_commands[] = {
  {"help", shell_help_handler, "Lists all commands"},
  {"ping", cli_command_ping, "Prints PONG"},
  {"coredump", cli_command_coredump, "Get coredump"},
  {"assert", cli_command_assert, "Assert"},
};

const sShellCommand *const g_shell_commands = s_shell_commands;
const size_t g_num_shell_commands = ARRAY_SIZE(s_shell_commands);
