//! @file
//! A drop in replacement for main.c in the NRF52 V15.2 SDK located at:
//!  examples/peripheral/blinky/pca10056/main.c
//!
//! This was tested running "make" from:
//!  examples/peripheral/blinky/pca10056/blank/armgcc/
//!
//! NOTE: For simplicity, the example code makes use of a couple GCC specific
//! extensions but could be easily modified to target other compilers

#include <stdbool.h>
#include <stdint.h>

#include "boards.h"
#include "nrf_delay.h"

// Four modes:
//  1: Trigger a PendSV Exception
//  2: Trigger External Interrupt 0 in the NVIC and then pre-empt it with a
//  PendSV 3: Trigger 3 External Interrupts in the NVIC of varying priority
//  levels at the same time Anything Else: Nothing happens
#ifndef BLINK_EXCEPTION_EXAMPLE_CONFIG
#define BLINK_EXCEPTION_EXAMPLE_CONFIG 1
#endif

// Make the variable global so the compiler doesn't optimize away the setting
// and the crash taken can be overriden from gdb without having to recompile
// the app. For example, to cause a stack overflow crash:
//
// (gdb) break main
// (gdb) continue
// (gdb) set g_exception_example_config=1
// (gdb) continue
int g_exception_example_config = BLINK_EXCEPTION_EXAMPLE_CONFIG;

void PendSV_Handler(void) { __asm("bkpt 1"); }

__attribute__((optimize("O0"))) static void trigger_pendsv(void) {
  volatile uint32_t *icsr = (void *)0xE000ED04;
  // Pend a PendSV exception using by writing 1 to PENDSVSET at bit 28
  *icsr = 0x1 << 28;
  // flush pipeline to ensure exception takes effect before we
  // return from this routine
  __asm("isb");
}

__attribute__((optimize("O0"))) void POWER_CLOCK_IRQHandler(void) {
  __asm("bkpt 2");
  trigger_pendsv();
  __asm("bkpt 3");
}

static void trigger_nvic_int0(void) {
  // Let's set the interrupt priority to be the
  // lowest possible for the NRF52. Note the default
  // NVIC priority is zero which would match our current pendsv
  // config so no pre-emption would take place if we didn't change this
  volatile uint32_t *nvic_ipr = (void *)0xE000E400;
  *nvic_ipr = 0xe0;

  // Enable the POWER_CLOCK_IRQ (External Interrupt 0)
  volatile uint32_t *nvic_iser = (void *)0xE000E100;
  *nvic_iser |= 0x1;

  // Pend an interrupt
  volatile uint32_t *nvic_ispr = (void *)0xE000E200;
  *nvic_ispr |= 0x1;

  // flush pipeline to ensure exception takes effect before we
  // return from this routine
  __asm("isb");
}

// External Interrupt 9
void TIMER1_IRQHandler(void) {
  __asm("bkpt 4");
}

// External Interrupt 10
void TIMER2_IRQHandler(void) {
  __asm("bkpt 5");
}

// External Interrupt 11
void RTC0_IRQHandler(void) {
  __asm("bkpt 6");
}

static void trigger_nvic_int9_int10_int11(void) {
  // Let's prioritize the interrupts with 9 having the lowest priority
  // and 10 & 11 having the same higher priority.

  // Each interrupt has 8 config bits allocated so
  // 4 interrupts can be configured per 32-bit register. This
  // means 9, 10, 11 are next to each other in IPR[2]
  volatile uint32_t *nvic_ipr2 = (void *)(0xE000E400 + 8);
  // Only 3 priority bits are implemented so we need to program
  // the upper 3 bits of each mask
  *nvic_ipr2 |= (0x7 << 5) << 8;
  *nvic_ipr2 |= (0x6 << 5) << 16;
  *nvic_ipr2 |= (0x6 << 5) << 24;

  // Enable interrupts for TIMER1_IRQHandler,
  // TIMER2_IRQHandler & RTC0_IRQHandler
  volatile uint32_t *nvic_iser = (void *)0xE000E100;
  *nvic_iser |= (0x1 << 9) | (0x1 << 10) | (0x1 << 11);

  // Pend an interrupt
  volatile uint32_t *nvic_ispr = (void *)0xE000E200;
  *nvic_ispr |= (0x1 << 9) | (0x1 << 10) | (0x1 << 11);

  // flush pipeline to ensure exception takes effect before we
  // return from this routine
  __asm("isb");
}

int main(int a, char *argv[]) {
  while (true) {
    for (int i = 0; i < LEDS_NUMBER; i++) {
      if (g_exception_example_config == 1) {
        trigger_pendsv();
      } else if (g_exception_example_config == 2) {
        trigger_nvic_int0();
      } else if (g_exception_example_config == 3) {
        trigger_nvic_int9_int10_int11();
      }
      bsp_board_led_invert(i);
      nrf_delay_ms(500);
    }
  }
}
