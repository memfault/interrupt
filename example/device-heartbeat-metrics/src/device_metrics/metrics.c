#include "metrics.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#define TRAP  do {} while (1)

#define NUM_METRICS 6
// Keep this array so we can deprecate old metrics, keep 
// the old ID's stable, and not have gaps in an array
static const int s_metric_definitions[] = {
    kDeviceMetricId_ElapsedTime,
    kDeviceMetricId_MainTaskTime,
    kDeviceMetricId_TimerTaskTime,
    kDeviceMetricId_TimerTaskCount,
    kDeviceMetricId_SensorOnTime,
    kDeviceMetricId_HeapHighWatermark,
};
_Static_assert(ARRAY_SIZE(s_metric_definitions) == NUM_METRICS, 
               "Should be the same size!");

static int32_t s_metric_values[NUM_METRICS];
static DeviceMetricsGetTicksCallback s_tick_callback;
static DeviceMetricsClientCallback s_metric_callback;

static uint32_t prv_get_definition_index(eDeviceMetricId metric_id) {
    for (uint32_t i = 0; i < NUM_METRICS; i++) {
        if (s_metric_definitions[i] == metric_id) {
            return i;
        }
    }
    // Should never get here.
    TRAP;
}

static int32_t *prv_get_value_ptr(eDeviceMetricId metric_id) {
    uint32_t idx = prv_get_definition_index(metric_id);
    return &s_metric_values[idx];
}

void device_metrics_init(DeviceMetricsGetTicksCallback get_ticks, 
                         DeviceMetricsClientCallback callback) {
    s_tick_callback = get_ticks;
    s_metric_callback = callback;
    device_metrics_reset_all();
}

void device_metrics_incr_by(eDeviceMetricId metric_id, int32_t n) {
    int32_t *val = prv_get_value_ptr(metric_id);
    *val += n;
}

void device_metrics_set(eDeviceMetricId metric_id, int32_t value) {
    int32_t *val = prv_get_value_ptr(metric_id);
    *val = value;
}

void device_metrics_incr(eDeviceMetricId metric_id) {
    device_metrics_incr_by(metric_id, 1);
}

void device_metrics_timer_start(uint32_t *tick_buf) {
    *tick_buf = s_tick_callback();
}

void device_metrics_timer_end_counted(eDeviceMetricId metric_id, const uint32_t *tick_buf, eDeviceMetricId counter_metric_id) {
    uint32_t end_tick_count = s_tick_callback();
    uint32_t total_ticks = end_tick_count - *tick_buf;
    device_metrics_incr_by(metric_id, total_ticks);

    if (counter_metric_id) {
        device_metrics_incr(counter_metric_id);
    }
}

void device_metrics_timer_end(eDeviceMetricId metric_id, const uint32_t *tick_buf) {
    device_metrics_timer_end_counted(metric_id, tick_buf, 0);
}

static void prv_call_client_handler(bool is_flushing) {
    if (s_metric_callback) {
        s_metric_callback(is_flushing);
    }
}

void device_metrics_flush(void) {
    prv_call_client_handler(true /* is_flushing */);
    device_metrics_reset_all();
    prv_call_client_handler(false /* is_flushing */);
}

void device_metrics_reset_all(void) {
    memset(s_metric_values, 0, sizeof(s_metric_values));
    prv_call_client_handler(false /* is_flushing */);
}

void device_metrics_each(DeviceMetricEachCallback callback) {
    for (uint32_t i = 0; i < NUM_METRICS; i++) {
        eDeviceMetricId metric_id = s_metric_definitions[i];
        int32_t *val_ptr = prv_get_value_ptr(metric_id);
        callback(metric_id, *val_ptr);
    }
}