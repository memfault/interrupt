#include "hal/uart.h"
#include "hal/assert.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_shim.h"

// What the NRF52 uart peripheral looks like
typedef struct {
  __OM  uint32_t  TASKS_STARTRX;
  __OM  uint32_t  TASKS_STOPRX;
  __OM  uint32_t  TASKS_STARTTX;
  __OM  uint32_t  TASKS_STOPTX;
  __IM  uint32_t  RESERVED[3];
  __OM  uint32_t  TASKS_SUSPEND;
  __IM  uint32_t  RESERVED1[56];
  __IOM uint32_t  EVENTS_CTS;
  __IOM uint32_t  EVENTS_NCTS;
  __IOM uint32_t  EVENTS_RXDRDY;
  __IM  uint32_t  RESERVED2[4];
  __IOM uint32_t  EVENTS_TXDRDY;
  __IM  uint32_t  RESERVED3;
  __IOM uint32_t  EVENTS_ERROR;
  __IM  uint32_t  RESERVED4[7];
  __IOM uint32_t  EVENTS_RXTO;
  __IM  uint32_t  RESERVED5[46];
  __IOM uint32_t  SHORTS;
  __IM  uint32_t  RESERVED6[64];
  __IOM uint32_t  INTENSET;
  __IOM uint32_t  INTENCLR;
  __IM  uint32_t  RESERVED7[93];
  __IOM uint32_t  ERRORSRC;
  __IM  uint32_t  RESERVED8[31];
  __IOM uint32_t  ENABLE;
  __IM  uint32_t  RESERVED9;
  struct {
    __IOM uint32_t  RTS;
    __IOM uint32_t  TXD;
    __IOM uint32_t  CTS;
    __IOM uint32_t  RXD;
  } PSEL;
  __IM  uint32_t  RXD;
  __OM  uint32_t  TXD;
  __IM  uint32_t  RESERVED10;
  __IOM uint32_t  BAUDRATE;
  __IM  uint32_t  RESERVED11[17];
  __IOM uint32_t  CONFIG;
} sNrfUartConfig;

static sNrfUartConfig *const UART = ((sNrfUartConfig *)0x40002000);

static void prv_enable_nvic(int exti_id) {
  volatile uint32_t *nvic_ipr = (void *)(0xE000E400 + 4 * (exti_id / 4));
  *nvic_ipr = 0xe0 << ((exti_id % 4) * 8);

  volatile uint32_t *nvic_iser = (void *)0xE000E100;
  *nvic_iser |= (1 << (exti_id % 32));
}

void uart_boot(void) {
  UART->PSEL.RTS = 5;
  UART->PSEL.TXD = 6;
  UART->PSEL.CTS = 7;
  UART->PSEL.RXD = 8;


  UART->BAUDRATE = 0x01D60000; // 115200

  // no parity, 1 stop bit, no flow control
  UART->CONFIG = 1;
  UART->ENABLE = 8;

  prv_enable_nvic(2);
  UART->INTENSET |= (1 << 2);
  UART->TASKS_STARTRX = 1;
}

void uart_tx_blocking(const void *buf, size_t buf_len) {
  for (size_t i = 0; i < buf_len; i++) {
    UART->EVENTS_TXDRDY = 0;
    UART->TASKS_STARTTX = 1;


    UART->TXD = ((uint8_t *)buf)[i];

    while (!UART->EVENTS_TXDRDY) { }
  }
}
