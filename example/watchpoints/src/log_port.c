//! @brief
//!
//! @file
//! A minimal implementation of logging platform dependencies

#include <stdarg.h>
#include <stdio.h>

#include "hal/uart.h"

static void prv_log(const char *fmt, va_list *args) {
  char log_buf[256];
  const size_t size = vsnprintf(log_buf, sizeof(log_buf) - 1, fmt, *args);
  log_buf[size] = '\n';
  uart_tx_blocking(log_buf, size + 1);
}

void example_log(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  prv_log(fmt, &args);
  va_end(args);
}
