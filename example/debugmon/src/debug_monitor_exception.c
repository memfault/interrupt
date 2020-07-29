#include <stdbool.h>

#include "cmsis_shim.h"
#include "hal/uart.h"
#include "hal/logging.h"
#include "dummy_functions.h"
#include "shell_port.h"
//#include "literal_remap_example.h"
#include "debug_monitor.h"
#include "fpb.h"


typedef struct __attribute__((packed)) ContextStateFrame {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr;
  uint32_t return_address;
  uint32_t xpsr;
} sContextStateFrame;

typedef enum {
  kDebugState_None,
  kDebugState_SingleStep,
} eDebugState;

static eDebugState s_user_requested_debug_state = kDebugState_None;

void debug_monitor_handler_c(sContextStateFrame *frame) {
  volatile uint32_t *demcr = (uint32_t *)0xE000EDFC;

  volatile uint32_t *dfsr = (uint32_t *)0xE000ED30;
  const uint32_t dfsr_dwt_evt_bitmask = (1 << 2);
  const uint32_t dfsr_bkpt_evt_bitmask = (1 << 1);
  const uint32_t dfsr_halt_evt_bitmask = (1 << 0);
  const bool is_dwt_dbg_evt = (*dfsr & dfsr_dwt_evt_bitmask);
  const bool is_bkpt_dbg_evt = (*dfsr & dfsr_bkpt_evt_bitmask);
  const bool is_halt_dbg_evt = (*dfsr & dfsr_halt_evt_bitmask);

  EXAMPLE_LOG("DebugMonitor Exception");

  EXAMPLE_LOG("DEMCR: 0x%08x", *demcr);
  EXAMPLE_LOG("DFSR:  0x%08x (bkpt=%d, halt=%d, dwt=%d)", *dfsr,
              (int)is_bkpt_dbg_evt, (int)is_halt_dbg_evt,
              (int)is_dwt_dbg_evt);

  EXAMPLE_LOG("Register Dump");
  EXAMPLE_LOG(" r0  =0x%08x", frame->r0);
  EXAMPLE_LOG(" r1  =0x%08x", frame->r1);
  EXAMPLE_LOG(" r2  =0x%08x", frame->r2);
  EXAMPLE_LOG(" r3  =0x%08x", frame->r3);
  EXAMPLE_LOG(" r12 =0x%08x", frame->r12);
  EXAMPLE_LOG(" lr  =0x%08x", frame->lr);
  EXAMPLE_LOG(" pc  =0x%08x", frame->return_address);
  EXAMPLE_LOG(" xpsr=0x%08x", frame->xpsr);

  if (is_dwt_dbg_evt || is_bkpt_dbg_evt ||
      (s_user_requested_debug_state == kDebugState_SingleStep))  {
    EXAMPLE_LOG("Debug Event Detected, Awaiting 'c' or 's'");
    while (1) {
      char c;
      if (!shell_port_getchar(&c)) {
        continue;
      }

      EXAMPLE_LOG("Got char '%c'!\n", c);
      if (c == 'c') { // 'c' == 'continue'
        s_user_requested_debug_state = kDebugState_None;
        break;
      } else if (c == 's') { // 's' == 'single step'
        s_user_requested_debug_state = kDebugState_SingleStep;
        break;
      }
    }
  } else {
    EXAMPLE_LOG("Resuming ...");
  }

  const uint32_t demcr_single_step_mask = (1 << 18);

  if (is_bkpt_dbg_evt) {
    const uint16_t instruction = *(uint16_t*)frame->return_address;
    if ((instruction & 0xff00) == 0xbe00) {
      // advance past breakpoint instruction
      frame->return_address += sizeof(instruction);
    } else {
      // It's a FPB generated breakpoint
      // We need to disable the FPB and single-step
      fpb_disable();
      EXAMPLE_LOG("Single-Stepping over FPB at 0x%x", frame->return_address);
    }

    // single-step to the next instruction
    // This will cause a DebugMonitor interrupt to fire
    // once we return from the exception and a single
    // instruction has been executed. The HALTED bit
    // will be set in the DFSR when this happens.
    *demcr |= (demcr_single_step_mask);
    // We have serviced the breakpoint event so clear mask
    *dfsr = dfsr_bkpt_evt_bitmask;
  } else if (is_halt_dbg_evt) {
    // re-enable FPB in case we got here via single-step
    // for a BKPT debug event
    fpb_enable();

    if (s_user_requested_debug_state != kDebugState_SingleStep) {
      *demcr &= ~(demcr_single_step_mask);
    }

    // We have serviced the single step event so clear mask
    *dfsr = dfsr_halt_evt_bitmask;
  } else if (is_dwt_dbg_evt) {
    // Future exercise: handle DWT debug events
    *dfsr = dfsr_dwt_evt_bitmask;
  }
}

__attribute__((naked))
void DebugMon_Handler(void) {
  __asm volatile(
      "tst lr, #4 \n"
      "ite eq \n"
      "mrseq r0, msp \n"
      "mrsne r0, psp \n"
      "b debug_monitor_handler_c \n");
}

static void prv_enable(bool do_enable) {
  volatile uint32_t *demcr = (uint32_t *)0xE000EDFC;
  const uint32_t mon_en_bit = 16;
  if (do_enable) {
    // clear any stale state in the DFSR
    volatile uint32_t *dfsr = (uint32_t*)0xE000ED30;
    *dfsr = *dfsr;
    *demcr |= 1 << mon_en_bit;
  } else {
    *demcr &= ~(1 << mon_en_bit);
  }
}

static bool prv_halting_debug_enabled(void) {
  volatile uint32_t *dhcsr = (uint32_t *)0xE000EDF0;
  return (((*dhcsr) & 0x1) != 0);
}

bool debug_monitor_enable(void) {
  if (prv_halting_debug_enabled()) {
    EXAMPLE_LOG("Halting Debug Enabled - "
                "Can't Enable Monitor Mode Debug!");
    return false;
  }
  prv_enable(true);


  // Priority for DebugMonitor Exception is bits[7:0].
  // We will use the lowest priority so other ISRs can
  // fire while in the DebugMonitor Interrupt
  volatile uint32_t *shpr3 = (uint32_t *)0xE000ED20;
  *shpr3 = 0xff;

  EXAMPLE_LOG("Monitor Mode Debug Enabled!");
  return true;
}

bool debug_monitor_disable(void) {
  prv_enable(false);
  return true;
}
