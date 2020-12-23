#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "cmsis_shim.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "mtb.h"
// TODO

//! Modes
//!  0 - System runs in a minimal while (1) loop
//!  1 - Trigger a stack overflow exception
//!  2 - Trigger an exception due to executing bogus instructions
//!  3 - System runs normally, no crashes
#ifndef TRACE_EXAMPLE_CONFIG
#define TRACE_EXAMPLE_CONFIG 0
#endif

// Make the variable global so the compiler doesn't optimize away the setting
// and the variable can be overriden from gdb without having to recompile
// the app.
//
// (gdb) break main
// (gdb) continue
// (gdb) set g_trace_example_config=1
// (gdb) continue
volatile int g_trace_example_config = TRACE_EXAMPLE_CONFIG;

// the FreeRTOS heap
uint8_t ucHeap[configTOTAL_HEAP_SIZE];
static QueueHandle_t xQueue = NULL;

void ExternalInt0_Handler(void) {
  return;
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

static void prvQueuePingTask(void *pvParameters) {
  while (1) {
    const uint32_t ulValueToSend = 100;
    xQueueSend(xQueue, &ulValueToSend, portMAX_DELAY);
    vTaskDelay(50);
    trigger_nvic_int0();
  }
}

// for illustrative purposes, an assembly function that clobbers
// the lr and then jumps to a bogus address
__attribute__((naked))
static void bad_asm_function(void) {
  __asm("mov r0, 0 \n"
        "ldr r2, =3204505165 \n"
        "mov lr, 0 \n"
        "bx r2 \n"
        );
}

static void prvQueuePongTask(void *pvParameters) {
  while (1) {
    uint32_t ulReceivedValue = 0xff;

    if (xQueueReceive(xQueue, &ulReceivedValue, portMAX_DELAY) == pdFALSE) {
      continue;
    }

    if (g_trace_example_config == 2) {
      bad_asm_function();
    }

    vTaskDelay(50);
  }
}

void vAssertCalled(const char *file, int line) {
  __asm("bkpt 3");
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName) {
  __asm("bkpt 4");
}

void HardFault_Handler(void) {
  mtb_disable();
  __asm("bkpt 6");
}

__attribute__((optimize("O0")))
int recurse_func0(int val) {
  uint8_t computation_buf[val];
  memset(computation_buf, val & 0xff, sizeof(computation_buf));
  uint32_t total;
  for (size_t i = 0; i < val; i++) {
    total += computation_buf[i];
  }

  return total;
}

__attribute__((optimize("O0")))
int recurse_func1(int val) {
  return recurse_func0(val + 1);
}

__attribute__((optimize("O0")))
int recurse_func2(int val) {
  return recurse_func1(val + 1);
}

__attribute__((optimize("O0")))
int recurse_func3(int val) {
  return recurse_func2(val + 1);
}

__attribute__((optimize("O0")))
int recurse_func4(int val) {
  return recurse_func3(val + 1);
}

static void prvAlgoTask(void *pvParameters) {
  while (1) {
    const int val = g_trace_example_config == 1 ? 500 : 10;
    recurse_func4(val);

    vTaskDelay(50);
  }
}

__attribute__((optimize("O0")))
void infinite_loop(void) {
  volatile int i;
  while (1) {
    i++;

    if ((i % 5) == 0) {
      __asm("bkpt 6");
    }
  }
}

int main(void) {
  // DA1469X enables the watchdog by default. For the purposes of
  // this demo application, we freeze it so it doesn't get in the way
  volatile uint32_t *SET_FREEZE_REG = (uint32_t *)0x50040300;
  *SET_FREEZE_REG |= (1 << 10) | (1 << 3);


  mtb_enable(8192);

  if (g_trace_example_config == 0) {
    infinite_loop();
  }

  const uint32_t main_queue_length = 1;
  xQueue = xQueueCreate(main_queue_length, sizeof(uint32_t));
  configASSERT(xQueue != NULL);

  #define mainQUEUE_RECEIVE_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
  #define mainQUEUE_SEND_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
  #define mainQUEUE_WATCHDOG_TASK_PRIORITY (tskIDLE_PRIORITY)

  xTaskCreate(prvQueuePongTask, "Pong", configMINIMAL_STACK_SIZE,
              NULL,
              mainQUEUE_RECEIVE_TASK_PRIORITY,
              NULL);

  xTaskCreate(prvQueuePingTask, "Ping", configMINIMAL_STACK_SIZE,
              NULL,
              mainQUEUE_SEND_TASK_PRIORITY,
              NULL);

  xTaskCreate(prvAlgoTask, "Algo", configMINIMAL_STACK_SIZE,
              NULL,
              mainQUEUE_WATCHDOG_TASK_PRIORITY,
              NULL);

  vTaskStartScheduler();

  // should be unreachable
  configASSERT(0);
  return -1;
}
