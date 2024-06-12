---
title: Exploring printf on Cortex-M
description:
  Diving into the various ways to get printf on Cortex-M microcontrollers
author: noah
---

<!-- excerpt start -->

The C `printf` function is a staple of embedded development. It's a simple way
to get logs or debug statements off the system and into a terminal on the host.
This article explores the various ways to get `printf` on Cortex-M
microcontrollers.

<!-- excerpt end -->

{% include newsletter.html %}

{::options toc_levels="1..4" /}

{% include toc.html %}

## `printf`

Ah, the first and last of debugging techniques!

Let's first explore what happens when printf runs. Here's a simplified flowchart
showing what happens when `printf` is called:

{% blockdiag size:120x40 %} blockdiag { A [label = "printf"]; B [label =
"vprintf"]; C [label = "_write syscall"]; A -> B -> C; } {% endblockdiag %}

Going call by call:

1. `printf` is called by the application, for example:

   ```c
   printf("hello world! today is %d-%d-%d\n", 2021, 2, 16);
   ```

2. `printf` calls `vprintf`, the variadic version of `printf` (simplifying a bit
   for this example). `vprintf` processes the format string and consumes the
   "variadic args" (here the date components), and then calls `_write` to output
   the formatted data.

3. `_write` is a libc function responsible for writing bytes to a file
   descriptor. It's called with the file descriptor for stdout (1), and a
   pointer to the formatted string. It executes the necessary system call to
   write the data to the appropriate file descriptor. On hosted platforms (eg
   Linux, macOS, Windows), this is usually a `write` syscall. On embedded
   platforms, this might be a call to a UART driver.

### Using `printf` in a bare-metal application

On unhosted platforms (eg bare-metal or minimal RTOS), you may not have built-in
support for `printf`/`scanf`/`fprintf`. Fear not, you can get some printf in
your life by using newlib-nano's support:

```bash
# add these to your linker command line
LDFLAGS += --specs=nano.specs --specs=nosys.specs
```

That adds libc and "nosys" support to your application, which provide the
implementations for the various libc functions like `printf`, `sprintf`, etc.
This means you can add `printf` calls and your application will link! But we're
not quite done.

### Redirecting `_write`

`_write()` is the function eventually called by `printf`/`fprintf` responsible
for emitting the bytes formatted by those functions to the appropriate file
descriptor.

If you want to redirect the output of your `printf`/`fprintf` calls, you might
want to implement your own copy of `_write()`. By default, newlib libc doesn't
provide an implementation for `_write()`: you might see an error like this when
linking:

