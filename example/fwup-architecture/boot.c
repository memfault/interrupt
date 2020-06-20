#include <stdio.h>
#include <libopencm3/cm3/vector.h>

#include "clock.h"
#include "gpio.h"
#include "image.h"
#include "memory_map.h"
#include "usart.h"

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

    printf("Bootloader started\n");

    usart_teardown();

    vector_table_t *app_vectors = (vector_table_t *) &__slot1rom_start__;
    printf("Jumping to %p\n", app_vectors->reset);
    start_app(app_vectors->reset, app_vectors->initial_sp_value);

    while (1) {
    }

    return 0;
}
