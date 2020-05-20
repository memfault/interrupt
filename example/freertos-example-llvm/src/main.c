#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include <malloc.h>
#include <stdbool.h>
#include <unistd.h>

#include "example_project/examples.h"
#include "example_project/compiler.h"


// the FreeRTOS heap
extern uint8_t ucHeap[];
uint8_t ucHeap[configTOTAL_HEAP_SIZE];


static uint8_t g_newlib_heap[2048];

//! A very naive implementation of the newlib _sbrk dependency function
caddr_t _sbrk(int incr);
caddr_t _sbrk(int incr) {
  static uint32_t s_index = 0;

  // Out of memory if this check fails!
  configASSERT((s_index + (uint32_t)incr) <= sizeof(g_newlib_heap));

  caddr_t result = (caddr_t)&g_newlib_heap[s_index];
  s_index += (uint32_t)incr;
  return result;
}

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define mainQUEUE_SEND_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

/* The rate at which data is sent to the queue.  The 200ms value is converted
to ticks using the portTICK_RATE_MS constant. */
#define mainQUEUE_SEND_FREQUENCY_MS (1000 / portTICK_PERIOD_MS)

#define mainQUEUE_LENGTH (1)

#define mainQUEUE_SEND_PARAMETER (0x1111UL)
#define mainQUEUE_RECEIVE_PARAMETER (0x22UL)

static QueueHandle_t xQueue = NULL;

static void prvQueuePingTask(void *pvParameters) {
  TickType_t xNextWakeTime;
  const unsigned long ulValueToSend = 100UL;

  configASSERT(((unsigned long)pvParameters) == mainQUEUE_SEND_PARAMETER);

  xNextWakeTime = xTaskGetTickCount();

  while (1) {
    vTaskDelayUntil(&xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS);
    xQueueSend(xQueue, &ulValueToSend, 0U);
  }
}

static void prvQueuePongTask(void *pvParameters) {
  while (1) {
    unsigned long ulReceivedValue;
    xQueueReceive(xQueue, &ulReceivedValue, portMAX_DELAY);

    if (ulReceivedValue == 100UL) {
      ulReceivedValue = 0U;
      example_project_run_memory_leak_examples();
    }
  }
}

void vAssertCalled(const char *file, int line) {
  __asm("bkpt 3");
  while (1) { }
}
#if 0
typedef int __attribute__((capability("role"))) MyLockResource;
MyLockResource g_lock;
void my_lock()  __attribute((acquire_capability(g_lock)));
void my_unlock()  __attribute((release_capability(g_lock))) ;
void example_1(void) {
  my_lock();
  void *leak = my_malloc();
  if (leak) {
    //my_unlock(MyLock);
    return;
  }
  my_unlock();
}
#endif

//! We disable optimizations here to force the compiler to emit
//! a call __aeabi_uldivmod.
//!
//! This means the project will need to include libgcc.a or libclang_rt.builtins*
//! to actually link
EXAMPLE_PROJ_NO_OPT
uint64_t force_libgcc_builtin_dependency(uint64_t a, uint64_t b) {
  return a / b;
}

int main(void) {
  force_libgcc_builtin_dependency(1, 2);
  example_operate_on_pointer(NULL);
  example_run_mutex_examples();
  example_divide_by_zero(2);


  xQueue = xQueueCreate(mainQUEUE_LENGTH, sizeof(unsigned long));
  configASSERT(xQueue != NULL);

  xTaskCreate(prvQueuePongTask,
              "Pong", configMINIMAL_STACK_SIZE,
              (void *) mainQUEUE_RECEIVE_PARAMETER,
              mainQUEUE_RECEIVE_TASK_PRIORITY,
              NULL);

  xTaskCreate(prvQueuePingTask, "Ping", configMINIMAL_STACK_SIZE,
              (void *)mainQUEUE_SEND_PARAMETER, mainQUEUE_SEND_TASK_PRIORITY,
              NULL);
  vTaskStartScheduler();

  // should be unreachable
  configASSERT(0);
  return -1;
}
