#include <stdio.h>
#include <libopencm3/cm3/vector.h>

#include "clock.h"
#include "gpio.h"
#include "memory_map.h"
#include "usart.h"

int main(void) {
    clock_setup();
    gpio_setup();
    usart_setup();

    printf("Starting app\n");

    while (1) {
    }

    return 0;
}
