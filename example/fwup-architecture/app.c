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
    .image_type = IMAGE_TYPE_APP,
    .version_major = 1,
    .version_minor = 0,
    .version_patch = 1,
    .vector_addr = (uint32_t)&vector_table,
};

int main(void) {
    clock_setup();
    gpio_setup();
    usart_setup();
    shared_memory_init();

    printf("App started\n");
    if (!shared_memory_is_update_complete()) {
        printf("Requesting update\n");
        shared_memory_set_update_requested(true);
        scb_reset_system();
    }

    while (1) {
    }

    return 0;
}
