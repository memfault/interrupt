//! @file
//!
//! A port of the tiny shell to a baremetal NRF52 system using the UART as a console

#include "shell_port.h"

#include <stdbool.h>
#include <stddef.h>

#include "cmsis_shim.h"
#include "hal/uart.h"
#include "shell/shell.h"

static volatile struct {
  size_t read_idx;
  size_t num_bytes;
  char buf[64];
} s_uart_buffer = {
  .num_bytes = 0,
};

void uart_byte_received_from_isr_cb(char c) {
  if (s_uart_buffer.num_bytes >= sizeof(s_uart_buffer.buf)) {
    return; // drop, out of space
  }

  s_uart_buffer.buf[s_uart_buffer.read_idx] = c;
  s_uart_buffer.num_bytes++;
}

bool shell_port_getchar(char *c_out) {
  if (s_uart_buffer.num_bytes == 0) {
    return false;
  }

  char c = s_uart_buffer.buf[s_uart_buffer.read_idx];

  __disable_irq();
  s_uart_buffer.read_idx = (s_uart_buffer.read_idx + 1) % sizeof(s_uart_buffer.buf);
  s_uart_buffer.num_bytes--;
  __enable_irq();
  *c_out = c;
  return true;
}

static int prv_console_putc(char c) {
  uart_tx_blocking(&c, sizeof(c));
  return 1;
}

void shell_processing_loop(void) {
  const sShellImpl shell_impl = {
    .send_char = prv_console_putc,
  };
  shell_boot(&shell_impl);

  while (1) {
    char c;
    if (shell_port_getchar(&c)) {
      shell_receive_char(c);
    }
  }
}
