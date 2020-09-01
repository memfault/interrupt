#include <stdint.h>

#define EXAMPLE_WEAK __attribute__((weak))

EXAMPLE_WEAK void MemoryManagement_Handler(void) { }
EXAMPLE_WEAK void BusFault_Handler(void) { }
EXAMPLE_WEAK void UsageFault_Handler(void) { }

void Reset_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void HardFault_Handler(void);
void NMI_Handler(void);
void Irq1_Handler(void);
void Irq2_Handler(void);
void Irq3_Handler(void);
void Irq4_Handler(void);
void DefaultIrq_Handler(void);

EXAMPLE_WEAK void DefaultIrq_Handler(void) {
  while (1) { }
}

// NB: Depending on the test config, these may be overriden
EXAMPLE_WEAK void Irq0_Handler(void) {
  DefaultIrq_Handler();
}
EXAMPLE_WEAK void Irq1_Handler(void) {
  DefaultIrq_Handler();
}
EXAMPLE_WEAK void Irq2_Handler(void) {
  DefaultIrq_Handler();
}
EXAMPLE_WEAK void Irq3_Handler(void) {
  DefaultIrq_Handler();
}
EXAMPLE_WEAK void HardFault_Handler(void) {
  DefaultIrq_Handler();
}
EXAMPLE_WEAK void NMI_Handler(void) {
  DefaultIrq_Handler();
}


// Variable defined in linker script
extern uint32_t _estack;
#define STACK_START &_estack
#define RESET_VECTOR Reset_Handler
#define PLACE_IN_VECTOR_TABLE_SECTION __attribute__((section(".isr_vector")))


// A minimal vector table for a Cortex M.
PLACE_IN_VECTOR_TABLE_SECTION void (*const g_pfnVectors[])(void) = {
    (void (*)(void))(STACK_START), // initial stack pointer
    RESET_VECTOR,
    NMI_Handler,
    HardFault_Handler,
    MemoryManagement_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    DefaultIrq_Handler,
    DefaultIrq_Handler,
    DefaultIrq_Handler,
    DefaultIrq_Handler,
    SVC_Handler,
    DefaultIrq_Handler,
    DefaultIrq_Handler,
    PendSV_Handler,
    SysTick_Handler,
    DefaultIrq_Handler,
    [16 + 0] = Irq0_Handler,
    [16 + 1] = Irq1_Handler,
    [16 + 2] = Irq2_Handler,
    [16 + 3] = Irq3_Handler,
};
