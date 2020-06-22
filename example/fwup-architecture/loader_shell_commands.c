#include "dfu.h"
#include "image.h"
#include "shell/shell.h"
#include "shared_memory.h"

#include <libopencm3/cm3/scb.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

extern char build_fwup_example_app_bin[];
extern int build_fwup_example_app_bin_len;

int cli_command_do_dfu(int argc, char *argv[]) {
    shell_put_line("Starting update");

    uint8_t *data_ptr = (uint8_t *)build_fwup_example_app_bin;

    // grab header
    image_hdr_t *hdr = (image_hdr_t *)data_ptr;

    shell_put_line("Writing data");
    // write image data
    data_ptr += sizeof(image_hdr_t);
    if (dfu_write_data(IMAGE_SLOT_2,
                       data_ptr,
                       build_fwup_example_app_bin_len)) {
        shell_put_line("Failed");
        return -1;
    }

    shell_put_line("Validating image");
    // Check & commit image
    if (dfu_validate_image(IMAGE_SLOT_2, hdr)) {
        shell_put_line("Failed");
        return -1;
    };

    shell_put_line("Committing image");
    if (dfu_commit_image(IMAGE_SLOT_2, hdr)) {
        shell_put_line("Failed");
        return -1;
    };

    shell_put_line("Rebooting");
    scb_reset_system();
    while (1) {}
    return 0;
}

int cli_command_erase_app(int argc, char *argv[]) {
    shell_put_line("Erasing app");
    return dfu_invalidate_image(IMAGE_SLOT_2);
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
