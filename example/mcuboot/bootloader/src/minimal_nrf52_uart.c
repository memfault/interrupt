//! @file
//! In the interest of brevity, an _extremely_ barebones port for the NRF52 UART Peripheral

#include "hal/uart.h"

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
  __IM  uint32_t  RESERVED[7];
  __OM  uint32_t  TASKS_FLUSHRX;
  __IM  uint32_t  RESERVED1[52];
  __IOM uint32_t  EVENTS_CTS;
  __IOM uint32_t  EVENTS_NCTS;
  __IOM uint32_t  EVENTS_RXDRDY;

  __IM  uint32_t  RESERVED2;
  __IOM uint32_t  EVENTS_ENDRX;
  __IM  uint32_t  RESERVED3[2];
  __IOM uint32_t  EVENTS_TXDRDY;
  __IOM uint32_t  EVENTS_ENDTX;
  __IOM uint32_t  EVENTS_ERROR;
  __IM  uint32_t  RESERVED4[7];
  __IOM uint32_t  EVENTS_RXTO;
  __IM  uint32_t  RESERVED5;
  __IOM uint32_t  EVENTS_RXSTARTED;
  __IOM uint32_t  EVENTS_TXSTARTED;
  __IM  uint32_t  RESERVED6;
  __IOM uint32_t  EVENTS_TXSTOPPED;
  __IM  uint32_t  RESERVED7[41];
  __IOM uint32_t  SHORTS;
  __IM  uint32_t  RESERVED8[63];
  __IOM uint32_t  INTEN;
  __IOM uint32_t  INTENSET;
  __IOM uint32_t  INTENCLR;
  __IM  uint32_t  RESERVED9[93];
  __IOM uint32_t  ERRORSRC;
  __IM  uint32_t  RESERVED10[31];
  __IOM uint32_t  ENABLE;
  __IM  uint32_t  RESERVED11;
  struct {
    __IOM uint32_t  RTS;
    __IOM uint32_t  TXD;
    __IOM uint32_t  CTS;
    __IOM uint32_t  RXD;
  } PSEL;
  __IM  uint32_t  RESERVED12[3];
  __IOM uint32_t  BAUDRATE;
  __IM  uint32_t  RESERVED13[3];
  struct {
    __IOM uint32_t  PTR;
    __IOM uint32_t  MAXCNT;
    __IM  uint32_t  AMOUNT;
  } RXD;
  __IM  uint32_t  RESERVED14;
  struct {
    __IOM uint32_t  PTR;
    __IOM uint32_t  MAXCNT;
    __IM  uint32_t  AMOUNT;
  } TXD;
  __IM  uint32_t  RESERVED15[7];
  __IOM uint32_t  CONFIG;
} sNrfUarteConfig;

static sNrfUarteConfig *const UARTE = ((sNrfUarteConfig *)0x40002000);

static uint8_t s_rx_recv_buf[4];

static void prv_enable_nvic(int exti_id) {
  volatile uint32_t *nvic_ipr = (void *)(0xE000E400 + 4 * (exti_id / 4));
  *nvic_ipr = 0xe0 << ((exti_id % 4) * 8);

  volatile uint32_t *nvic_iser = (void *)0xE000E100;
  *nvic_iser |= (1 << (exti_id % 32));
}

void uart_boot(void) {
  UARTE->PSEL.RTS = 5;
  UARTE->PSEL.TXD = 6;
  UARTE->PSEL.CTS = 7;
  UARTE->PSEL.RXD = 8;

  UARTE->BAUDRATE = 0x01D60000; // 115200

  // no parity, 1 stop bit, flow control
  UARTE->CONFIG = 1;
  UARTE->ENABLE = 8;

  memset(s_rx_recv_buf, 0x0, sizeof(s_rx_recv_buf));
  UARTE->RXD.PTR = (uint32_t)s_rx_recv_buf;
  UARTE->RXD.MAXCNT = 1;

  prv_enable_nvic(2);
  UARTE->INTENSET |= (1 << 4);
  UARTE->TASKS_STARTRX = 1;
}

void uart_tx_blocking(void *buf, size_t buf_len) {
  UARTE->EVENTS_ENDTX = 0;
  UARTE->EVENTS_TXSTOPPED = 0;

  UARTE->TXD.PTR = (uint32_t)buf;
  UARTE->TXD.MAXCNT = buf_len;

  UARTE->TASKS_STARTTX = 1;

  while (!UARTE->EVENTS_ENDTX && !UARTE->EVENTS_TXSTOPPED) {  }
}

void Irq2_Handler(void) {
  if (UARTE->EVENTS_ENDRX != 0) {
    UARTE->EVENTS_ENDRX = 0;

    char c = s_rx_recv_buf[0];
    uart_byte_received_from_isr_cb(c);

    // re-arm
    UARTE->TASKS_STARTRX = 1;
  }
}
