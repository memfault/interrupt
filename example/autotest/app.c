#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <libopencm3/cm3/vector.h>
#include <libopencm3/stm32/usart.h>
#include <shell/shell.h>

#include "memfault/core/platform/debug_log.h"
#include "memfault/core/platform/device_info.h"
#include "memfault/core/compiler_gcc.h"
#include "memfault/core/reboot_tracking.h"

#include "clock.h"
#include "gpio.h"
#include "usart.h"

// I need to to this because I can't init the boot info properly
void memfault_reboot_tracking_clear_reset_info(void);

int main(void) {
    clock_setup();
    gpio_setup();
    usart_setup();

    memfault_reboot_tracking_boot((void *)0x20024000, NULL);
    memfault_reboot_tracking_clear_reset_info();

    printf("App STARTED\n");

    // Configure shell
    sShellImpl shell_impl = {
      .send_char = usart_putc,
    };
    shell_boot(&shell_impl);

    char c;
    while (1) {
        c = usart_getc();
        shell_receive_char(c);
    }

    return 0;
}


void memfault_platform_log(eMemfaultPlatformLogLevel level, const char *fmt, ...) {
  // Hook up to your logging implementation.
  // The Memfault SDK will call this API sparingly when issues are detected to aid in debug
}
void memfault_platform_get_device_info(sMemfaultDeviceInfo *info) {
  // platform specific version information
  *info = (sMemfaultDeviceInfo) {
    .device_serial = "DEMOSERIAL",
    .software_type = "nrf-main",
    .software_version = "1.0.0",
    .hardware_version = "nrf-proto",
  };
}


MEMFAULT_NORETURN void memfault_platform_reboot(void) {
  __asm("bkpt 1");
  while (1) { }
}
