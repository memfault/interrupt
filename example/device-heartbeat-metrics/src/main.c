#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "timers.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "hal/assert.h"
#include "hal/device_info.h"
#include "hal/logging.h"
#include "hal/uart.h"
#include "metrics.h"

// Hide FreeRTOS initialization which isn't
// relevant to example code
#include "freertos.def.c"

static void prv_metrics_each(eDeviceMetricId metric_id, int32_t value) {
  EXAMPLE_LOG_INFO("Metric ID: %d -- Value: %"PRIu32, metric_id, value);
}

static void prv_metrics_flush(TimerHandle_t handle) {
  device_metrics_flush();
}

static void prv_work(int n) {
  // Fudge this number a little
  int add = rand() % 1000;
  n += add;

  vTaskDelay(n);
}

// A timer that is tracked how long and how many times it runs for
static void prv_busy_timer_callback(TimerHandle_t handle) {
  // Record start time
  uint32_t tick_count;
  device_metrics_timer_start(&tick_count);

  prv_work(5);

  // Record duration and increment counter
  device_metrics_timer_end_counted(
      kDeviceMetricId_TimerTaskTime, 
      &tick_count, 
      kDeviceMetricId_TimerTaskCount
  );
}

static void prv_main_task(void *ctx) {
  uint32_t task_tick_count;
  uint32_t sensor_tick_count;

  while (1) {
    // Record start time
    device_metrics_timer_start(&task_tick_count);

    // Execute something
    prv_work(100);

    // Maybe turn a sensor on
    device_metrics_timer_start(&sensor_tick_count);
    prv_work(10);
    device_metrics_timer_end(kDeviceMetricId_SensorOnTime, &sensor_tick_count);

    // Record duration
    device_metrics_timer_end(kDeviceMetricId_MainTaskTime, &task_tick_count);

    vTaskDelay(1000);
  }
}

static void prv_device_metrics_flush_callback(bool is_flushing) {
  static uint32_t s_tick_count;

  if (is_flushing) {
    // Get high water mark heap
    device_metrics_set(kDeviceMetricId_HeapHighWatermark, xPortGetMinimumEverFreeHeapSize());
    device_metrics_timer_end(kDeviceMetricId_ElapsedTime, &s_tick_count);

    // Debug print
    device_metrics_each(prv_metrics_each);
  } else {
    device_metrics_timer_start(&s_tick_count);
  }
}

static uint32_t prv_get_ticks(void) {
  return xTaskGetTickCount();
}

void main_task_boot(void) {
  xTaskCreate(prv_main_task, "Main", 1024,
              (void *)0, (tskIDLE_PRIORITY + 1) | portPRIVILEGE_BIT, NULL);
}

int main(void) {
  vPortEnableVFP();
  uart_boot();

  device_metrics_init(prv_get_ticks, 
                      prv_device_metrics_flush_callback);

  EXAMPLE_LOG_INFO("Example App Booting");

  main_task_boot();

  TimerHandle_t timer = xTimerCreate("timer1Sec",
                                     1000, /* period/time */
                                     pdTRUE, /* auto reload */
                                     (void*)0,
                                     prv_busy_timer_callback);

  xTimerStart(timer, 0);

  TimerHandle_t metrics_flush_timer = 
      xTimerCreate("timer2Sec",
                   15000, /* period/time */
                   pdTRUE, /* auto reload */
                   (void*)0,
                   prv_metrics_flush);

  xTimerStart(metrics_flush_timer, 0);

  vTaskStartScheduler();

  // should be unreachable
  configASSERT(0);
  return -1;
}
