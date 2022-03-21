---
title: Cortex-M Debug Adapter Rundown
description:
  Overview and comparison of some popular debug probes and software for flashing
  and debugging Cortex-M devices
author: noah
image: img/<post-slug>/cover.png # 1200x630
---

<!-- excerpt start -->

This article is intended as an overview and comparison of some of the current
popular debug probes used for flashing and debugging Cortex-M targets.

<!-- excerpt end -->

I wrote this article because I couldn't find one like that already existed!
Hopefully there's some useful information on the various probes and software
discussed below. If there's any tools I missed in this category, please let me
know in the comments or on theinterrupt.slack.com !

{% include newsletter.html %}

{% include toc.html %}

## Debug Adapter Hardware

First off, if you haven't already, take a look at Chris's deep dive on Cortex-M
debug interfaces, which also covers some of the probes in this article:

<https://interrupt.memfault.com/blog/a-deep-dive-into-arm-cortex-m-debug-interfaces>

The table below lists the most popular debug probes for Cortex-M devices at the
moment, to my knowledge.

Note that this article is focusing on probes that support Serial Wire Debug
(SWD, documentation
[here](https://developer.arm.com/documentation/ihi0031/a/The-Serial-Wire-Debug-Port--SW-DP-/Introduction-to-the-ARM-Serial-Wire-Debug--SWD--protocol)).
Some probes will support JTAG too, but when working with Cortex-M devices, it's
often not as preferrable as SWD due to higher pin utilization.

Also I'll be focusing on non-trace enabled probes (though some will support some
rudimentary trace functionality). That deserves a separate article, since
tracing (which is extremely useful!) is a somewhat more niche use case than
general flashing + halt debugging.

| Probe             | Price (USD, March 2022) | Built in debug serial port | Target power output     | Open Source | Software + documentation quality _see note below_ |
| ----------------- | ----------------------- | -------------------------- | ----------------------- | ----------- | ------------------------------------------------- |
| SEGGER J-Link ARM | $448                    | ❌                         | yes, +5v (300mA) only   | ❌          | ⏫ very good                                      |
| ST-Link           | $11 (v3)-$22(v2)        | ✅                         | yes, +3-3.6v only       | ❌          | 🔼 good                                           |
| Black Magic Probe | $60                     | ✅                         | yes, +3.3v (100mA) only | ✅          | 〰️ ok (but open source!)                          |
| DAPLink           | $5-$15                  | ❌                         |                         | ✅          | 〰️ ok (but open source!)                          |
| FTDI FT2232H      | $35 ($5 for chip only)  | ✅                         | varies, adafruit yes    | ❌          | 🔼 good                                           |
| Raspberry Pi Pico | $4                      | ✅                         |                         | ❌          | 🔼 good + open source                             |

> "Software + documentation quality" : This is a _very_ subjective personal
> rating of how reliable + feature-loaded the accompanying software is, and how
> comprehensive and useful the documentation is.

### J-Link / J-Trace

![](/img/debug-adapter-rundown/jlink-base.png){:height="250px"}

The venerable SEGGER J-Link series of debug probes is a classic for a reason.
These devices are very reliable and fast, and support a _LOT_ of target devices,
including flash algorithms (ability to write a program to target internal
flash).

It's fairly commonly included on development boards, which makes setup simpler
if you already have the host computer J-Link software installed. The integrated
J-Links often have a debug serial port attached to a target UART too!

Other features of note:

- semihosting, SWO, and RTT support
- great documentation, and extensible flash loader architecture. the wiki is
  particularly nice, includes a lot of context for various features:
  <https://wiki.segger.com/Main_Page>
- "EDU" variants that are very reasonably priced, about $60 (but may only be
  used in non-commercial applications)
- able to be flashed to on-board ST-Link probes (eg on the STM32 Discovery
  boards):
  <https://www.segger.com/products/debug-probes/j-link/models/other-j-links/st-link-on-board/>
  that tool requires a Windows host though.

### ST-Link

![](/img/debug-adapter-rundown/stlink-v2.jpg){:height="250px" style="float:
left"} ![](/img/debug-adapter-rundown/stlink-v3mini.jpg){:height="250px"}

ST Microelectronics' "ST-Link" debug probe has been around for a long time. It's
commonly included on dev boards as a built-in debug probe.

The probe devices + on-board software, as well as ST's default host-side tools,
are all closed source, but there are several open-source tools that are able to
talk to the ST-Link devices (openocd, PyOCD, probe-rs, etc).

There's several generations of the ST-Link probe. The V2(.1) are probably the
most common.

- overview of ST-Link varieties:
  <https://www.st.com/resource/en/technical_note/tn1235-overview-of-stlink-derivatives-stmicroelectronics.pdf>
- open source tools: <https://github.com/stlink-org/stlink>

### Black Magic Probe

![](/img/debug-adapter-rundown/black-magic.png){:height="250px"}

The Black Magic Probe is a fully open source (hardware + software) debug probe.
One standout feature of this device is the gdb server runs on the probe itself;
it appears as a serial device when plugged into the host computer, and you can
directly target it with `extended-remote` without any other software required.

The Black Magic Probe firmware is also capable of being installed on a few other
hardware devices, including ST-Link's and stm32 boards:

<https://github.com/blackmagic-debug/blackmagic/tree/e82d4f2edaefb74c9dee27f6d26c4d2038af0384/src/platforms>

See the links below for some more information:

- <https://1bitsquared.com/products/black-magic-probe>
- <https://github.com/blackmagic-debug/blackmagic/wiki>

### DAPLink

![](/img/debug-adapter-rundown/MAX32625PICO.png){:height="250px"}

DAPLink is an open-source intiative started by ARM Mbed. It defines a standard
interface for host tools to interact with any Cortex-M target via a USB HID
based adapter device (the probe itself).

The original DAPLink probes are no longer stocked, but a de-facto replacement in
the [MAX32625PICO](https://os.mbed.com/platforms/MAX32625PICO).

Also, many arduino devices can operate as DAPLink adapters, for example the very
inexpensive "XIAO" SAMD21 dev board:

<https://www.seeedstudio.com/Seeeduino-XIAO-Arduino-Microcontroller-SAMD21-Cortex-M0+-p-4426.html>

There's also a WebUSB-based host available:

<https://github.com/ARMmbed/dapjs>

Some more information in the following links:

- <https://github.com/armmbed/DAPLink>
- <https://github.com/Seeed-Studio/Seeed_Arduino_DAPLink/>

### FTDI FT2232H

The classic FTDI Serial-to-USB adapter IC's can be used as generic SWD adapters.

OpenOCD has a built-in configuration for using those devices as a debug probe:

<https://openocd.org/doc/html/Debug-Adapter-Hardware.html>

Depending on which FTDI chip you're using, you can use spare UARTs for a debug
console to your target.

One nice thing about this approach is it's possible to add the FTDI chip to your
own custom dev board, to have a on-board debug adapter just like the commercial
dev boards.

### Raspberry PI Pico

![](/img/debug-adapter-rundown/pico.png){:height="250px"}

- <https://www.digikey.com/en/maker/projects/raspberry-pi-pico-and-rp2040-cc-part-2-debugging-with-vs-code/470abc7efb07432b82c95f6f67f184c0>

<!-- ## Debug Adapter Software

### J-Link

- https://www.segger.com/purchase/pricing/jlink-related/#adapters

### OpenOCD

### PyOCD

CMSIS-Pack

### probe-rs

https://probe.rs/docs/knowledge-base/cmsis-packs/

"HS probe" is the hardware

https://github.com/probe-rs/probe-rs -->

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->

- <https://interrupt.memfault.com/blog/a-deep-dive-into-arm-cortex-m-debug-interfaces>
  : how the debug stack works on Cortex-M chips, including the role of the debug
  probe device
- <https://interrupt.memfault.com/blog/profiling-firmware-on-cortex-m> :
  strategies for profiling without full hardware tracing
- <https://interrupt.memfault.com/blog/cortex-m-debug-monitor> : implement a
debug probe in software that runs on the target MCU!
<!-- prettier-ignore-end -->
