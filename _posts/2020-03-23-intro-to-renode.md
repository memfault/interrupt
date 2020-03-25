---
title: "Cortex-M Emulation with Renode"
description: "WIP"
author: francois
---

In this new era of work-from-home, firmware engineer may not have all the
equipment and development board they are used to having at the office. One way
around this? Emulation!

While they're not quite the real thing, emulators can run our firmware, print
data over UART, read registers from I2C sensors, and even run a filesystem on a
SPI flash device. That's more than enough to write some real firmware!

<!-- excerpt start -->
In this post, I walk through setting up the Renode emulator and running a
firmware in it for STM32. Using that setup, we'll debug our firmware, run it
through integrated tests, and shorten the iteration cycle of development.
<!-- excerpt end -->

_Like Interrupt? [Subscribe](http://eepurl.com/gpRedv) to get our latest posts
straight to your mailbox_

{:.no_toc}

## Table of Contents

<!-- prettier-ignore -->
* auto-gen TOC:
{:toc}

## What is Renode?

[Renode](https://renode.io/) is an open source Emulator for embedded platforms.
Today, it supports x86 (Intel Quark), Cortex-A (Nvidia Tegra), Cortex-M, SPARC
(Leon), and RISC-V based platforms.

Renode can take the same firmware you are running in production, and run it
againsts emulated cores, peripherals, and even sensors and actuators. Better
yet, its extensive networking support and multi-system emulation make it a shoe
in for testing systems made up of multiple devices talking together.

With Renode, you can start development before your hardware is ready, test your
firmware without deploying racks of hardware, and shorten your iteration cycles
by cutting out flash loading delays.

Renode is built using the Mono framework, which allows it to run cross-platform.

> **Why not QEMU?** - Readers who have experience with emulation will point out
> that QEMU has existed for a long time, and is capable of emulating Cortex-M
> targets. In our experience, QEMU is focused on emulating systems meant for
> higher level OSes (e.g. Linux computers) rather than embedded devices. To
> date, it only supports two cortex-M series targets, both made by TI.

## Installing Renode

The Renode projects releases installers for Windows, MacOS, and multiple Linux
distributions. As of this writing, you can find the v1.9 release on
[Github](https://github.com/renode/renode/releases/tag/v1.9.0).

This guide was written on MacOS, though it is not OS specific.

To verify your Renode installation, you can run one of the examples:

1. Open Renode, on MacOS I prefer to use the command line directly with `$ sh
   /Applications/Renode.app/Contents/MacOS/macos_run.command`
2. A Renode terminal window will open. Load the example with `start
   @scripts/single-node/stm32f4_discovery.resc`
![](/img/intro-to-renode/renode-first-demo-start.png)
3. A second terminal window should open, displaying serial output
![](/img/intro-to-renode/renode-first-demo-output.png)


## Running our own firmware

Let's write a hello world firmware to run via Renode. We will target the STM23F4
Discovery board, as the emulator supports it out of the box.

### A simple firmware

I chose to use libopencm3[^1], an open source library for ARM cortex-M
microcontrollers. I enjoy how easy it is to get started with it and appreciate
the consistent API across microcontrollers. Note however that the library is
still in its early stages and likely not fit for production use. I used the
libopencm3 template [^2] as a starting point.

To get to a "hello, world!", we must:
1. Setup the peripheral clocks
2. Enable the UART GPIOs
3. Configure the UART peripheral
4. Implement the `_write` syscall

I've reproduced the source code below, you can find the full project (Makefile
included) in the interrupt Github repo.

```c
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

static void clock_setup(void)
{
    /* Enable GPIOD clock for LED & USARTs. */
    rcc_periph_clock_enable(RCC_GPIOD);
    rcc_periph_clock_enable(RCC_GPIOA);

    /* Enable clocks for USART2. */
    rcc_periph_clock_enable(RCC_USART2);
}

static void usart_setup(void)
{
    /* Setup USART2 parameters. */
    usart_set_baudrate(USART2, 115200);
    usart_set_databits(USART2, 8);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_mode(USART2, USART_MODE_TX);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

    /* Finally enable the USART. */
    usart_enable(USART2);
}

static void gpio_setup(void)
{
    /* Setup GPIO pin GPIO12 on GPIO port D for LED. */
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12);

    /* Setup GPIO pins for USART2 transmit. */
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);

    /* Setup USART2 TX pin as alternate function. */
    gpio_set_af(GPIOA, GPIO_AF7, GPIO2);
}

int _write(int file, char *ptr, int len)
{
    int i;

    if (file == STDOUT_FILENO || file == STDERR_FILENO) {
        for (i = 0; i < len; i++) {
            if (ptr[i] == '\n') {
                usart_send_blocking(USART2, '\r');
            }
            usart_send_blocking(USART2, ptr[i]);
        }
        return i;
    }
    errno = EIO;
    return -1;
}

int main(void) {
    clock_setup();
    gpio_setup();
    usart_setup();

    printf("hello world!\n");

    while (1) {}

    return 0;
}
```

If all goes well, we now have a `renode-example.elf` file we can load in the
emulator.

### Running our firmware in Renode

Next up, we must spin up a machine in Renode and run our firmware in it.

Like we did earlier, we first start Renode:
```
# Note: on Linux this just is the "renode" command
$ sh /Applications/Renode.app/Contents/MacOS/macos_run.command
```

The window that pops up is called the Renode *monitor*. In it, you can run
Renode commands.

We now need to create a *machine*. A machine represents a device, which can have
a number of cores. Renode can run multiple machines in a single run.

We can create, add, and remove machines with the `mach` command:

```
(monitor) mach create
(machine-0)
```

The prompt now includes our machine name. Since we did not name our machine, it
just says "machine-0".

Next, we configure our machine. We could specify each bus and peripheral by
hand, but instead we will load a prebuilt configuration.

```
(machine-0) machine LoadPlatformDescription @platforms/boards/stm32f4_discovery-kit.repl
(machine-0)
```

You will notice that the path is prepended with the `@` sign. This is a C#-ism.
Renode will load absolute paths, as well as paths relative to your working
directory, and paths relative to the renode installation. In our case, we used
the latter as `platforms` is bundled with the Renode installation.

Next, we load our firmware:

```
(machine-0) sysbus LoadELF @renode-example.elf
(machine-0)
```

Before we start the machine, we want to do one last thing: open a terminal to
display UART data. Some peripherals come with what Renode calls "Analyzers",
which are ways to display their state and data. We enabled UART2 in our
firmware, so we will show the analyzer for UART2.

```
(machine-0) showAnalyzer sysbus.uart2
(machine-0)
```

You should now have two Renode windows open, the monitor window and the UART2
window.

We are now ready to run our simulation with the `start` command:

```
(machine-0) start
```

#### Adding CCM memory

... Well, this is embarassing. Nothing happened.

In the terminal window we used to start the Renode app, we see line after line
of warnings:

```
[...]
20:46:07.5783 [WARNING] sysbus: [cpu: 0x8000926] WriteByte to non existing peripheral at 0x10152AA3, value 0x0.
20:46:07.5783 [WARNING] sysbus: [cpu: 0x8000926] WriteByte to non existing peripheral at 0x10152AA4, value 0x0.
20:46:07.5783 [WARNING] sysbus: [cpu: 0x8000926] WriteByte to non existing peripheral at 0x10152AA5, value 0x0.
20:46:07.5795 [WARNING] sysbus: [cpu: 0x8000926] WriteByte to non existing peripheral at 0x10152AA6, value 0x0.
20:46:07.5799 [WARNING] sysbus: [cpu: 0x8000926] WriteByte to non existing peripheral at 0x10152AA7, value 0x0.
20:46:07.5800 [WARNING] sysbus: [cpu: 0x8000926] WriteByte to non existing peripheral at 0x10152AA8, value 0x0.
20:46:07.5800 [WARNING] sysbus: [cpu: 0x8000926] WriteByte to non existing peripheral at 0x10152AA9, value 0x0.
20:46:07.5800 [WARNING] sysbus: [cpu: 0x8000926] WriteByte to non existing peripheral at 0x10152AAA, value 0x0.
20:46:07.5801 [WARNING] sysbus: [cpu: 0x8000926] WriteByte to non existing peripheral at 0x10152AAB, value 0x0.
20:46:07.5801 [WARNING] sysbus: [cpu: 0x8000926] WriteByte to non existing peripheral at 0x10152AAC, value 0x0.
20:46:07.5801 [WARNING] sysbus: [cpu: 0x8000926] WriteByte to non existing peripheral at 0x10152AAD, value 0x0.
20:46:07.5802 [WARNING] sysbus: [cpu: 0x8000926] WriteByte to non existing peripheral at 0x10152AAE, value 0x0.
20:46:07.5802 [WARNING] sysbus: [cpu: 0x8000926] WriteByte to non existing peripheral at 0x10152AAF, value 0x0.
[...]
```

Looking at our chip's memory map, we notice that `0x10XXXXXX` is in the CCM
region. Turns out the renode model does not implement it. Fear not! This is
easily fixed.

The easiest way to modify our Machine to add the CCM RAM is to write a Renode
Platform (or "repl") file. Repl files use YAML-like syntax to define
peripherals, including their type, address, and size.

You can read more about the Platform Description file format in the [Renode
Documentation](https://renode.readthedocs.io/en/latest/advanced/platform_description_format.html)
Adding our CCM region requires the following:

```
// In "add-ccm.repl"
ccm: Memory.MappedMemory @ sysbus 0x10000000
    size: 0x10000
```

Since platform descriptions are composable, We can now load that file using the
same `LoadPlatformDescription` command we used earlier:

```
(machine-0) machine LoadPlatformDescription @add-ccm.repl
(machine-0)
```

We restart our machine, and voila!

![](/img/intro-to-renode/renode-hello-world.png)

### Automating setup with a `.resc` script

```
:name: STM32F4 Discovery Printf
:description: This script runs the usart_printf example on stm32f4 discovery

$bin?=@renode-example.elf

# Create Machine & Load config
mach create
machine LoadPlatformDescription @platforms/boards/stm32f4_discovery-kit.repl
machine LoadPlatformDescription @add-ccm.repl

# Create a terminal window showing the output of UART2
showAnalyzer sysbus.uart2

macro reset
"""
    sysbus LoadELF $bin
"""

runMacro $reset
```


## Debugging with Renode


## Renode & Integration Tests

## References

- [^1] https://github.com/libopencm3/libopencm3
- [^2] https://github.com/libopencm3/libopencm3-template