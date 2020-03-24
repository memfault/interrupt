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
firmware in it for STM32.
<!-- excerpt end -->

_Like Interrupt? [Subscribe](http://eepurl.com/gpRedv) to get our latest posts
straight to your mailbox_

{:.no_toc}

## Table of Contents

<!-- prettier-ignore -->
* auto-gen TOC:
{:toc}

## What is Renode?


### Why not QEMU?


## Installing Renode


## Running our Hello World

### A simple firmware

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

### Configuring Renode 

```
:name: STM32F4 Discovery Printf
:description: This script runs the usart_printf example on stm32f4 discovery

$name?="STM32F4_Discovery"
$bin?=@renode-example.elf

# Create Machine & Load config
mach create $name
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