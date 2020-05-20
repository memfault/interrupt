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
  //(void)main();

  main();

  // should be unreachable
  while (1) { }
}

// DefaultIntHandler is used for unpopulated interrupts
static void DefaultIntHandler(void) {
  __asm__("bkpt");
  // Go into an infinite loop.
  while (1)
    ;
}

static void NMI_Handler(void) {
  DefaultIntHandler();
}

static void HardFault_Handler(void) {
  DefaultIntHandler();
}

static void MemoryManagement_Handler(void) {

}

static void BusFault_Handler(void) {

}

static void UsageFault_Handler(void) {

}

void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

// A minimal vector table for a Cortex M.
extern void (*const g_pfnVectors[16])(void);
__attribute__((section(".isr_vector"))) void (*const g_pfnVectors[16])(void) = {
    (void *)(&_estack), // initial stack pointer
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemoryManagement_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0,
    0,
    0,
    0,
    SVC_Handler,
    0,
    0,
    PendSV_Handler,
    SysTick_Handler,
};
