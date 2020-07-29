#include <stdio.h>

#include "cmsis_shim.h"
#include "hal/uart.h"
#include "hal/logging.h"

#include "shell_port.h"

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
  __asm volatile
      (
          "ldr.w r0, =0xE000ED88      \n" /* The FPU enable bits are in the CPACR. */
          "ldr r1, [r0]               \n"
          "orr r1, r1, #( 0xf << 20 ) \n" /* Enable CP10 and CP11 coprocessors, then save back. */
          "str r1, [r0]               \n"
          "bx r14                      "
       );
}

int main(void) {
  prv_enable_vfp();
  uart_boot();

  EXAMPLE_LOG("==Booted==");

  shell_processing_loop();

  __builtin_unreachable();
}
