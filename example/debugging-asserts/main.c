#include "FreeRTOS.h"
#include "task.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "impl_common.h"

// Return true or false
static bool prv_rand_bool(void) {
  return (bool)(rand() / 2);
}

void prv_main_path_A() {
  bool two = prv_rand_bool();
  if (two) {
    assert_path_A();
  } else {
    assert_path_B();
  }
}

void prv_main_path_B() {
  bool two = prv_rand_bool();
  if (two) {
    assert_path_A();
  } else {
    assert_path_B();
  }
}

static void prv_main(void *pvParameters) {
  while (1) { 
    bool one = prv_rand_bool();
    if (one) {
      prv_main_path_A();
    } else {
      prv_main_path_B();
    }
  }
}

// --------------------------
// Module Internals
// --------------------------

sAssertInfo g_assert_info;

// --------------------------
// FreeRTOS Internals
// --------------------------

// the FreeRTOS heap
uint8_t ucHeap[configTOTAL_HEAP_SIZE];

/* Priorities at which the tasks are created. */
#define mainQUEUE_MAIN_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

void vAssertCalled(const char *file, int line) {
  __asm("bkpt 0");
}

int main(void) {
  xTaskCreate(prv_main,
              "Main", 2048,
              NULL,
              mainQUEUE_MAIN_TASK_PRIORITY,
              NULL);

  vTaskStartScheduler();

  // should be unreachable
  configASSERT(0);
  return -1;
}
