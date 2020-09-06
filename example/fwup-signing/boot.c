#include <stdio.h>
#include <libopencm3/cm3/vector.h>

#include "clock.h"
#include "gpio.h"
#include "image.h"
#include "memory_map.h"
#include "usart.h"

int main(void) {
    clock_setup();
    gpio_setup();
    usart_setup();

    printf("Bootloader started\n");

    for (image_slot_t slot = IMAGE_SLOT_1; slot < IMAGE_NUM_SLOTS; ++slot) {
        const image_hdr_t *hdr = image_get_header(slot);
        if (hdr == NULL) {
            continue;
        }
        if (image_validate(slot, hdr) != 0) {
            continue;
        }

        printf("Booting slot %d\n", slot);
        usart_teardown();
        image_start(hdr);
    }

    printf("No valid image found.\n");

    while (1) {
    }

    return 0;
}