```log
arm-none-eabi/lib/thumb/v7e-m/nofp/libg_nano.a(libc_a-writer.o): in function `_write_r':
newlib/libc/reent/writer.c:49: warning: _write is not implemented and will always fail
```

See a thorough explanation here[^0].

The minimal implementation might look like this:

```c
// stderr output is written via the 'uart_write' function
#include <stddef.h>  // for size_t
#include <unistd.h>  // for STDERR_FILENO
int _write (int file, const void * ptr, size_t len) {
  if (file == STDERR_FILENO) {
    uart_write(ptr, len);
  }

  // return the number of bytes passed (all of them)
  return len;
}
```

Note that the libc `setbuf` API controls I/O buffering for newlib. By default
`_write` will be called for every byte. You might want to instead buffer up to
line ends, by:

```c
#include <stdio.h>
setvbuf(stout, NULL, _IOLBF, 0);
```

See `man setvbuf` or <http://www.cplusplus.com/reference/cstdio/setvbuf/> for
details.

### Reentrancy

Fancy word for if a function can be safely premepted and called simultaneously
from multiple (interrupt) contexts.

Some libc functions in newlib share a context structure that contains global
data structures. If you want to safely use them in multiple threads, you have a
couple of options:

- only use these functions in one thread in your program
- disable interrupts when using non-reentrant functions
- only use the reentrant versions of the functions (denoted by `_xxx_r()`, eg
  `_printf_r` in newlib)
- recompile newlib with `__DYNAMIC_REENT__` to have the non-reentrant functions
  call a user-defined function to get the current thread's reentrancy structure
- override the default global reentrancy structure as necessary

See the header file here for some detailed information:

<https://github.com/bminor/newlib/blob/6226bad0eafe762b811c62d1dc096bc0858b0d1a/newlib/libc/include/reent.h>

And great explanations here- especially if using FreeRTOS, there's built-in
support for managing reentrancy structures:

- <https://www.codeinsideout.com/blog/freertos/reentrant/>
- <https://mcuoneclipse.com/2020/11/15/steps-to-use-freertos-with-newlib-reentrant-memory-allocation/>

Let's go on a small side adventure into reentrancy approaches for
newlib\[-nano\]!

#### One thread only

It might not be necessary to use the non-reentrant functions in small interrupt
handlers (and is probably a reasonable idea to prohibit them).

Here's an approach that will cause a runtime error if a non-reentrant function
is called in an interrupt context. Unfortunately it requires rebuilding newlib,
so it's really only for curiosity or the very paranoid.

First, rebuild newlib (or build it as part of your project build; this is
probably only practical if you're using a compiler cache like ccache), setting
`__DYNAMIC_REENT__` when building.

Then, when building your application source, define a `__getreent()` function
with the following contents:

```c
#include <reent.h>
// Function to check if in interrupt context
int isInInterruptContext(void) {
    // Read the CONTROL register
    uint32_t control_reg;
    asm volatile ("MRS %0, CONTROL" : "=r" (control_reg));

    // Check the nPRIV bit (bit 0)
    return (control_reg & 1) != 0;
}
struct _reent * __getreent(void) {
  if (isInInterruptContext()) {
    __asm__ ("bkpt 0");
  }
  // return the built in global reentrancy context
  return _impure_ptr;
}
```

Also be sure to define `__DYNAMIC_REENT__` in your CFLAGS when building your
application source.

This example trips a software breakpoint if a non-reentrant function is called
from an interrupt context, to catch the error condition. Alternatively, the
function could return `NULL`, and libc calls using the reentrancy structure
_should_ return with no effect, but in practice this may not always be handled
in newlib.

Note that the same trap can be set up (applying to `printf` only though) by
adding a similar snippet to `_write()` (or using linker `--wrap` to inject the
check), without requiring rebuilding newlib 🥹.

#### Disable interrupts

This approach requires that every lower-priority thread enclose any
non-reentrant calls in a critical section, eg:

```c
__disable_irq();
printf("phew!\n");
__enable_irq();
```

This can add latency to interrupts (on cortex parts, typically interrupts will
pend until enabled after the call), particularly bad if your printf output is
relatively slow, eg 115200 baud UART for example.

It also requires you to be diligent about where you are using non-reentrant
functions.

#### Only use reentrant libc functions

This approach requires only calling the reentrant versions of the libc
functions. See an example below.

```c
#include <reent.h>
#include <stdio.h>

// call printf from an interrupt without breaking other threads
void Foo_Interrupt_Handler(void) {
  struct _reent my_impure_data = _REENT_INIT(my_impure_data);
  _printf_r(&my_impure_data, "hey!\n");
}
```

This is a pretty manual technique but it is relatively simple, and doesn't
require disabling interrupts or knowing the relative preemption priority of the
current call site.

#### Dynamic reentrancy

See the section above about building newlib; we need to enable
`__DYNAMIC_REENT__` in the library build to have the non-reentrant functions
call `__getreent()` (i.e. enable it in the `crosstool-ng` configuration if
you're building a whole toolchain). Then we can implement a version that
provides the correct reentrancy structure depending on our thread context:

```c
// this system only needs 1 reentrancy context
static struct _reent isr_impure_structs[] = {
  _REENT_INIT(isr_impure_structs[0]),
};

struct _reent * __getreent (void) {
  struct _reent *isr_impure_ptr;

  // this might use NVIC calls to figure out which ISR is active, or if there is
  // an OS, return current executing thread
  int thread_id = get_thread_id();

  switch thread_id {
    case 0:
      isr_impure_ptr = isr_impure_structs[0];
    default:
      // crash
      __asm__("bkpt 0");
  }

