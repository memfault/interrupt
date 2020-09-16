#include <stddef.h>
#include <stdint.h>

#include "hal/logging.h"

typedef struct {
  volatile uint32_t COMP;
  volatile uint32_t MASK;
  volatile uint32_t FUNCTION;
  volatile uint32_t RSVD;
} sDwtCompCfg;

typedef struct {
  volatile uint32_t CTRL;
  volatile uint32_t CYCCNT;
  volatile uint32_t CPICNT;
  volatile uint32_t EXCCNT;
  volatile uint32_t SLEEPCNT;
  volatile uint32_t LSUCNT;
  volatile uint32_t FOLDCNT;
  volatile const  uint32_t PCSR;
  sDwtCompCfg COMP_CONFIG[];
} sDWTUnit;

static sDWTUnit *const DWT = (sDWTUnit *)0xE0001000;

void dwt_dump(void) {
  EXAMPLE_LOG("DWT Dump:");
  EXAMPLE_LOG(" DWT_CTRL=0x%x", DWT->CTRL);

  const size_t num_comparators = (DWT->CTRL>>28) & 0xF;
  EXAMPLE_LOG("   NUMCOMP=0x%x", num_comparators);

  for (size_t i = 0; i < num_comparators; i++) {
    const sDwtCompCfg *config = &DWT->COMP_CONFIG[i];

    EXAMPLE_LOG(" Comparator %d Config", (int)i);
    EXAMPLE_LOG("  0x%08x DWT_FUNC%d: 0x%08x",
                &config->FUNCTION, (int)i, config->FUNCTION);
    EXAMPLE_LOG("  0x%08x DWT_COMP%d: 0x%08x",
                &config->COMP, (int)i, config->COMP);
    EXAMPLE_LOG("  0x%08x DWT_MASK%d: 0x%08x",
                &config->MASK, (int)i, config->MASK);
  }
}

void dwt_reset(void) {
  const size_t num_comparators = (DWT->CTRL>>28) & 0xF;

  for (size_t i = 0; i < num_comparators; i++) {
    sDwtCompCfg *config = &DWT->COMP_CONFIG[i];
    config->FUNCTION = config->COMP = config->MASK = 0;
  }
}

void dwt_install_watchpoint(int comp_id, uint32_t func, uint32_t comp, uint32_t mask) {
  const size_t num_comparators = (DWT->CTRL>>28) & 0xF;
  if (comp_id > num_comparators) {
    EXAMPLE_LOG("Invalid COMP_ID of %d", comp_id);
    return;
  }

  sDwtCompCfg *config = &DWT->COMP_CONFIG[comp_id];
  config->FUNCTION = 0; // disable while we configure

  config->COMP = comp;
  config->MASK = mask;
  config->FUNCTION = func;
}
