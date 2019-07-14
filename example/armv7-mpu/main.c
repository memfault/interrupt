//! @file
//! A drop in replacement for main.c in the NRF52 V15.2 SDK located at:
//!  examples/peripheral/blinky/pca10056/main.c
//!
//! This was tested running "make" from:
//!  examples/peripheral/blinky/pca10056/blank/armgcc/
//!
//! NOTE: For simplicity, the example code makes use of a couple GCC specific extensions but could
//! be easily modified to target other compilers

#include <stdbool.h>
#include <stdint.h>

#include "boards.h"
#include "nrf_delay.h"

// Four modes:
//  1: MPU crash due to stack overflow
//  2: MPU crash due to write issued to flash region
//  3: MPU crash due to executing data as code
//  Anything Else: No crashes enabled
#ifndef BLINK_MPU_EXAMPLE_CONFIG
#define BLINK_MPU_EXAMPLE_CONFIG 0
#endif

// Make the variable global so the compiler doesn't optimize away the setting
// and the crash taken can be overriden from gdb without having to recompile
// the app. For example, to cause a stack overflow crash:
//
// (gdb) break main
// (gdb) continue
// (gdb) set g_crash_config=1
// (gdb) continue
int g_crash_config = BLINK_MPU_EXAMPLE_CONFIG;

__attribute__((optimize("O0")))
int recursive_sum(int n) {
  if (n == 0) {
    return 0;
  }

  return n + recursive_sum(n - 1);
}

__attribute__((optimize("O0")))
void invalid_write_to_flash(void) {
  volatile uint32_t *bad_pointer = (void *)0x0;
  *bad_pointer = 0xdeadbeef;
}

// An array of data that "happens" to have a pattern of valid
// arm instructions.
//
// Note: We are aligning the data along a boundary that matches its
// size for simplicity in our MPU configuration example
static uint16_t s_data[64] __attribute__((aligned(128))) = {
  0xbf00, // nop
  0xbf00, // nop
  0xdf00, // svc 0
  0x4770, // bx lr
};

__attribute__((optimize("O0")))
void execute_data_array_as_code(void) {
  uint32_t func_addr = (uint32_t)&s_data[0];
  // bit[0] of a function pointer must be set to 1 per per
  // "ARMv7-M and interworking support" section of reference manual
  func_addr |= 0x1;

  // perform a read/write
  s_data[5] |= 0xabab;

  // execute the array like it's a function
  void (*data_as_function)(void) = (void *)func_addr;
  data_as_function();
}

// The NRF52 fault handlers can be overriden because they are declared as weak
// Let's override the handler and insert a breakpoint when it's hit
__attribute__((naked))
void MemoryManagement_Handler(void) {
  __asm("bkpt 1");
}

int main(void) {
  bsp_board_init(BSP_INIT_LEDS);

  // Let's set MEMFAULTENA so MemManage faults get tripped
  // (otherwise we will immediately get a HardFault)
  volatile uint32_t *shcsr = (void *)0xE000ED24;
  *shcsr |= (0x1 << 16);

  volatile uint32_t *mpu_rbar = (void *)0xE000ED9C;
  volatile uint32_t *mpu_rasr = (void *)0xE000EDA0;

  //
  // Setup MPU Region 0 to check for stack overflow
  //

  extern uint32_t __StackLimit;
  // align base address to nearest 64 byte boundary
  // because it needs to match SIZE
  uint32_t mask = 64 - 1;
  uint32_t base_addr = ((uint32_t)&__StackLimit + mask) & ~mask;

  // Let's set the base_address and use the 'VALID'
  // bit to make sure region 0 is active in 'MPU_RNR'
  *mpu_rbar = (base_addr | 1 << 4 | 0);

  // MPU_RASR settings for Stack MPU Region:
  //  AP=0b000 because we don't want to allow _any_ access
  //  TEXSCB=0b000110 because the Stack is in "Internal SRAM"
  //  SIZE=5 because we want a 64 byte MPU region
  //  ENABLE=1
  *mpu_rasr = (0b000 << 24) | (0b000110 << 16) | (5 << 1) | 0x1;

  //
  // Setup MPU Region 1 to catch writes to internal flash
  //

  // The NRF52840 internal flash is 1MB and starts at address 0x0
  // No need to mask the address since it's fixed and 1MB aligned
  base_addr = 0x0;
  // Let's use the 'VALID' bit to select region 1 in 'MPU_RNR'
  *mpu_rbar = (base_addr | 1 << 4 | 1);

  // MPU_RASR settings for flash write protection:
  //  AP=0b110 to make the region read-only regardless of privilege
  //  TEXSCB=0b000010 because the Code is in "Flash memory"
  //  SIZE=19 because we want to cover 1MB
  //  ENABLE=1
  *mpu_rasr = (0b110 << 24) | (0b000010 << 16) | (19 << 1) | 0x1;

  //
  // Setup MPU Region 2 to catch attempts to execute a s_data as code
  //

  // The s_data struct is 128 bytes in size and is already 128 byte aligned
  base_addr = (uint32_t)&s_data[0];
  // Let's use the 'VALID' bit to select region 2 in 'MPU_RNR'
  *mpu_rbar = (base_addr | 1 << 4 | 2);

  // MPU_RASR settings for flash write protection:
  //  XN=1 to block _any_ attempts to execute the region as code
  //  AP=0b011 to allow full read/write access to the region
  //  TEXSCB=0b000110 because the Stack is in "Internal SRAM"
  //  SIZE=6 because we want to cover 128 bytes
  //  ENABLE=1
  *mpu_rasr =
      (0b1 << 28) | (0b011 << 24) | (0b000110 << 16) | (6 << 1) | 0x1;

  // Finally, activate the MPU and default memory map (PRIVDEFENA)
  volatile uint32_t *mpu_ctrl = (void *)0xE000ED94;
  *mpu_ctrl = 0x5;

  while (true) {
    for (int i = 0; i < LEDS_NUMBER; i++) {
      if (g_crash_config == 1) {
        recursive_sum(600);
      } else if (g_crash_config == 2) {
        invalid_write_to_flash();
      } else if (g_crash_config == 3) {
        execute_data_array_as_code();
      }
      bsp_board_led_invert(i);
      nrf_delay_ms(500);
    }
  }
}
