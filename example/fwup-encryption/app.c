#include <stdint.h>
#include "stm32l4s5xx.h"

#define DELAY_TIME      (1000000U)
#define GPIO_PIN_14     (14U)
#define MODE_HIGH_SPEED (0x00000002U)
#define MODE_OUTPUT     (0x1UL << 0U)
#define OUTPUT_PP       (0x0UL << 4U)
#define GPIO_MODE_OUTPUT_PP (MODE_OUTPUT | OUTPUT_PP) 
#define GPIO_MODE       (0x3uL << 0u)


void gpio_setup( void );
void gpio_toggle( void );

int main(void) {

    gpio_setup();

    while(1){
        gpio_toggle(); 
    }

    return 0;
}

void gpio_setup( void ) {
    uint32_t ouput_mode;

    //! Enable the clock source GPIOB
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    //! Set the speed GPIO
    GPIOB->OSPEEDR |= ((MODE_HIGH_SPEED) << (GPIO_PIN_14*2));

    //! Set the General purpose output mode
    ouput_mode = GPIOB->MODER;
    ouput_mode &= ~(GPIO_MODER_MODE0 << (GPIO_PIN_14 * 2u));
    ouput_mode |= ((GPIO_MODE_OUTPUT_PP & GPIO_MODE) << (GPIO_PIN_14 * 2u));
    GPIOB->MODER = ouput_mode;
}

void gpio_toggle( void ) {
    //! Toggle the state 
    GPIOB->BSRR = ((GPIOB->ODR & ((uint16_t)0x4000)) << (16U)) | (~GPIOB->ODR & ((uint16_t)0x4000));

    //! Delay time
    for(uint32_t i = 0; i < DELAY_TIME; i++) {
        __asm__("nop");
    }
}

