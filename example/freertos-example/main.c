#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

// the FreeRTOS heap
uint8_t ucHeap[configTOTAL_HEAP_SIZE];

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
    __asm("bkpt 1");
    xQueueSend(xQueue, &ulValueToSend, 0U);
  }
}

static void prvQueuePongTask(void *pvParameters) {
  while (1) {
    unsigned long ulReceivedValue;
    xQueueReceive(xQueue, &ulReceivedValue, portMAX_DELAY);

    if (ulReceivedValue == 100UL) {
      __asm("bkpt 2");
      ulReceivedValue = 0U;
    }
  }
}

void vAssertCalled(const char *file, int line) {
  __asm("bkpt 3");
}

int main(void) {
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
