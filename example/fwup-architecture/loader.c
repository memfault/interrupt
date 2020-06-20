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
    .image_type = IMAGE_TYPE_LOADER,
    .version_major = 1,
    .version_minor = 0,
    .version_patch = 0,
    .vector_addr = (uint32_t)&vector_table,
};

static void start_app(void *pc, void *sp) {
    __asm("           \n\
          msr msp, r1 \n\
          bx r0       \n\
    ");
}

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

    vector_table_t *app_vectors = (vector_table_t *) &__slot2rom_start__;
    printf("Jumping to %p\n", app_vectors->reset);
    start_app(app_vectors->reset, app_vectors->initial_sp_value);

    while (1) {
    }

    return 0;
}
