---
title: "What we've been reading in October (2022)"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

Here are the articles, videos, and tools that we've been excited about this
October

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).

## Articles & Learning

- [**The Power of Firmware Metrics - Memfault Webinar**](https://go.memfault.com/webinar-power-of-metrics-monitoring-battery-life-connectivity-power-consumption) by Tyler Hoffman<br>
  I'm doing a webinar on the most important metrics to monitor on battery-powered embedded devices. It will extend on my [heartbeat metrics Interrupt post]({% post_url 2020-09-02-device-heartbeat-metrics %}).

- [**Understanding Arm Cortex-M Intel-Hex (ihex) files - Feabhas Sticky Bits**](https://blog.feabhas.com/2022/10/understanding-arm-cortex-m-intel-hex-ihex-files/) by Niall Cooling<br>
  A post on the Intel Hex formats that many debuggers and programmers use when flashing firmware on devices.

- [**Make your QEMU 10 times faster with this one weird trick - Linus's blog**](https://linus.schreibt.jetzt/posts/qemu-9p-performance.html) by Linus Heckemann<br>
  Nice write-up about using perf to diagnose a QEMU performance issue.

- [**What is io_uring?**](https://unixism.net/loti/what_is_io_uring.html)<br>
  Didn't know much about io_uring myself, so was happy bout finding Lord of the io_uring with a quick summary but also deeper materials

- [**Call Stack Logger - Function instrumentation as a way to trace program’s flow of execution - DEV Community 👩‍💻👨‍💻**](https://dev.to/taugustyn/call-stack-logger-function-instrumentation-as-a-way-to-trace-programs-flow-of-execution-419a) by Tomasz Augustyn<br>
  Posted in the Interrupt community, a step by step guide to logging all function calls in C++ for debugging purposes on devices that are not currently connected to a debugger.

- [**Arm Assembly, Reverse Engineering, Exploitation - Azeria**](https://azeria.gumroad.com/)<br>
  some cool cheatsheets for assembly!

- [**GPS**](https://ciechanow.ski/gps) by Bartosz Ciechanowski<br>
  Lots of wonderful stuff on this website, but in particular I enjoyed this description of GPS and time.

- [**The HTTP crash course nobody asked for**](https://fasterthanli.me/articles/the-http-crash-course-nobody-asked-for) by amos, fasterthanlime<br>
  Another spectacular article from an author we've featured before, this time all about HTTP

## Projects & Tools

- [**Linux source code (v4.17.19) - Bootlin**](https://elixir.bootlin.com/linux/v4.17.19/source)<br>
  An Elixer-based source code browser that has indexed the Zephyr, Linux, FreeRTOS, and many more code bases.

- [**espressif/freertos-gdb: Python module for operating with freeRTOS kernel objects in GDB**](https://github.com/espressif/freertos-gdb)<br>
  I try to look for new GDB Pythons scripts every year or so since I [really]({% post_url 2020-04-14-gdbundle-plugin-manager %}) [like]({% post_url 2019-07-23-using-pypi-packages-with-GDB %}) them. This time, I came across a parsing library for FreeRTOS internal data structures that was written by the Espressif folks. I like that they allow the package to be installed normally. Note that this can be used on any FreeRTOS installation.

- [**pigz/try.h at master · madler/pigz**](https://github.com/madler/pigz/blob/master/try.h)<br>
  Interesting approach to implementing try-catch in C99.

- [**dottspina/dtsh: Shell-like interface to DeviceTrees**](https://github.com/dottspina/dtsh)<br>
  This popped up in the Zephyr #devicetree channel, still in the PoC phase. We haven't installed to give it a test yet, but seems promising for DeviceTree debugging and learning.

- [**Libcamera Official Repo**](https://git.libcamera.org/libcamera/libcamera.git/tree/README.rst)<br>
  Libcamera, a new camera control library for Linux, Android, and ChromeOS, had its first release!

- [**rbaron/b-parasite: 🌱💧 A Bluetooth Low Energy (BLE) soil moisture sensor.**](https://github.com/rbaron/b-parasite)<br>
  An open-source hardware and software soil moisture monitor.

- [**ExtremeGTX/USBWatcher: A simple GUI tool to manage USB Serial Devices**](https://github.com/ExtremeGTX/USBWatcher)<br>
  A tool to rename USB serial ports on Windows

- [**stawiski/Saleae-MIPI-DSI-LP-Analyzer: Saleae logic analyzer library for MIPI DSI Low Power mode commands**](https://github.com/stawiski/Saleae-MIPI-DSI-LP-Analyzer)<br>
  A salae plug-in for MIPI DSI LP mode. I’ve looked for this before when debugging MIPI panel bring up code!

## Random

- [**The Matthew Test: 2022 Survey Results - Cove Mountain Software**](https://covemountainsoftware.com/2022/10/27/the-matthew-test-2022-survey-results/) by Matthew Eshleman<br>
  A survey was done by the team at Cove Mountain Software to compare team and company size demographics against a best practices list for firmware development. The most shocking thing here is the 52% of teams that don't use Continuous Integration. Yikes!

- [**Some remotely exploitable kernel WiFi vulnerabilities [LWN.net]**](https://lwn.net/Articles/911062/)<br>
  Not great! 5 hardware-independent CVEs in the Linux WiFi stack.

- [**Announcing KataOS and Sparrow | Google Open Source Blog**](https://opensource.googleblog.com/2022/10/announcing-kataos-and-sparrow.html)<br>
  A new embedded OS by Google written in Rust. It looks like it will be in the same class of firmware as embedded Linux.
