#pragma once

#ifndef HARDWARE_WATCHDOG_TIMEOUT_SECS
#define HARDWARE_WATCHDOG_TIMEOUT_SECS 10
#endif

void hardware_watchdog_enable(void);
void hardware_watchdog_feed(void);
