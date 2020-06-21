#include "image.h"
#include "memory_map.h"
#include "shell/shell.h"
#include "shared_memory.h"

#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/f4/flash.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

extern char build_fwup_example_app_bin[];
extern int build_fwup_example_app_bin_len;

int cli_command_do_dfu(int argc, char *argv[]) {
    shell_put_line("Starting update");
    if (build_fwup_example_app_bin_len > 128 * 1024) {
        shell_put_line("App bin too large");
        return 0;
    }

    // FIXME -- Renode implements STM32 flash as generic Memory
    flash_erase_sector(5, 0);
    shell_put_line("Writing to flash");
    // TODO refactor this into "DFU" module, and write the image
    // header last once the image is verified
    flash_program((uint32_t)&__slot2rom_start__,
                  (uint8_t *)build_fwup_example_app_bin,
                  build_fwup_example_app_bin_len);
    shell_put_line("Rebooting");
    scb_reset_system();
    while (1) {}
    return 0;
}

int cli_command_erase_app(int argc, char *argv[]) {
    shell_put_line("Erasing app");
    for (int i = 0; i < sizeof(image_hdr_t); ++i) {
        flash_program_byte((uint32_t)&__slot2rom_start__ + i, 0);
    }
    return 0;
}

int cli_command_reboot(int argc, char *argv[]) {
    shell_put_line("Rebooting");
    scb_reset_system();
    while (1) {}
    return 0;
}

static const sShellCommand s_shell_commands[] = {
  {"do-dfu", cli_command_do_dfu, "Do a firmware update"},
  {"erase-app", cli_command_erase_app, "Erase app from slot 2"},
  {"reboot", cli_command_reboot, "Reboot device"},
  {"help", shell_help_handler, "Lists all commands"},
};

const sShellCommand *const g_shell_commands = s_shell_commands;
const size_t g_num_shell_commands = ARRAY_SIZE(s_shell_commands);
