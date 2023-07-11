---
title: "Using Asserts in Embedded Systems"
description:
  "Asserts are a quick and proven way to prevent bugs in embedded systems. An
  optimal assert implementation maximizes the context around the assert with
  minimal code and RAM overhead."
author: tyler
image: /img/debugging-asserts/assert.png
tags: [debugging, better-firmware]
---

The use of asserts is one of the best ways to find bugs, unintended behavior,
programmatic errors, and to catch when systems are no longer 100% functional and
need to be reset to recover. If instrumented correctly, an assert can give a
developer context about when and where in the code an issue took place. Despite
the numerous benefits, the practice of using asserts in firmware is not common
or agreed upon.

<!-- excerpt start -->

By using asserts proactively in embedded systems on debug **and production**
builds, developers can both prevent more bugs before shipping and quickly
surface and fix them after shipping. Proper assert handling is also the safest
way to handle issues and undefined behavior that occur in production. In this
post, we'll go over best practices with asserts, when to use asserts, and then
come up with a production ready custom assert implementation for an ARM Cortex-M
device, all while keeping the code size usage to a minimum.

<!-- excerpt end -->

> **Why you shouldn't compile asserts out in production builds**
> Many of the popular embedded platforms have options to
> [compile](https://github.com/aws/amazon-freertos/blob/master/vendors/nordic/nRF5_SDK_15.2.0/components/libraries/util/nrf_assert.h#L78-L115)
> [out](https://github.com/espressif/esp-idf/blob/master/components/esp_common/include/esp_err.h#L104-L124)
> [error](https://github.com/ARMmbed/mbed-os/blob/master/platform/mbed_error.h#L191-L202)
> [handling](https://github.com/zephyrproject-rtos/zephyr/blob/master/include/sys/__assert.h#L34-L89)
> and assertions. In our opinion, this is a mistake. In the event of
> inconsistent behavior or memory corruption, crashing the device is often the
> safest thing you could do. Embedded systems can often reboot quickly, getting
> the system back to a known good state. Over time, it will also be noticed and
> provide valuable feedback back to your engineering teams. The alternative is
> worse: the system could behave in unpredictable way and perform poorly, lose
> customer information, …etc.

{% include toc.html %}

### Article Disclaimers

- This post is about runtime asserting, not compile time asserting, such as
  [`static_assert`](https://en.wikichip.org/wiki/c/assert.h/static_assert).
- This post is targeting the ever-growing number of **non-safety-critical**
  embedded systems, such as consumer electronics, Class 1 medical devices, home
  automation devices, etc.
- The above types of embedded systems are primarily constrained by memory and
  code space limitations, so the information below takes this into account.
- The examples below are implemented for the ARM Cortex-M series of MCUs. Many
  of the concepts can be generalized to other architectures.

{% include newsletter.html %}

## Making the Most of Asserts

To get the most value from asserting, it is important to set up some postmortem
infrastructure. When an assert takes place (or even a system fault), there is _a
lot_ of context and data one can capture to help debug these issues. These
include but are not limited to:

- the file and line number the assert took place at
- the registers at the time of assert
- values or function pointers on the stack, to help reassemble the backtrace.
- the logs that took place before the assert and possibly after the reboot took
  place
- for those using MyNewt[^1], ESP-IDF[^2], or have Memfault[^3] integrated, a
  coredump can be captured at the time of the assert, which will provide a full
  dump of system memory and registers, allowing one to clearly see why and how
  the system triggered an assert

With all of the above, especially coredumps, it becomes advantageous to add as
many asserts to the system as possible, especially for capturing those 1 in
10,000 hour bugs. This allows developers to quickly track down and investigate
bugs affecting users in the field, all without having to reproduce the issue on
a development board.

## Why You Should Assert

- Asserts generally occur while the system is still in control (e.g. not in a
  HardFault, etc). This means that the system can safely perform cleanup
  operations and record debugging information.
- Developers connected to a debugger very quickly find programmatic mistakes and
  invalid usage of API's.
- Asserting function arguments removes the need to check for argument validity
  and error handling code in upper layers, which may in turn reduce code size.
- Good placement of asserts can reduce chances of exceptions by catching
  out-of-bounds accesses, invalid pointer dereferences, invalid state machine
  transitions, and nonsensical operations (like a malloc of 12MB on a 64kB MCU).
- Asserts can help developers quickly understand assumptions made by a certain
  piece of code (e.g. a module that takes a `uint8_t` but requires a value
  within the range of 1 to 64).

## Common Usages of Asserts

Many assertions are application-specific, but we've got a few rules of thumbs on
when you should use assertions. Below are some examples of when using assertions
is a good idea.

### When malloc shouldn't fail

Dynamic memory allocation is common in embedded systems, especially in today's
Internet connected consumer electronics. These devices need to be able to
receive an influx of packets, use large HTTP buffers, and possibly load assets
or fonts into RAM for a user interface. Keeping around statically allocated
buffers for all of these functions would be wasteful, especially for the devices
that measure RAM in kilobytes. In these devices, dynamic memory allocation is a
must.

There are also places where malloc could fail, and where **it shouldn't**.

For example, let's give an example of dynamically allocated mutexes where malloc
should not fail.

```c
Mutex *mutex_create(void) {
  Mutex *mutex = malloc(sizeof(Mutex));
  ASSERT(mutex);
  mutex_init(mutex);

  return mutex;
}
```

This malloc **should never fail**. If it did, then it would mean the system is
out of heap space. If we didn't assert here, then the system would risk using an
unprotected resource.

On the contrary, there are cases where it is perfectly fine for malloc to fail,
such as when a surge of packets are received and the heap becomes exhausted. The
system should process the packets, free the memory, and then ask for more data.

### Invalid arguments passed into function

Argument checking is one of the best (and easiest) places to assert.

We can check for NULL pointers

```c
void handle_event(Event *evt) {
  ASSERT(evt);
  ...
}
```

or check sizes of buffers

```c
void read_packet(void *buf, size_t buf_len) {
  ASSERT(buf_len >= sizeof(Packet));
  memcpy(...)
}

```

or prevent silly operations.

```c
void *my_malloc(uint32_t n) {
  ASSERT(n < 131072);
  ...
}
```

All of the above examples above are **bugs** introduced by a developer and they
likely shouldn't be handled gracefully. The alternative is to return an error
code, and burden the developer with finding out _exactly_ where in the call
stack a particular argument check filed.

> It's not common to see asserts on arguments in vendor and library code, since
> they aren't in the business of crashing devices, but it is sometimes
> configurable on mature software. An example of configurable asserting in a
> library can be found in FreeRTOS, e.g. `xTaskCreateStatic`[^4]. In this
> function, `configASSERT` is used to check for non-null pointers, and if
> `configASSERT` is configured to a no-op, the code will still check those
> values later in the function and return an error if they are invalid.

### When undefined behavior is imminent

```c
typedef enum {
  EventType_A,
  EventType_B,
} EventType;

void handle_event(Event *evt) {
  ...
  switch (evt->type) {
    case EventType_A:
      // something
      break;
    case EventType_B:
      // something
      break;
    default:
      // shouldn't ever get here.
      // something is corrupted
      ASSERT(0);
  }
}
```

If we want to make this even more understandable for future developers who come
across this, something like the following can also be done.

```c
#define WTF ASSERT(0)


void handle_event(Event *evt) {
  ...
  switch (evt->type) {
    ...
    default:
      WTF;
  }
}
```

This makes it pretty obvious that anything that falls through to the `default:`
case is undefined behavior.

### RTOS API functions fail

Another great place to assert is when you expect certain system functions to
succeed, and if they don't, the system is in a bad state. Below is a snippet
where we assert if a mutex wasn't locked after a few seconds, which likely means
there is a deadlock somewhere else.

```c
int rv = xSemaphoreTake(s_mutex, sec_to_ticks(5));
ASSERT(rv == pdTRUE);
```

> WARNING: Do not place code which performs operations within an `ASSERT`, such
> as `ASSERT(pdTRUE == xSemaphoreTake(s_mutex, 0))`. Aside from it being bad
> practice, if a developer _were_ to disable asserts, this would cause undefined
> behavior.

## When Not to Assert

There are places where it is advised to assert. On the contrary, there are
places where it's critical to **not** assert.

- Avoid asserting on boot up sequences. This is the most common way to introduce
  reboot loops.
- Don't assert on operations that depend on the hardware behaving appropriately.
  If a sensor says it will return a value between 0-100, it's probably best not
  assert when it's above 100, because you can never trust today's cheap
  hardware.
- Don't assert on the contents of data read from persistent storage, unless it's
  guaranteed to be valid. The data read from flash or a filesystem could be
  corrupted.
- If it is very likely the system will recover in a few moments, it might be
  best to not assert. e.g. when running out of heap or byte pool allocations
  during a spike in network packets.

## Custom Assert Implementation Examples

It's now time to start asserting ourselves. The examples provided below progress
from a naive solution to the best one I've seen and used to date, with each
becoming more expressive while also using less (precious) code space.

> NOTE: All of these implementations will behave similarly if a debugger is
> attached. The goal of the exercise is to improve upon asserts that are hit
> while a debugger is **not** attached and all we have is a logging system.

The code starts out in `prv_main`, and will call one of two functions,
`prv_main_path_A`, or `prv_main_path_B`, depending on a randomly generated
boolean variable. Both of these functions will then call either `assert_path_A`
or `assert_path_B`, which gives us **four** unique paths our code can take on
boot.

The example is a bit contrived, but if we had to do a postmortem analysis of
this assert, we'd like to know the path the system took before the assert.
Thankfully, we can build an assert implementation that does just this.

### Setup

For this setup we will use:

- a nRF52840-DK[^5] (ARM Cortex-M4F) as our development board
- SEGGER JLinkGDBServer[^6] as our GDB Server.
- GCC 8.3.1 / GNU Arm Embedded Toolchain as our compiler[^7]
- GNU make as our build system

All the code can be found on the
[Interrupt Github page](https://github.com/memfault/interrupt/tree/master/example/debugging-asserts)
with more details in the `README` in the directory linked.

#### Compiling the code and launching it with GDB

```
# Compile the first assert implementation
$ IMPL=ASSERT1 make
Compiling main.c
Compiling startup.c
Compiling freertos_kernel/tasks.c
Compiling freertos_kernel/queue.c
Compiling freertos_kernel/list.c
Compiling freertos_kernel/timers.c
Compiling freertos_kernel/portable/GCC/ARM_CM4F/port.c
Compiling freertos_kernel/portable/MemMang/heap_1.c
Compiling impls/assert1.c
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

#### Assert metadata structure

To help with the usability of our implementation, both while connected to a
debugger and when logging to persistent storage, we'll define a structure to
store information about the assert.

```c
// Convenience structure that we can store items in
//  to print out later when we add logging.
typedef struct sAssertInfo {
  uint32_t pc;
  uint32_t lr;
  uint32_t line;
  // I don't suggest actually using these, but they
  // are included for the examples.
  char file[256];
  char msg[256];
} sAssertInfo;

extern sAssertInfo g_assert_info;
```

This provides fields for the ARM program counter and link register, the file and
line number, and the message if any provided to the assert. In our example
assert implementations, we'll fill in the fields we can access.

### Standard Library Assert (#1)

The initial approach is to use what already exists within the standard library,
`assert(expr)`. This requires us to implement the function `__assert_func`.

```c

void __assert_func(const char *file,
                   int line,
                   const char *func,
                   const char *failedexpr) {
  snprintf(g_assert_info.msg, sizeof(g_assert_info.msg),
           "ASSERT: %s at %s\n", failedexpr, func);
  strncpy(g_assert_info.file, file, sizeof(g_assert_info.file));
  g_assert_info.line = line;

  __asm("bkpt 1");
}

void assert_path_A(void) {
  assert(0);
}
```

With the `stdlib.h` implementation of `assert.h`, the filepath, line number,
function name, and the actual expression being tested are **all** passed into
the assert handler.

If code space is of any concern to you, I would advise never to use this
approach.

### Error Message Assert (#2)

The next simplest approach is to pass in a unique string to the assert which
provides context for the developer about what the error was and where. We can
then search for this error message within the codebase and figure out at which
location the assert took place at.

```c
#define MY_ASSERT(expr, msg) \
  do {                                          \
    if (!(expr)) {                              \
      my_assert(msg);                           \
    }                                           \
  } while (0)
```

```c
void my_assert(const char *msg) {
  strncpy(g_assert_info.msg, msg, sizeof(g_assert_info.msg));
  __asm("bkpt 2");
}

void assert_path_A(void) {
  MY_ASSERT(0, "Assert in `assert2.c::assert_path_A`");
}
```

The drawback of this method is that every message is stored in the resulting
binary, and this takes up precious code space.

Below, we use `objdump` to view the resulting strings in the binary. We can see
that each assert string takes up ~32 bytes.

```
$ arm-none-eabi-objdump -s -j '.text' build/nrf52.elf
 ...
 2710 63000000 41737365 72742069 6e206061  c...Assert in `a
 2720 73736572 74322e63 3a3a7477 6f5f7472  ssert2.c::two_tr
 2730 75650000 41737365 72742069 6e206061  ue..Assert in `a
 2740 73736572 74322e63 3a3a7477 6f5f6661  ssert2.c::two_fa
 2750 6c736500                             lse.
```

The longer the string, the more codespace that is used, so this would actually
**discourage** the use of asserts, which is not ideal.

#### Extracted Information

We can look up the file and line number of the assert hit assuming all assert
strings are unique.

### File**path** and Line Number Assert (#3)

The most common approach I've seen for asserts is to include the filepath and
line number into the assert macro, using the `__FILE__` and `__LINE__` defines
provided by the compiler.

```c
#define MY_ASSERT(expr) \
  do {                                          \
    if (!(expr)) {                              \
      my_assert(__FILE__, __LINE__);            \
    }                                           \
  } while (0)
```

```c
void my_assert(const char *file, uint32_t line) {
  strncpy(g_assert_info.file, file, sizeof(g_assert_info.file));
  g_assert_info.line = line;
  __asm("bkpt 3");
}

void assert_path_A(void) {
  MY_ASSERT(0);
}
```

This approach works very well and gives the developer exactly the amount of
context required to find the issue. However, the `__FILE__` macro includes the
**entire** filepath of the file, and those can get quite lengthy. In my local
build, `__FILE__` is 77 bytes!

```
(gdb) p __FILE__
$1 = "/Users/tyler/dev/memfault/interrupt/example/debugging-asserts/impls/assert3.c"
```

Thankfully, the `__FILE__` string table entry is reused between asserts in the
same file, so the code size bloat primarily depends on the number of files with
asserts within them. However, most projects have a non-trivial number of files.

As a rough estimate, if we have asserts in 50 different files, this results in
**~50 bytes per filepath \* 50 files => 2500 bytes**, which is an enormous
amount of bloat just for storing filepaths!

For this reason, many RTOS implementations that include an `ASSERT` macro can
optionally leave these strings out of the build, such as Zephyr[^8] and
MyNewt[^9].

> NOTE: One simple code size (and sanity) optimization is to normalize these
> `__FILE__` paths to the root of the project directory. This will result in a
> build path that removes the `/Users/tyler/dev/memfault/...` portion. An added
> benefit of this is that all all machines should produce builds with the same
> filepaths, resulting in more similarly sized builds.

#### Extracted Information

We know the file and line number of the assert hit.

### File**name** and Line Number Assert (#4)

Since the filepaths can be lengthy and waste code space, one solution is to use
only the filename itself, e.g. `assert3.c`, instead of the filepath, e.g.
`/Users/tyler/.../assert3.c`.

This can be accomplished by adding a define called `__FILENAME__` and using this
in the assert implementation instead. In the Makefile or similar build system,
this macro is defined in line with the compilation rule.

```
$(BUILD_DIR)/%.o: $(ROOT_DIR)/%.c | $(BUILD_DIR) $(DEP_DIR) $(FREERTOS_PORT_ROOT)
  ...
  $(Q) arm-none-eabi-gcc $(DEP_CFLAGS) $(CFLAGS) $(INCLUDE_PATHS) \
      -D__FILENAME__=\"$(notdir $<)\" \
      -c -o $@ $<
```

In the assert header, we can now use `__FILENAME__` instead of `__FILE__`.

```c
#define MY_ASSERT(expr)                   \
  do {                                    \
    if (!(expr)) {                        \
      my_assert(__FILENAME__, __LINE__);  \
    }                                     \
  } while (0)
```

With this change, we reduce our file string from
`/Users/tyler/dev/memfault/interrupt/example/debugging-asserts/impls/assert3.c`
to `assert3.c`, resulting in 68 bytes of code space savings per file which
contains an assert.

```
$ arm-none-eabi-objdump -s -j '.text' build/nrf52.elf
 ...
26b0 61737365 7274342e 6300               assert4.c
```

#### Extracted Information

We know the file and line number of the assert hit.

### Register Values Only Assert (#5)

The problem with all of the above assert implementations is that our code size
is bloated due to storing string filenames, and also that our code size can
change depending on how we name the files in our project. This encourages poor
naming practices, as well as produces unexpected build sizes.

My favorite way to implement asserts is to record both the program counter and
the link register within the macro. This provides the following benefits:

- Each assert consumes a fixed number of bytes, no matter the filepath,
  filename, or number of asserts per file.
- By capturing both the PC and LR, we can get an extra frame in our 'backtrace'.
- If we really wanted to micro-optimize, we can choose not to record the LR and
  save a couple more bytes.

Below we define two macros, `GET_LR()` and `GET_PC()`, to retrieve the registers
`pc` and `lr` respectively.

```c
#define GET_LR() __builtin_return_address(0)
// This is ARM and GCC specific syntax
#define GET_PC(_a) __asm volatile ("mov %0, pc" : "=r" (_a))

#define MY_ASSERT_RECORD()     \
  do {                         \
    void *pc;                  \
    GET_PC(pc);                \
    const void *lr = GET_LR(); \
    my_assert(pc, lr);         \
  } while (0)

#define MY_ASSERT(exp)         \
  do {                         \
    if (!(exp)) {              \
      MY_ASSERT_RECORD();      \
    }                          \
  } while (0)
```

In our `my_assert` implementation, we then record the two register values in our
struct. Since we aren't storing the file or line number information directly in
our firmware, we can also save RAM and flash space. This could allow us to store
many more asserts in our logs!

```c
void my_assert(uint32_t *pc, uint32_t *lr) {
  // File and line deliberately left empty
  g_assert_info.pc = (uint32_t)pc;
  g_assert_info.lr = (uint32_t)lr;
  __asm("bkpt 5");
}
```

The drawback to this approach is that we'll have to use `addr2line` or a similar
tool to go from our register values to the file and line information, but this
can be added to a simple script and used by the entire team with ease.

```
$ arm-elf-linux-addr2line --exe build/nrf52.elf 0x2250
/Users/tyler/dev/memfault/interrupt/example/debugging-asserts/impls/assert5.c:13
```

> NOTE: Compiler optimizations such as function inlining may result in different
> asserts being merged together, which will produce confusing results. One way
> to mitigate this is to also pass in the `__LINE__` information to the assert
> to make every `my_assert` call unique.

#### Extracted Information

We can use `addr2line` on both the PC and LR to find the file and line number of
the assert hit, as well as **caller's** file and line number.

### Code Size Differences

Why did we compile in FreeRTOS to our examples? Because we can now check out the
differences in code sizes between our different assert implementations by
hooking up our `MY_ASSERT` macro to the FreeRTOS `configASSERT` macro.

Below are the results for compiling with `-O0` or `-Os` while only keeping in
the FreeRTOS asserts (removing those from `assert_path_A` and `assert_path_B`)

| Assert Impl #                              | `-Os` CFLAG (B) | Delta  |
| ------------------------------------------ | --------------- | ------ |
| None                                       | 17711           |        |
| [#1](#standard-library-assert-1) (Default) | 43477           | +25766 |
| [#3](#filepath-and-line-number-assert-3)   | 19404           | +1693  |
| [#4](#filename-and-line-number-assert-4)   | 18845           | +1134  |
| [#5](#register-values-only-assert-5)       | 18519           | +808   |

As one can see, there are significant code size wins over the standard
`__FILE__` assert implementations used by the common embedded platforms by using
the `__FILENAME__` approach, or by capturing the PC/LR registers on assert.

## Closing

We as embedded developers should be using every tool in the toolbox to help us
and our teams ship rock solid code, firmware, and embedded products. Asserts are
only one piece of this puzzle, and they need to be paired with solid debugging
infrastructure such as logging, postmortem backtrace and coredump collection,
and automated analysis of these diagnostics.

{% include submit-pr.html %}

## Reference Links

[^1]: [MyNewt Coredump Documentation](https://mynewt.apache.org/latest/tutorials/tooling/error_diagnostics.html#coredump)
[^2]: [ESP32 Coredump Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/core_dump.html#esp32-core-dump)
[^3]: [Memfault Error Analysis](https://memfault.com/features/error-analysis.html?utm_source=interrupt&utm_medium=link&utm_campaign=debugging-asserts)
[^4]: [FreeRTOS - xTaskCreateStatic](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/7c67f18ceebd48ae751693377166df0c52f4a562/tasks.c#L589-L605)
[^5]: [nRF52840 Development Kit](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK)
[^6]: [JLinkGDBServer](https://www.segger.com/products/debug-probes/j-link/tools/j-link-gdb-server/about-j-link-gdb-server/)
[^7]: [GNU ARM Embedded toolchain for download](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
[^8]: [Zephyr `sys/assert.h`](https://github.com/zephyrproject-rtos/zephyr/blob/b8add4aa0b20b3b15c989ee1a03443ba154d06a7/include/sys/__assert.h#L54-L60)
[^9]: [MyNewt `OS_CRASH()`](https://github.com/apache/mynewt-core/blob/f598bc4bf0b28aaa51f0bf7f9b9318848cef8c77/kernel/os/include/os/os_fault.h#L32-L36)
