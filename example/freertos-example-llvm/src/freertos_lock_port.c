#include "example_project/mutex.h"

#include <stdbool.h>

#include "example_project/examples.h"

#include "FreeRTOS.h"
#include "semphr.h"

static SemaphoreHandle_t s_flash_lock;
static SemaphoreHandle_t s_accel_lock;

void example_locks_boot(void) {
  static bool s_initialized = false;
  if (s_initialized) {
    return;
  }
  s_flash_lock = xSemaphoreCreateMutex();
  s_accel_lock = xSemaphoreCreateMutex();

  s_initialized = true;
}

EXAMPLE_PROJ_FUNC_DISABLE_LOCK_CHECKS
void flash_lock(void) {
  xSemaphoreTake(s_flash_lock, portMAX_DELAY);
}

EXAMPLE_PROJ_FUNC_DISABLE_LOCK_CHECKS
void flash_unlock(void) {
  xSemaphoreGive(s_flash_lock);
}

EXAMPLE_PROJ_FUNC_DISABLE_LOCK_CHECKS
void accel_lock(void) {
  xSemaphoreTake(s_accel_lock, portMAX_DELAY);
}

EXAMPLE_PROJ_FUNC_DISABLE_LOCK_CHECKS
void accel_unlock(void) {
  xSemaphoreGive(s_accel_lock);
}
