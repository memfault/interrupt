#include "example_project/mutex.h"
#include "example_project/examples.h"

#include <stdbool.h>

void example_flash_lock_bug(void) {
  flash_lock();

  int rv = do_some_work_while_holding_locks();
  if (rv == -1) {
    return;
  }
  flash_unlock();
}

void example_accel_lock_bug(void) {
  example_func_requires_accel();
}

void example_run_mutex_examples(void) {
  example_locks_boot();
  example_flash_lock_bug();
  example_accel_lock_bug();
}
