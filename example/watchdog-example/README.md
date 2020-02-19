# Introduction

A minimal Cortex-M startup environment targetting the NRF52840-DK written entirely in C (inspired
by [this project](https://github.com/noahp/minimal-c-cortex-m)) written solely for illustrative
purposes about Hardware Watchdogs

A bare bones `GCC/ARM_CM4F` of FreeRTOS was added to the project.

On boot, a "Ping" **FreeRTOS** task and a "Pong" **FreeRTOS** task are created. The "Ping" task sends a
message to the "Pong" task and each time the event loop in a task runs. While the program is
running different types of hangs can be triggered by changing the value of `g_watchdog_hang_config`:

```c
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
```

# Apply Patches Added During Article

```bash
$ git apply 01-patch-task-watchdog.patch
$ git apply 02-patch-software-watchdog.patch
```

# Compiling & Running

The toolchain used for this example can be found on [ARM's Developer Website](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

```bash
# Compile the code
$ make
Compiling main.c
Compiling startup.c
Compiling mock_external_ics.c
Compiling freertos_kernel/tasks.c
Compiling freertos_kernel/queue.c
Compiling freertos_kernel/list.c
Compiling freertos_kernel/timers.c
Compiling freertos_kernel/portable/GCC/ARM_CM4F/port.c
Compiling freertos_kernel/portable/MemMang/heap_1.c
Compiling hardware_watchdog.c
Compiling task_watchdog.c
Compiling software_watchdog.c
Linking library
Generated build/nrf52.elf

# In one terminal, start a GDB Server
$ JLinkGDBServer  -if swd -device nRF52840_xxAA

SEGGER J-Link GDB Server V6.52a Command Line Version

# Flash the code on the NRF52 and start gdb
$ arm-none-eabi-gdb-py --eval-command="target remote localhost:2331" --ex="mon reset" --ex="load" \
    --ex="mon reset" --ex="source gdb_resume_handlers.py" --se=build/nrf52.elf

GNU gdb (GNU Tools for Arm Embedded Processors 9-2019-q4-major) 8.3.0.20190709-git
Copyright (C) 2019 Free Software Foundation, Inc.
[...]
Resetting target
Loading section .interrupts, size 0x40 lma 0x0
Loading section .text, size 0x194d lma 0x40
Loading section .data, size 0x4 lma 0x1990
Start address 0x40, load size 6545
Transfer rate: 2130 KB/sec, 2181 bytes/write.
Resetting target
(gdb) continue
```
