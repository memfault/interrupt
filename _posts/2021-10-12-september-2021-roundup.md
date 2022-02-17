---
title: "What we've been reading in September (2021)"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

Here are the articles, videos, and tools that we've been excited about this
September.

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).

## Articles & Learning

- [Troubleshooting at Sea - Debugging Remote Arm Cortex-M Devices without Physical Access](https://devsummit.arm.com/en/tech-hub/144) by Alvaro Prieto<br>
ARM DevSummit is happening Oct 19th - 21st, and one of Memfault's customers, Sofar Ocean, talks about how they debug devices without physical access in the middle of the Pacific Ocean. Low power, bandwidth constrained, and no access for months. 

- [TDD in C with Ceedling and WSL2 | Sticky Bits](https://blog.feabhas.com/2021/10/tdd-in-c-with-ceedling-and-wsl2-performance-issues/) by Niall Cooling<br>
Niall talks about how having your project in the Windows filesystem but compiling in Window's WSL environment can cause build times to be slow. He suggests a clever "hack" where you push/pull from Windows to WSL and vice versa to help speed things up.

- [#556 – Firmware for Hardware Engineers with Phillip Johnston | The Amp Hour Electronics Podcast](https://theamphour.com/556-firmware-for-hardware-engineers-with-phillip-johnston/)<br>
A wonderful podcast with Phillip Johnston of Embedded Artistry talking about best practices for developing and testing firmware. Worth the listen.

- [Programming the STM32 Option Bytes using SEGGER J-Flash | Beningo Embedded Group](https://www.beningo.com/configuring-the-stm32-option-bytes-using-segger-j-flash/) by Jacob Beningo<br>
A short article talking about how to use a J-Link to program the STM32's Option Bytes, the registers that control various features of the chip such as read/write protection of the flash chip, watchdog configuration, and more.

- [Building an IoT Product — Continuous Battery Lifetime Testing | by tado° - Product Development | tado° — Product Development Blog | Sep, 2021 | Medium](https://medium.com/tado-product-development-blog/building-an-iot-product-continuous-battery-lifetime-testing-3245b6cdfa40) by Matthias Boesl<br>
Testing power consumption in an automated fashion is notoriously difficult for embedded devices, as it requires tons of extra hardware and software in the testing loop to do so. Matthias from tado° goes into detail about how they built such a system using a [Joulescope](https://www.joulescope.com/) in their CI/CD pipeline.

- [Fun with an ATTiny85, Liyafy HC-35 keypad with eight LEDs, and a serial to parallel shift register](https://www.nu42.com/2021/01/attiny85-liyafy-hc-35-8-led-keypad-serial-in-parallel-out-shift-register.html) by A. Sinan Unur<br>
In their search to build something more difficult and struggle a little, the author purchases an undocumented keypad, an ATtiny85, and a well-documented OLED display and tries to get something working.

- [How does gdb call functions? | Julia Evans](https://jvns.ca/blog/2018/01/04/how-does-gdb-call-functions/) by Julia Evans<br>
A fun post digging into some lower-level functionality of GDB. Not entirely relevant to embedded software, but I found it useful, especially when I'm debugging my unit tests in GDB.

- [Writing embedded firmware using Rust | AnyLeaf](https://www.anyleaf.org/blog/writing-embedded-firmware-using-rust)<br>
AnyLeaf builds sensors for scientists, such as water monitors and temperature sensors. In their latest blog post, they talk about why they chose Rust to write the firmware for their water monitor, why you should consider it, and they post some code snippets as well.

- [Dev Ops with CMake | Stratify Labs](https://blog.stratifylabs.co/device/2021-9-29-CMake-Dev-Ops/)<br>
The author calls out that CMake could be used to bootstrap a development environment. This could entail downloading and sourcing the ARM toolchain and other scripts. This could be a replacement for using Docker or Conda for instance.

- [Grafana Data Source for The Things Network](https://lupyuen.github.io/articles/grafana) by Lup Yuen Lee<br>
Lup Yuen Lee connects a device to The Things Network and publishes CBOR-encoded sensor data to an MQTT broker. Once this is in place, he talks about hooking up Grafana & Prometheus to the broker to plot the data and store it long-term.

- [CFI CaRE: Hardware-supported Call and Return Enforcement for Commercial Microcontrollers - 1706.05715.pdf](https://arxiv.org/pdf/1706.05715.pdf)
Details a technique using ARM's TrustZone-M to keep a "shadow stack" to prevent some classes of return-oriented-programming exploits.

## Tools & Projects
- [ARM-software/LLVM-embedded-toolchain-for-Arm: A project dedicated to build LLVM toolchain for 32-bit Arm embedded targets | GitHub](https://github.com/ARM-software/LLVM-embedded-toolchain-for-Arm)<br>
A wild project has appeared for an LLVM-based ARM toolchain to compile C/C++ code for architectures starting at Armv6-M! Works on Windows too.

- [flipperdevices/flipperzero-firmware: Flipper Zero Firmware | GitHub](https://github.com/flipperdevices/flipperzero-firmware)<br>
The open-source firmware for the [Flipper Zero](https://flipperzero.one/). [Their blog is great too](https://blog.flipperzero.one/).

- [J-Run: Automating performance tests on real hardware | SEGGER Blog](https://blog.segger.com/j-run-automating-performance-tests/) by Paul Curtis<br>
A tool that uses a J-Link to help orchestrate running on-device integration and/or unit tests. It works by capturing the logs from the RTT buffer on a device and verifying the output. Clever!

- [USBGuard/usbguard: Software Framework for USB device authorization policies | GitHub](https://github.com/USBGuard/usbguard)<br>
Neat tool which allows you to, on Linux, write rules to whitelist or blacklist devices based upon the attributes of the devices connecting.

- [gimli-rs/ddbug: Display debugging information | GitHub](https://github.com/gimli-rs/ddbug)<br>
An easy to use Rust-based CLI tool to print DWARF information of ELF or Mach-O executables

- [pkolbus/compiler-warnings: A list of compiler warning flags for different GCC and clang versions | GitHub](https://github.com/pkolbus/compiler-warnings)<br>
Where has this been my whole firmware career? For each compiler and for each version, this repo has all the warnings and errors and the delta between each version. I would have loved this when upgrading the GCC compiler from 4.x to 5.x for a few projects and trying to compile with -Werror and see what surfaces.

- [Wenzel/checksec.py: Checksec tool in Python, Rich output. Based on LIEF | GitHub](https://github.com/Wenzel/checksec.py)<br>
Cross-platform tool to print and verify the security properties of your binaries.

- [rui314/mold: mold: A Modern Linker | GitHub](https://github.com/rui314/mold)<br>
A next-generation linker that seeks to replace existing linkers. It claims to be able to link Firefox and Chrome 5-7x quicker than LLVM's lld and 20x quicker than GNU gold. Not yet compatible with ARM, but [hopefully one day](https://github.com/rui314/mold/issues/55).

## Random

- [Lead Time Trends | DigiKey](https://www.digikey.com/en/resources/reports/lead-time-trends)<br>
A great but definitely sad dashboard of lead times for common hardware parts.
