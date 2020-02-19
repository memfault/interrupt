#include "hardware_watchdog.h"

#include <stdint.h>

#include "cmsis_shim.h"

// NOTE: For brevity, a hand rolled struct for the NRF52 Watchdog
// Peripheral. Typically, this definition would be sourced from
// the headers provided with the NRF52 SDK.
typedef struct __attribute__((packed)) Nrf52Wdt {
  __OM uint32_t TASKS_START;
  __IM uint32_t RSVD[63];
  __IOM uint32_t EVENTS_TIMEOUT;
  __IM uint32_t RSVD1[128];
  __IOM uint32_t INTENSET;
  __IOM uint32_t INTENCLR;
  __IM uint32_t RSVD2[61];
  __IM uint32_t RUNSTATUS;
  __IM uint32_t REQSTATUS;
  __IM uint32_t RSVD3[63];
  __IOM uint32_t CRV;
  __IOM uint32_t RREN;
  __IOM uint32_t CONFIG;
  __IM uint32_t RSVD4[60];
  __OM uint32_t RR[8];
} sNrf52Wdt;
static sNrf52Wdt *const WDT = ((sNrf52Wdt *)0x40010000UL);

void hardware_watchdog_enable(void) {
  if ((WDT->RUNSTATUS & 0x1) != 0) {
    // the watchdog is already running and cannot be configured
    hardware_watchdog_feed();
    return;
  }

  // "Counter reload value" - The number of 32.768kHz clock cycles to elapse
  // before a watchdog timeout
  WDT->CRV = 32768 * HARDWARE_WATCHDOG_TIMEOUT_SECS;

  // Starts the watchdog peripheral
  WDT->TASKS_START = 0x1;
}

void hardware_watchdog_feed(void) {
  // Per "6.36.4.10" if this value is written to a reload register that is
  // enabled, the watchdog counter will be reset
  const uint32_t reload_magic_value = 0x6E524635;
  WDT->RR[0] = reload_magic_value;
}
