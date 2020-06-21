#include <stdio.h>
#include <libopencm3/cm3/vector.h>
#include <libopencm3/cm3/scb.h>

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

    if (shared_memory_is_update_requested()) {
        printf("Update requested\n");
        shared_memory_set_update_requested(false);
        shared_memory_set_update_complete(true);
        scb_reset_system();
    }

    usart_teardown();

    const vector_table_t *vectors = image_get_vectors(IMAGE_SLOT_2);
    printf("Booting slot 2 at %p\n", vectors->reset);
    image_boot_vectors(vectors);

    while (1) {
    }

    return 0;
}
