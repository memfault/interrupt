//! @file
//!
//! A pure C-based Reset Handler and Vector Table for Cortex-M devices

#include <stdint.h>

extern int main(void);

// Following symbols are defined by the linker.
// Start address for the initialization values of the .data section.
extern uint32_t _sidata;
// Start address for the .data section
extern uint32_t _sdata;
// End address for the .data section
extern uint32_t _edata;
// Start address for the .bss section
extern uint32_t _sbss;
// End address for the .bss section
extern uint32_t _ebss;
// End address for stack
extern uint32_t _estack;

// Prevent inlining to avoid persisting any variables on stack
__attribute__((noinline)) static void prv_cinit(void) {
  // Initialize data and bss
  // Copy the data segment initializers from flash to SRAM
  for (uint32_t *dst = &_sdata, *src = &_sidata; dst < &_edata;) {
    *(dst++) = *(src++);
  }

  // Zero fill the bss segment.
  for (uint32_t *dst = &_sbss; dst < &_ebss;) {
    *(dst++) = 0;
  }
}

void Reset_Handler(void) {
  prv_cinit();

  // Call the application's entry point.
  main();

  // should be unreachable
  while (1) { }
}

// DefaultIntHandler is used for unpopulated interrupts
static void DefaultIntHandler(void) {
  __asm__("bkpt");
  // Go into an infinite loop.
  while (1);
}

static void NMI_Handler(void) {
  DefaultIntHandler();
}

static void HardFault_Handler(void) {
  DefaultIntHandler();
}

void DebugMon_Handler(void);
void Irq2_Handler(void);

#define EXTERNAL_INT_BASE 16 // NVIC Interrupt 0 starts here
// A minimal vector table for a Cortex M.
__attribute__((section(".isr_vector"))) void (*const g_pfnVectors[])(void) = {
    [0] = (void *)(&_estack), // initial stack pointer
    [1] = Reset_Handler,
    [2] = NMI_Handler,
    [3] = HardFault_Handler,
    [4] = DefaultIntHandler,
    [5] = DefaultIntHandler,
    [6] = DefaultIntHandler,
    [7] = DefaultIntHandler,
    [8] = DefaultIntHandler,
    [9] = DefaultIntHandler,
    [10] = DefaultIntHandler,
    [11] = DefaultIntHandler,
    [12] = DebugMon_Handler,
    [13] = DefaultIntHandler,
    [14] = DefaultIntHandler,
    [15] = DefaultIntHandler,
    // NVIC Interrupts
    [16 + 2] = Irq2_Handler, // Uart
};
