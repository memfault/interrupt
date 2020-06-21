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

    usart_teardown();

    for (image_slot_t slot = IMAGE_SLOT_1; slot < IMAGE_NUM_SLOTS; ++slot) {
        const vector_table_t *vectors = image_get_vectors(slot);
        if (vectors != NULL) {
            printf("Booting slot %d at %p\n", slot, vectors->reset);
            image_boot_vectors(vectors);
        }
    }

    printf("No valid image found.\n");

    while (1) {
    }

    return 0;
}
