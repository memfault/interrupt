# Introduction

A minimal Cortex-M startup environment targetting the DA14695 Development Kit – USB[^1] written entirely in C (inspired
by [this project](https://github.com/noahp/minimal-c-cortex-m)) written solely for illustrative
purposes about the Micro Trace Buffer (MTB)/

A bare bones FreeRTOS was added to the project.

The usefuleness of the MTB can be explored through several different compile/runtime options:

```c
//! Modes
//!  0 - System runs in a minimal while (1) loop
//!  1 - Trigger a stack overflow exception
//!  2 - Trigger an exception due to executing bogus instructions
//!  3 - System runs normally, no crashes
#ifndef TRACE_EXAMPLE_CONFIG
#define TRACE_EXAMPLE_CONFIG 0
#endif

// Make the variable global so the compiler doesn't optimize away the setting
// and the variable can be overriden from gdb without having to recompile
// the app.
//
// (gdb) break main
// (gdb) continue
// (gdb) set g_trace_example_config=1
// (gdb) continue
volatile int g_trace_example_config = TRACE_EXAMPLE_CONFIG;
```

# Compiling & Running

The toolchain used for this example can be found on [ARM's Developer Website](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

```bash
# Compile the code
$ make
Compiling main.c
//...
Linking library
Generated build/da1469x.elf

# In one terminal, start a GDB Server
$ JLinkGDBServer  -if swd -device DA14695 -nogui

SEGGER J-Link GDB Server V6.84a Command Line Version

# Flash the code on the DA14695 and start gdb
$ make flash
(gdb) continue
```
