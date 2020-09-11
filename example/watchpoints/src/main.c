#include <stdio.h>

#include "cmsis_shim.h"
#include "hal/uart.h"
#include "hal/logging.h"

#include "accel.h"
#include "graphics.h"
#include "shell_port.h"

#include <string.h>

//! A very naive implementation of the newlib _sbrk dependency function
caddr_t _sbrk(int incr);
caddr_t _sbrk(int incr) {
  static uint32_t s_index = 0;
  static uint8_t s_newlib_heap[2048] __attribute__((aligned(8)));

  if ((s_index + (uint32_t)incr) <= sizeof(s_newlib_heap)) {
    EXAMPLE_LOG("Out of Memory!");
    return 0;
  }

  caddr_t result = (caddr_t)&s_newlib_heap[s_index];
  s_index += (uint32_t)incr;
  return result;
}

__attribute__((noinline))
static void prv_enable_vfp( void ){
  volatile uint32_t *cpacr = (volatile uint32_t *)0xE000ED88;
  *cpacr |= 0xf << 20;
}

void accel_data_processed(void) {
  EXAMPLE_LOG("Accel Sample Processing Complete!");
  // ... do work with new samples
}

static uint16_t s_graphics_buf[2];

int main(void) {
  prv_enable_vfp();
  uart_boot();

  accel_register_watcher(accel_data_processed);
  graphics_boot(s_graphics_buf, sizeof(s_graphics_buf));

  EXAMPLE_LOG("==Booted==");

  shell_processing_loop();

  __builtin_unreachable();
}
