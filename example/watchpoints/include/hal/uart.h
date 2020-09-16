#pragma once

#include <stddef.h>

void uart_boot(void);
void uart_tx_blocking(void *buf, size_t len);

extern void uart_byte_received_from_isr_cb(char c);
