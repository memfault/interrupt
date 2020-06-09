#include "app_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf.h"
#include "bsp.h"
#include "nrf_uart.h"

#include "shell/shell.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256                         /**< UART RX buffer size. */

#define UART_HWFC APP_UART_FLOW_CONTROL_DISABLED

// Required for nrf52 uart setup
void uart_error_handle(app_uart_evt_t * p_event) {}


int console_putc(char c) {
  app_uart_put(c);
  return 1;
}

char console_getc(void) {
  uint8_t cr;
  while (app_uart_get(&cr) != NRF_SUCCESS);
  return (char)cr;
}


int main(void) {
  uint32_t err_code;
  bsp_board_init(BSP_INIT_LEDS);

  const app_uart_comm_params_t comm_params =
    {
        RX_PIN_NUMBER,
        TX_PIN_NUMBER,
        RTS_PIN_NUMBER,
        CTS_PIN_NUMBER,
        UART_HWFC,
        false,
        NRF_UART_BAUDRATE_115200
    };

  APP_UART_FIFO_INIT(&comm_params,
                     UART_RX_BUF_SIZE,
                     UART_TX_BUF_SIZE,
                     uart_error_handle,
                     APP_IRQ_PRIORITY_LOWEST,
                     err_code);
  APP_ERROR_CHECK(err_code);

  sShellImpl shell_impl = {
    .send_char = console_putc,
  };
  shell_boot(&shell_impl);
  
  char c;
  while (true) {
    c = console_getc();
    shell_receive_char(c);
  }
}
