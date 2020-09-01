#pragma once

#include <inttypes.h>
#include <stdbool.h>

// Don't ever re-use an ID!
typedef enum {
    kDeviceMetricId_INVALID = 0,
    kDeviceMetricId_ElapsedTime = 1,
    kDeviceMetricId_MainTaskTime = 2,
    kDeviceMetricId_TimerTaskTime = 3,
    kDeviceMetricId_TimerTaskCount = 4,
    kDeviceMetricId_SensorOnTime = 5,
    kDeviceMetricId_HeapHighWatermark = 6,
} eDeviceMetricId;

typedef uint32_t (*DeviceMetricsGetTicksCallback)(void);
typedef void (*DeviceMetricsClientCallback)(bool is_flushing);

// Initialization
void device_metrics_init(DeviceMetricsGetTicksCallback get_ticks,
                         DeviceMetricsClientCallback callback);

// Counters
void device_metrics_incr(eDeviceMetricId metric_id);
void device_metrics_incr_by(eDeviceMetricId metric_id, int32_t n);

// Counted Timers
void device_metrics_timer_start(uint32_t *start);
void device_metrics_timer_end(eDeviceMetricId metric_id, const uint32_t *tick_buf);
void device_metrics_timer_end_counted(eDeviceMetricId metric_id, const uint32_t *tick_buf, eDeviceMetricId counter_metric_id);

// Gauges
void device_metrics_set(eDeviceMetricId metric_id, int32_t value);

// Call this every hour
void device_metrics_flush(void);

// After flushing, reset all metrics!
void device_metrics_reset_all(void);

// For debugging 
typedef void (*DeviceMetricEachCallback)(eDeviceMetricId metric_id, int32_t value);
void device_metrics_each(DeviceMetricEachCallback callback);