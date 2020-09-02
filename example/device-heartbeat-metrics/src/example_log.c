#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "hal/logging.h"
#include "hal/uart.h"

static void prv_log(const char *fmt, va_list *args) {
  char log_buf[128];
  const size_t size = vsnprintf(log_buf, sizeof(log_buf) - 1, fmt, *args);
  log_buf[size] = '\n';
  uart_tx_blocking(log_buf, size + 1);
}

void example_log(eExampleLogLevel level, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  prv_log(fmt, &args);
  va_end(args);
}

void example_log_str(eExampleLogLevel level, const char *msg, size_t msg_len) {
  uart_tx_blocking(msg, msg_len);
  char buf[] = "\n";
  uart_tx_blocking(buf, sizeof(buf));
}