  return isr_impure_ptr;
}
```

Note that if you're using an RTOS, this may already be handled in some way or
there might be a hook when switching thread contexts (see [above](#reentrancy))
where we can move `__impure_ptr`, and we don't even need to recompile newlib!

For example, Zephyr RTOS
[handles logging from interrupts](https://docs.zephyrproject.org/3.4.0/services/logging/index.html#default-frontend)
or multiple preemptible thread contexts, so there's no need to fiddle with the
low-level libc reentrancy structures.

#### Overriding `__impure_ptr`

Another option is to override the global reentrancy structure when inside
interrupts; a little trickier but can be useful in certain systems.

The global reentrant structure is referenced by the pointer `__impure_ptr`,
defined here:

<https://sourceware.org/git/?p=newlib-cygwin.git;a=blob;f=newlib/libc/reent/impure.c;h=76f67459e48d3efc20698f8fda39620b1359f63f;hb=HEAD#l27>

You can instantiate your own copy of the structure:

```c
#include <reent.h>
struct _reent my_impure_data = _REENT_INIT(my_impure_data);
```

And then temporarily override `_impure_ptr`:

```c
_impure_ptr = &my_impure_data;
printf("yolo\n");
// restore default global reentrancy struct
_impoure_ptr = &impure_data;
```

This makes it safe to call the above snippet in an interrupt handler, because it
won't stomp on the global reentrancy data if it preempted an in-progress
non-reentrant function!

Note that you'll need to do the same thing in every preemption context, i.e. any
call site that can interrupt another thread that is using non-reentrant
functions. So this approach would be primarily useful in cases with only a small
number of threads, or a well-considered preemption hierarchy where we know
exactly where to move the pointer and where we don't need to.

## Redirecting stdio

This section describes a few approaches for redirecting libc standard
input/output functions, and compares the tradeoffs each one makes.

| Method      | Advantages                                                                              | Disadvantages                                                                              |
| ----------- | --------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------ |
| UART        | - simple, no special host HW needed                                                     | - requires spare UART + pins on target<br/>- can be slow                                   |
| Semihosting | - no extra HW requirements on target                                                    | - requires a debug probe for the host to use<br/>- _prohibitively_ slow, interrupts target |
| SWO         | - requires only a single (often specific) spare pin on the target<br/>- relatively fast | - usually needs a debug probe for the host to use<br/>- output only                        |
| RTT         | - no extra HW requirements on target<br>- relatively fast                               | - requires a debug probe for the host to use<br/>- license may be problematic              |

### UART

UART: asynchronous serial. Most embedded microcontrollers will have at least
one.

Pros:

- simple and widely available protocol (lots of available software and hardware
  tools interface to it)
- doesn't require an attached debugger; you can use it in PROD 😀

Cons:

- requires extra hardware to interface to the PC host, typically (USB to
  asynchronous serial adapter, like the FTDI or CP2102 adapters)
- requires configuring and using the UART peripheral (if your microcontroller
  doesn't have many UARTs, might be a problem using it for `printf`)

The biggest downside is you're spending one of your UART peripherals for printf,
which might be needed for your actual application.

Also, configuring a UART might require managing clock configuration, chip
power/sleep modes, figuring out buffering the data (DMA?), making sure
everything is interrupt safe, etc.

And finally, you'll usually need some kind of adapter dongle to interface the
UART with your PC (eg USB-to-TTL or USB-to-asynchronous serial).

Because the interface is contained entirely on the microcontroller, it can be
used without a debug probe, which can make it useful when not actively debugging
your system (eg you can use it to write log statements to an attached PC!).

Worth noting, if your microcontroller has support for USB, you might be able to
skip the need for an external FTDI-like adapter by implementing CDC
(Communications Device Class) on-chip. That adds a lot more software to your
system, but can be useful. Out of scope of this FAQ.

### Semihosting

Next up the food chain, semihosting is a protocol that proxies the "hosting"
(meaning libc hosting, eg `printf`/`scanf`/`open`/`close` etc) over an attached
debug probe to the PC.

Some good descriptions:

- <https://wiki.segger.com/Semihosting>
- <https://developer.arm.com/documentation/dui0375/g/What-is-Semihosting-/What-is-semihosting->

And the reference documentation can be found in:

> ARM Developer Suite (ADS) v1.2 Debug Target Guide, Chapter 5. Semihosting

This describes the detailed "RDIMON" ("Remote Debug Interface Monitor")
operation.

TLDR, semihosting operates by the target executing the breakpoint instruction
with a special breakpoint id, `bkpt 0xab`, with the I/O opcode stored in `r1`
and whatever necessary data stored in a pointer loaded into `r2`.

The debug probe executes the I/O operation based on the opcode and data, then
resumes the microcontroller.

To use it, the simplest approach is to use newlib's rdimon library spec:

```bash
# add these to your linker command line
LDFLAGS += --specs=nano.specs --specs=rdimon.specs
```

Then add this call somewhere in your system init, prior to calling any libc I/O
functions:

```c
// this executes some exchanges with the debug monitor to set up the correct
// file handles for stdout, stderr, and stdin
extern void initialise_monitor_handles(void);
initialise_monitor_handles();
```

`printf` and friends should work after that! Be sure to check the user manual
for your debug probe on how it exposes the semihosting interface. For example,
segger jlink requires running some commands to enable semihosting:

```bash
# from gdb client
monitor semihosting enable
monitor semihosting ioclient 2
```

(From <https://wiki.segger.com/J-Link_GDB_Server>).

For pyocd, you can pass the `--semihosting` argument when starting the gdb
server.

As an extra bonus, see below for a simplified semihosting solution based on the
newlib one that doesn't require linking against `rdimon.specs`, and saves some
code space. Note that some debug probes require the setup steps run by
[`initialise_monitor_handles()`](https://github.com/bminor/newlib/blob/1a0908203606527b6ac0ed438669b5bcd247a5f9/newlib/libc/sys/arm/syscalls.c#L114-L169),
so those may need to be included as well (depending on your setup).

```c
// Based on the implementation here:
// https://github.com/bminor/newlib/blob/1a0908203606527b6ac0ed438669b5bcd247a5f9/newlib/libc/sys/arm/syscalls.c#L314-L338
// And here:
// https://github.com/bminor/newlib/blob/69f4c4029183fb26d2fcae00790881620c1978a3/newlib/libc/sys/arm/swi.h#L74-L89
int _write(int file, uint8_t *ptr, int len) {
  int data[3] = {file, (int) ptr, len};
  __asm__("mov r0, %0 \n mov r1, %1 \n bkpt #0xAB"
          : : "r" (5), "r" (&data) : "r0", "r1", "memory");
  return len;
}
```

Pros:

- only requires SWD connection (SWCLK + SWDIO)
- built into most debug servers (pyocd, openocd, blackmagic, segger jlink)
- basic implementation is provided by newlib (no extra user code) and is dead
  simple to use
- can do lots more than just printf; open, fwrite, read from stdin!

Cons:

- slowwww. really slow. tens to hundreds of milliseconds per transfer, depending
  on debug probe and host
- doesn't work when debugger is disconnected (target will be stuck on `bkpt`
  instruction; either a forever hang or a crash depending on cortex-m
  architecture set). this can worked around by handling the `DebugMon` exception
  on-chip, or by checking the DHCSR register[^1] `C_DEBUGEN` bit, to detect if a
  debugger is connected.
- the newlib implementation (`--specs=rdimon.specs`) consumes about 1.5-2kB of
  flash

### Serial Wire Output (SWO)

A dedicated pin that can be used essentially as a UART for outputting data. It
can be pretty fast- baud rate sometimes reaching 1/1 of CPU core clock, since it
uses the TPIU.

It is however an optional hardware feature, so might not be available on every
chip (most common chips like STM32's and nRF chips tend to include it), and
requires an extra pin- typically routed to the 10-pin ARM debug header.

It's output only, and there's a small bit of code necessary on the target to
enable routing `printf` to SWO.

A good tutorial, including background information and an example implementation,
can be found here:

<https://mcuoneclipse.com/2016/10/17/tutorial-using-single-wire-output-swo-with-arm-cortex-m-and-eclipse/>

The debug adapter and software does need to support the protocol. Most debug
adapters do support it, however (ST-Link's, JLINK, DAPLink, etc).

Pros:

- generally pretty fast, similar performance to UART (speed is both target and
  probe hardware dependent)
- simple implementation

Cons:

- output only
- uses a pin on the target
- not all chips or probes support it

### Real Time Transfer (RTT)

<https://wiki.segger.com/RTT>

Segger's RTT reads/writes into a buffer in RAM while the chip is running. It can
be pretty fast compared to even a fast UART, and is _much_ more efficient than
Semihosting without requiring any extra pins or hardware.

It also supports both output (target -> host) and input (host -> target).

Segger provides the RTT target implementation, as well as a few tools for using
RTT on the host. OpenOCD and PyOCD also support the protocol.

You can download the implementation here:

<https://www.segger.com/products/debug-probes/j-link/technology/about-real-time-transfer/#downloading-rtt-target-package>

Zephyr also maintains a copy here, if you're curious about perusing it (or want
to use it as a git submodule):

<https://github.com/zephyrproject-rtos/segger/tree/master/SEGGER>

The downside is it requires a debug adapter to operate, since it relies on
reading and writing to memory.

It's also important to understand the implications of enabling this in
production; be sure your device doesn't get into a bad state if there's no debug
adapter connected (i.e. when no debugger is attached, drop data instead of
backing up the RTT buffers).

I'm personally a big fan of RTT over semihosting- it's usually going to be a
strict improvement if you don't need file I/O.

Pros:

- fast!
- simple on-device implementation
- supported out of the box on Zephyr RTOS, among other platforms

Cons:

- requires a debugger with support for RTT

## `flush()`

That's all for now! Hopefully this article provided a little window into the
various ways `printf` can be used on small microcontrollers (or other unhosted
environments). It's a surprisingly subtle corner of embedded development, and
there's a few pitfalls that should be watched for, especially around concurrency
or performance.

The links below should provide a lot of additional information if you're curious
about exploring the topic further.

And as always, if you have any questions or comments, feel free to reach out-
either in the comments here or in the Interrupt community Slack!

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->

- [Interrupt: Introduction to ARM Semihosting]({% post_url 2021-02-16-arm-semihosting %})
- [Interrupt: From Zero to main(): Bootstrapping libc with Newlib]({% post_url 2019-11-12-boostrapping-libc-with-newlib %})
- [libc `setvbuf()` stdlib reference](http://www.cplusplus.com/reference/cstdio/setvbuf/)
- [Newlib `reent.h` header](https://github.com/bminor/newlib/blob/6226bad0eafe762b811c62d1dc096bc0858b0d1a/newlib/libc/include/reent.h)
- [Reentrancy in Newlib](https://www.codeinsideout.com/blog/freertos/reentrant/)
- [MCU on Eclipse: Steps to use FreeRTOS with newlib reentrant Memory Allocation](https://mcuoneclipse.com/2020/11/15/steps-to-use-freertos-with-newlib-reentrant-memory-allocation/)
- [Zephyr Logging Documentation](https://docs.zephyrproject.org/3.4.0/services/logging/index.html#default-frontend)
- [Newlib `_impure_ptr` declaration](https://sourceware.org/git/?p=newlib-cygwin.git;a=blob;f=newlib/libc/reent/impure.c;h=76f67459e48d3efc20698f8fda39620b1359f63f;hb=HEAD#l27)
- [SEGGER ARM Semihosting Wiki](https://wiki.segger.com/Semihosting)
- [ARM Semihosting Documentation](https://developer.arm.com/documentation/dui0375/g/What-is-Semihosting-/What-is-semihosting-)
- [Newlib libc `initialise_monitor_handles()`](https://github.com/bminor/newlib/blob/1a0908203606527b6ac0ed438669b5bcd247a5f9/newlib/libc/sys/arm/syscalls.c#L114-L169)
- [Newlib semihosting `_write` implementation](https://github.com/bminor/newlib/blob/1a0908203606527b6ac0ed438669b5bcd247a5f9/newlib/libc/sys/arm/syscalls.c#L314-L338), and [here](https://github.com/bminor/newlib/blob/69f4c4029183fb26d2fcae00790881620c1978a3/newlib/libc/sys/arm/swi.h#L74-L89)
- [MCU on Eclipse: Tutorial: Using Single Wire Output SWO with ARM Cortex-M and Eclipse](https://mcuoneclipse.com/2016/10/17/tutorial-using-single-wire-output-swo-with-arm-cortex-m-and-eclipse/)
- [SEGGER RTT Wiki](https://wiki.segger.com/RTT)
- [SEGGER RTT download page](https://www.segger.com/products/debug-probes/j-link/technology/about-real-time-transfer/#downloading-rtt-target-package)
- [Zephyr SEGGER RTT GitHub repo](https://github.com/zephyrproject-rtos/segger/tree/master/SEGGER)

## Footnotes

[^0]: [Howto: Porting newlib, A Simple Guide 😅](https://www.embecosm.com/appnotes/ean9/ean9-howto-newlib-1.0.html#id2719973)
[^1]: [ARMv7-M Architecture Reference Manual, Section C1.6.2](https://developer.arm.com/documentation/ddi0403/ed/)

<!-- prettier-ignore-end -->
