# Introduction

A minimal Cortex-M startup enviornment targetting the NRF52840-DK written entirely in C (inspired
by [this project](https://github.com/noahp/minimal-c-cortex-m)) written solely for illustrative purposes

There are only two files:

- startup.c
- main.c

A bare bones `GCC/ARM_CM4F` of FreeRTOS was added to the project.

On boot, a "Ping" **FreeRTOS** task and a "Pong" **FreeRTOS** task are created. The "Ping" task sends a
message to the "Pong" task once per second and each time the event loop in a task runs, I've added
a breakpoint instruction so we can confirm with the debugger the tasks are switching between each
other.

# Compiling & Running

```bash
# Compile the code
$ make
Compiling main.c
Compiling startup.c
Compiling freertos_kernel/tasks.c
Compiling freertos_kernel/queue.c
Compiling freertos_kernel/list.c
Compiling freertos_kernel/timers.c
Compiling freertos_kernel/portable/GCC/ARM_CM4F/port.c
Compiling freertos_kernel/portable/MemMang/heap_1.c
Linking library
Generated build/nrf52.elf

# In one terminal, start a GDB Server
$ JLinkGDBServer  -if swd -device nRF52840_xxAA
SEGGER J-Link GDB Server V6.52a Command Line Version

# Flash the code on the NRF52 and start gdb
$ arm-none-eabi-gdb-py --eval-command="target remote localhost:2331" --ex="mon reset" --ex="load"
--ex="mon reset" --se=build/nrf52.elf
GNU gdb (GNU Tools for Arm Embedded Processors 8-2019-q3-update) 8.3.0.20190703-git
Copyright (C) 2019 Free Software Foundation, Inc.
[...]
Resetting target
Loading section .interrupts, size 0x40 lma 0x0
Loading section .text, size 0x194d lma 0x40
Loading section .data, size 0x4 lma 0x1990
Start address 0x40, load size 6545
Transfer rate: 2130 KB/sec, 2181 bytes/write.
Resetting target
(gdb)
```
