#include <libopencm3/stm32/usart.h>

#include "usart.h"

void usart_setup(void)
{
    /* Setup USART2 parameters. */
    usart_set_baudrate(USART2, 115200);
    usart_set_databits(USART2, 8);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_mode(USART2, USART_MODE_TX_RX);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

    /* Finally enable the USART. */
    usart_enable(USART2);
}

void usart_teardown(void)
{
    usart_disable(USART2);
}

int usart_putc(char c) {
    usart_send_blocking(USART2, c);
    return 0;
}

char usart_getc(void) {
  // Blocks until a character is captured from the UART
  uint16_t cr = usart_recv_blocking(USART2);
  return (char)cr;
}


