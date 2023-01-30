#include "dfu.h"
#include "image.h"
#include "memory_map.h"
#include "shell/shell.h"
#include "shared_memory.h"
#include "simple_fileio.h"

#include <libopencm3/cm3/scb.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>


#define JANPATCH_STREAM sfio_stream_t
#include <janpatch.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

static unsigned char source_buf[4096];
static unsigned char target_buf[4096];
static unsigned char patch_buf[4096];


extern char _binary_build_patch_bin_start;
extern char _binary_build_patch_bin_size;

int cli_command_do_dfu(int argc, char *argv[]) {
    shell_put_line("Starting update");

    uint8_t *data_ptr = (uint8_t *)&_binary_build_patch_bin_start;

    janpatch_ctx ctx = {
        // fread/fwrite buffers for every file, minimum size is 1 byte
        // when you run on an embedded system with block size flash, set it to the size of a block for best performance
        { source_buf, 4096 },
        { target_buf, 4096 },
        { patch_buf, 4096 },

        // define functions which can perform basic IO
        // on POSIX, use:
        &sfio_fread,
        &sfio_fwrite,
        &sfio_fseek,

	NULL, // ftell not implemented
        NULL, // progress callback not implemented
    };

    JANPATCH_STREAM source = {
        .type = SFIO_STREAM_SLOT,
	.offset = 0,
	.size = (size_t)&__slot2rom_size__,
	.slot = IMAGE_SLOT_2,
    };
    JANPATCH_STREAM patch = {
        .type = SFIO_STREAM_RAM,
	.offset = 0,
	.size = (size_t)&_binary_build_patch_bin_size,
	.ptr = data_ptr,
    };
    JANPATCH_STREAM target = {
        .type = SFIO_STREAM_SLOT,
	.offset = 0,
	.size = (size_t)&__slot2rom_size__,
	.slot = IMAGE_SLOT_2,
    };

    shell_put_line("Patching data");
    janpatch(ctx, &source, &patch, &target);

    // grab header
    const image_hdr_t *hdr = image_get_header(IMAGE_SLOT_2);
    // Check & commit image
    shell_put_line("Validating image");
    if (image_validate(IMAGE_SLOT_2, hdr)) {
        shell_put_line("Validation Failed");
        return -1;
    };

    shell_put_line("Checking signature");
    if (image_check_signature(IMAGE_SLOT_2, hdr)) {
        shell_put_line("Signature does not match");
        return -1;
    };

    shell_put_line("Committing image");
    if (dfu_commit_image(IMAGE_SLOT_2, hdr)) {
        shell_put_line("Image Commit Failed");
        return -1;
    };

    shell_put_line("Rebooting");
    scb_reset_system();
    while (1) {}
    return 0;
}

int cli_command_dump_app(int argc, char *argv[]) {
    uint8_t *ptr = (uint8_t *)&__slot2rom_start__;
    const image_hdr_t *hdr = image_get_header(IMAGE_SLOT_2);
    size_t size = (size_t)hdr->data_size;

    printf("Dumpling slot 2, (%u bytes)\n", size);

    for (int i = 0; i < size; i += 2) {
	if (i % 16 == 0) {
	    printf("\n");
            printf("%08x: ", i);
        }
        printf("%02x%02x ", ptr[i], ptr[i+1]);
    }

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
  {"dump-app", cli_command_dump_app, "Hexdump of app slot"},
  {"help", shell_help_handler, "Lists all commands"},
};

const sShellCommand *const g_shell_commands = s_shell_commands;
const size_t g_num_shell_commands = ARRAY_SIZE(s_shell_commands);
