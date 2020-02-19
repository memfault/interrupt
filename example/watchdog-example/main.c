#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "cmsis_shim.h"
#include "hardware_watchdog.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

// Modes:
//  0: while (1) { } loop which does _not_ feed the watchdog
//  1: while (1) { } loop which feeds the watchdog
//  2: Hang while busy looping for external flash erase complete
//  3: Deadlock
//  4: Hang from an ISR
// Any other value: Normal Operation
#ifndef WATCHDOG_HANG_CONFIG
#define WATCHDOG_HANG_CONFIG 0
#endif

// Make the variable global so the compiler doesn't optimize away the setting
// and the crash taken can be overriden from gdb without having to recompile
// the app. For example, to cause a stack overflow crash:
//
// (gdb) break main
// (gdb) continue
// (gdb) set g_watchdog_example_config=1
// (gdb) continue
int g_watchdog_hang_config = WATCHDOG_HANG_CONFIG;

// the FreeRTOS heap
uint8_t ucHeap[configTOTAL_HEAP_SIZE];
static QueueHandle_t xQueue = NULL;
static SemaphoreHandle_t s_temp_i2c_mutex = NULL;

int i2c_read_temp(uint32_t *temp);
bool spi_flash_erase_complete(void);

void ExternalInt0_Handler(void) {
  while (1) {}
}

int read_temp_sensor(uint32_t *temp) {
  xSemaphoreTake(s_temp_i2c_mutex, portMAX_DELAY);
  int rv = i2c_read_temp(temp);
  if (rv == -1) {
    // BUG: Semaphore should have been released here!
    return rv;
  }
  xSemaphoreGive(s_temp_i2c_mutex);
  return 0;
}

void erase_external_flash(void) {
  // some logic to start a flash erase
  // poll for completion
  while (!spi_flash_erase_complete()) { };
}

static void prvWatchdogTask(void *pvParameters) {
  while (1) {
    vTaskDelay(1000);
    hardware_watchdog_feed();
  }
}

static void prvQueuePingTask(void *pvParameters) {
  while (1) {
    const uint32_t ulValueToSend = 100;
    xQueueSend(xQueue, &ulValueToSend, portMAX_DELAY);
  }
}

static void trigger_nvic_int0(void) {
  // Let's set the interrupt priority to be the
  // lowest possible for the NRF52. Note the default
  // NVIC priority is zero which would match our current pendsv
  // config so no pre-emption would take place if we didn't change this
  volatile uint32_t *nvic_ipr = (void *)0xE000E400;
  *nvic_ipr = 0xe0;

  // Enable the POWER_CLOCK_IRQ (External Interrupt 0)
  volatile uint32_t *nvic_iser = (void *)0xE000E100;
  *nvic_iser |= 0x1;

  // Pend an interrupt
  volatile uint32_t *nvic_ispr = (void *)0xE000E200;
  *nvic_ispr |= 0x1;

  // flush pipeline to ensure exception takes effect before we
  // return from this routine
  __asm("isb");
}

static void prvQueuePongTask(void *pvParameters) {
  while (1) {
    uint32_t ulReceivedValue = 0xff;

    if (xQueueReceive(xQueue, &ulReceivedValue, portMAX_DELAY) == pdFALSE) {
      continue;
    }

    if (g_watchdog_hang_config == 2) {
      erase_external_flash();
    } else if (g_watchdog_hang_config == 3) {
      uint32_t temp;
      read_temp_sensor(&temp);
    } else if (g_watchdog_hang_config == 4) {
      trigger_nvic_int0();
    }
  }
}

void vAssertCalled(const char *file, int line) {
  __asm("bkpt 3");
}

void baremetal_while_loop_no_feed(void) {
  while (1) {
  }
}

void baremetal_while_loop_with_feed(void) {
  while (1) {
    hardware_watchdog_feed();
  }
}

typedef enum {
  kWatchdogExampleTaskId_Ping = 0,
  kWatchdogExampleTaskId_Pong,
  kWatchdogExampleTaskId_Watchdog,
} eWatchdogExampleTaskId;

volatile uint32_t *const RESETREAS = (uint32_t *)0x40000400;
static void prv_check_and_reset_reboot_reason(void) {
  const uint32_t last_reboot_reason = *RESETREAS;
  // clear any enabled reset reasons
  *RESETREAS |= *RESETREAS;
  // Halt the system on boot-up if a
  // Watchdog Reset took place
  const uint32_t watchdog_reset_mask = 0x2;
  if ((last_reboot_reason & watchdog_reset_mask) == watchdog_reset_mask) {
    __asm("bkpt 10");
  }
}

int main(void) {
  prv_check_and_reset_reboot_reason();

  hardware_watchdog_enable();

  if (g_watchdog_hang_config == 0) {
    baremetal_while_loop_no_feed();
  } else if (g_watchdog_hang_config == 1) {
    baremetal_while_loop_with_feed();
  } else {
    // hang will be from an RTOS task
  }

  const uint32_t main_queue_length = 1;
  xQueue = xQueueCreate(main_queue_length, sizeof(uint32_t));
  configASSERT(xQueue != NULL);

  s_temp_i2c_mutex = xSemaphoreCreateMutex();
  configASSERT(s_temp_i2c_mutex != NULL);


  #define mainQUEUE_RECEIVE_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
  #define mainQUEUE_SEND_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
  #define mainQUEUE_WATCHDOG_TASK_PRIORITY (tskIDLE_PRIORITY)

  xTaskCreate(prvQueuePongTask, "Pong", configMINIMAL_STACK_SIZE,
              (void *)kWatchdogExampleTaskId_Pong,
              mainQUEUE_RECEIVE_TASK_PRIORITY,
              NULL);

  xTaskCreate(prvQueuePingTask, "Ping", configMINIMAL_STACK_SIZE,
              (void *)kWatchdogExampleTaskId_Ping,
              mainQUEUE_SEND_TASK_PRIORITY,
              NULL);

  xTaskCreate(prvWatchdogTask, "Watchdog", configMINIMAL_STACK_SIZE,
              (void *)kWatchdogExampleTaskId_Watchdog,
              mainQUEUE_WATCHDOG_TASK_PRIORITY,
              NULL);

  vTaskStartScheduler();

  // should be unreachable
  configASSERT(0);
  return -1;
}
