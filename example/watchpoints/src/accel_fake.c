//! @file
//!


#include "accel.h"

#include <stddef.h>

#include "hal/logging.h"

static AccelSampleProcessedCallback s_data_processed_cb = NULL;

// Disable optimizations here for cleaner line resolution
// in GDB
__attribute__((optimize("O0")))
void accel_register_watcher(
    AccelSampleProcessedCallback data_processed_cb) {
  s_data_processed_cb = data_processed_cb;
}

void accel_process_reading(int x, int y, int z) {
  EXAMPLE_LOG("Processing Accel Reading ...");
  if (s_data_processed_cb == NULL) {
    return;
  }

  // ... process raw readings ...

  // notify consumer new data is available
  s_data_processed_cb();
}
