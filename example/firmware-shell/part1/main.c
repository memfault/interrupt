#include "app_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf.h"
#include "bsp.h"
#include "nrf_uart.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256                         /**< UART RX buffer size. */

#define UART_HWFC APP_UART_FLOW_CONTROL_DISABLED

// Required for nrf52 uart setup
void uart_error_handle(app_uart_evt_t * p_event) {}

void console_putc(char c) {
  app_uart_put(c);
}

char console_getc(void) {
  uint8_t cr;
  while (app_uart_get(&cr) != NRF_SUCCESS);
  return (char)cr;
}

void console_puts(char *s) {
  while (*s != '\0') {
    if (*s == '\r') {
      console_putc('\n');
    } else {
      console_putc(*s);
    }
    s++;
  }
}

void console_put_line(char *s) {
  console_puts(s);
  console_puts("\n");
}

int console_gets(char *s, int len) {
  char *t = s;
  char c;

  *t = '\000';
  /* read until a <CR> is received */
  while ((c = console_getc()) != '\n') {
    if ((c == '\010') || (c == '\127')) {
      if (t > s) {
        /* send ^H ^H to erase previous character */
        console_puts("\010 \010");
        t--;
      }
    } else {
      *t = c;
      console_putc(c);
      if ((t - s) < len) {
        t++;
      }
    }
    /* update end of string with NUL */
    *t = '\000';
  }
  return t - s;
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

  char buf[128];
  int len;
  while (true) {
    console_puts("$ ");
    len = console_gets(buf, 128);
    console_putc('\n');
    if (len) {
      if (0 == strncmp("help", buf, len)) {
        console_put_line("OK: ");
      } else {
        console_put_line("FAIL: unrecognized command");
      }
    }
  }
}
