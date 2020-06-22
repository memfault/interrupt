#include <stdio.h>
#include <libopencm3/cm3/vector.h>
#include <libopencm3/cm3/scb.h>
#include <shell/shell.h>

#include "clock.h"
#include "gpio.h"
#include "image.h"
#include "memory_map.h"
#include "shared_memory.h"
#include "usart.h"

image_hdr_t image_hdr __attribute__((section(".image_hdr"))) = {
    .image_magic = IMAGE_MAGIC,
    .image_type = IMAGE_TYPE_LOADER,
    .version_major = 1,
    .version_minor = 0,
    .version_patch = 0,
    .vector_addr = (uint32_t)&vector_table,
};

int main(void) {
    clock_setup();
    gpio_setup();
    usart_setup();
    shared_memory_init();

    printf("Updater started\n");

    while (!shared_memory_is_dfu_requested()) {
        const int max_boot_attempts = 3;
        if (shared_memory_get_boot_counter() >= max_boot_attempts) {
            shared_memory_clear_boot_counter();
            printf("App unstable, dropping back into DFU mode\n");
            break;
        }

        const vector_table_t *vectors = image_get_vectors(IMAGE_SLOT_2);
        if (vectors == NULL) {
            printf("No image found in slot 2\n");
            break;
        }

        // Everything checks out, let's boot
        usart_teardown();
        printf("Booting slot 2 at %p\n", vectors->reset);
        shared_memory_increment_boot_counter();
        image_boot_vectors(vectors);
    }

    printf("Entering DFU Mode\n");
    shared_memory_set_dfu_requested(false);

    // Configure shell
    sShellImpl shell_impl = {
      .send_char = usart_putc,
    };
    shell_boot(&shell_impl);

    char c;
    while (1) {
        c = usart_getc();
        shell_receive_char(c);
    }

    return 0;
}
