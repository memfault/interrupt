#include "metrics.h"

int main(int argc, char const *argv[])
{
    device_metrics_init();
    device_metrics_incr(kDeviceMetricId_HeapHighWaterMark);
    device_metrics_print_all();
    return 0;
}
