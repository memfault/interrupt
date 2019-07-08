/* 
 * Blinky example with a useless call to atoi function.
 * Made to study the consequences on the memory map
 * 
 * That file can be used as main.c to replace the blinky example
 * from nRF5 SDK
 * 
 * 3 configurations are possible:
 *  - No call to atoi
 *  - A call to atoi provided by the standard library
 *  - A custom definition of atoi
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

#define NO_ATOI         (0) // atoi function not called
#define STD_ATOI        (1) // atoi function from standard library
#define CUSTOM_ATOI     (2) // atoi function defined into that file

#define CONFIG_ATOI     NO_ATOI

#if (CONFIG_ATOI != NO_ATOI && CONFIG_ATOI != STD_ATOI)
#error Please use a configuration from NO_ATOI, STD_ATOI or CUSTOM_ATOI
#endif

#if (CONFIG_ATOI != NO_ATOI)
static const char* _delay_ms_str = "300";
#endif

#if (CONFIG_ATOI == STD_ATOI)
#include "stdlib.h"
#endif

#if (CONFIG_ATOI == CUSTOM_ATOI)
static int atoi(const char* str) 
{ 
    int res = 0;
  
    for (int i = 0; str[i] != '\0'; ++i) 
    {
        res = res * 10 + str[i] - '0'; 
    }

    return res; 
} 
#endif


/**
 * @brief Function for application main entry.
 */
int main(void)
{
    /* Configure board. */
    bsp_board_init(BSP_INIT_LEDS);
#if (CONFIG_ATOI != NO_ATOI)
    int delay = atoi(_delay_ms_str);
#endif

    /* Toggle LEDs. */
    while (true)
    {
        for (int i = 0; i < LEDS_NUMBER; i++)
        {
            bsp_board_led_invert(i);
#if (CONFIG_ATOI != NO_ATOI)
            nrf_delay_ms(delay);
#else
            nrf_delay_ms(300);
#endif
        }
    }
}