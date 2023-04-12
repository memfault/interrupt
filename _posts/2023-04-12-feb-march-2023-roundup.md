---
title: "What we've been reading in February & March"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

Here are the articles, videos, and tools that we've been excited about this
February & March. We're a little late with the February issue but we'll catch up!

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).

## Events

- [**Boston Embedded Systems Happy Hour by Memfault**](https://www.meetup.com/boston-firmware-embedded-systems-meetup/events/292686106/)<br>
Team Memfault would be thrilled to host you at Workbar Boston downtown for food, beer, and to talk firmware! Much of the team from our Boston office will be in attendance, as well as many of our friends in firmware we've made over the years. More information on [Meetup](https://www.meetup.com/boston-firmware-embedded-systems-meetup/events/292686106/). If you are looking for a firmware meetup in the San Francisco Bay Area or New York, hold tight. We are working on it!

## Articles & Learning

- [**Interrupt Panel Discussion - OTA Updates & Fleet Management at Scale**](https://go.memfault.com/ota-updates-fleet-management-at-scale)<br>
David Shoemaker (Tesla), Phillip Johnston (Embedded Artistry), Eric (Memfault), Noah (Memfault) all talk about how each of them think about peforming OTA for hardware devices. Need a podcast of sorts to listen to while writing your new OTA code? This is the one.

- [**Utilizing Memfault to Debug Your Embedded Devices**](https://go.memfault.com/utilizing-memfault-debug-your-embedded-devices)<br>
Eric and Noah from Memfault cover how to use Memfault to debug firmware, both while it's connected to your local computer using JTAG and remotely using Memfault.

- [**Embedded Online Conference 2023 on April 24-28th**](https://embeddedonlineconference.com/index.php)<br>
This wonderful yearly conference is just around the corner. Use `MEMFAULT2023` to get $100 off registration fees. 

- [**The cheapest flash microcontroller you can buy is actually an Arm Cortex-M0+ - Jay Carlson**](https://jaycarlson.net/2023/02/04/the-cheapest-flash-microcontroller-you-can-buy-is-actually-an-arm-cortex-m0/)<br>
Jay Carlson is at it again with his latest post, showing that a Cortex-M0+ might be the best chip when you need to be cheap.  

- [**Avoiding Stack Overflows: Application Monitoring the Stack Usage - MCU on Eclipse**](https://mcuoneclipse.com/2023/02/19/avoiding-stack-overflows-application-monitoring-the-stack-usage/)<br>
A quick guide on how to detect stack overflows using common RTOSs and if you had to build it yourself. The more modern version of this on the Arm M33 is to use the MSPLIM and PSPLIM, as documented in a recent Interrupt article, [A Guide to Using ARM Stack Limit Registers]({% post_url
({% post_url
2019-12-11-reproducible-firmware-builds %}) %}).

- [**ESP32 Buyer’s Guide: Different Chips, Firmware, Sensors - eitherway**](https://eitherway.io/posts/esp32-buyers-guide/)<br>
A comparison of all of the ESP32 (and ESP8266) flavors that one can buy. 

- [**Rust on Espressif chips - 2023 Roadmap**](https://mabez.dev/blog/posts/esp-rust-24-02-2023/)<br>
Rust on ESP32 is continuing to chug along. This post covers a 2022 Q4 recap and 2023 roadmap for Rust on the ESP32 platform.

- [**RFID Reader for Apartment Door**](http://dlaw.me/door-rfid-reader/)<br>
A beautifully documented project implementing a retrofitted RFID door lock, including mechanical + electrical + Rust firmware. Very polished final result! - Noah.

- [**Improving the Beginner’s PID (Arduino)**](http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/)<br>
A nice article about some work that went into the Arduino PID library - Noah.

- [**Introduction to meson build system - twdev.blog**](https://twdev.blog/2022/09/meson/)<br>
A tutorial on using the Meson build system for a C/C++ project - Noah.

- [**Commercially available RISC-V silicon - Muxup**](https://muxup.com/2023q1/commercially-available-risc-v-silicon)<br>
List of commercially available RISC-V chips. Exciting to see adoption grow, with new and established vendors! (Plug: Memfault recently expanded our architecture support to include RISC-V on the ESP32-C3) - Noah.

- [**Case study of interweaving zephyr and zig**](https://github.com/nodecum/zig-zephyr)<br>
A new attempt to integrate Zig with Zephyr in various ways. The current state is very experimental, but interesting work on converting the device tree into Zig!

- [**Capacitor Quick Reference Guide from Digi-Key - Passives / Capacitors**](https://forum.digikey.com/t/capacitor-quick-reference-guide/12843)<br>
Quick reference guide for capacitors - Noah


## Projects & Tools

- [**Samsung/UTopia: UT based automated fuzz driver generation**](https://github.com/Samsung/UTopia)<br>

- [**FreeRTOS on CH32V307**](https://blog.imi.moe/freertos-on-ch32v307/)<br>
Porting FreeRTOS to a pretty unique RISC-V chip - Noah.

- [**labgrid-project/labgrid: embedded systems control library for development, testing and installation**](https://github.com/labgrid-project/labgrid)<br>
This seems interesting for setting automated hardware testing. I like that there's a `pytest` plugin - Eric.

- [**GNU poke**](https://jemarch.net/poke)<br>
A TUI to edit binary files, with support for structured files. Neat! More of a discussion on [HackerNews](https://news.ycombinator.com/item?id=34986042).

- [**Serial Monitor - Visual Studio Marketplace**](https://marketplace.visualstudio.com/items?itemName=ms-vscode.vscode-serial-monitor)<br>
A nice VS Code extension for getting a serial console with a device - Eric

- [**hawkw/tinymetrics: a minimal, allocation-free Prometheus/OpenMetrics metrics implementation for `no-std` and embedded Rust.**](https://github.com/hawkw/tinymetrics)<br>
A minimal version of generating metrics on embedded devices using Rust and OpenTelemetry - Tyler.

- [**PotatoP - Hackaday.io**](https://hackaday.io/project/184340-potatop)<br>
A microcontroller-based Lisp computer with very low power consumption. Nice hardware design!

- [**Isysxp/Pico_1140: A PDP11/40 emulator that will run Unix v5/6**](https://github.com/Isysxp/Pico_1140)<br>
Nice combination of software components for a PDP11/40 emulator running on an RP2040

- [**Wren6991/OpenDAP**](https://github.com/Wren6991/OpenDAP)<br>
This is cool- an Arm DAP implementation designed to interface RISC-V cores. Enables using SWD probes with a RISC-V implementation. A work-in-progress, but very neat! - Noah.

- [**mborgerson/gdbstub: Simple, single-file, dependency-free GDB stub that can be easily dropped in to your project.**](https://github.com/mborgerson/gdbstub)<br>
An example single-file embeddable gdbstub, enables adding built-in gdb target support to an application

- [**plasma-umass/ChatDBG: ChatDBG - AI-assisted debugging. Uses AI to answer 'why'**](https://github.com/plasma-umass/chatdbg)<br>
ChatGPT meats GDB. It's awesome!

## News

- [**Release Zephyr 3.3.0**](https://github.com/zephyrproject-rtos/zephyr/releases/tag/v3.3.0)<br>
Zephyr 3.3.0 is out 🎉! Support added for fuel gauges (battery monitoring), USB-C, CMSIS-DSP, and picolibc, among other changes

- [**Version 7.0.0 Released - KiCad EDA**](https://www.kicad.org/blog/2023/02/Version-7.0.0-Released/)<br>
Kicad 7 is here! With tons of great features: kicad-cli, pack-and-move, and routing enhancements. The release highlights are well worth a read!

- [**Raspberry Pi Pico W gets Bluetooth support in SDK 1.5.0 - CNX Software**](https://www.cnx-software.com/2023/02/11/raspberry-pi-pico-w-bluetooth-le-support/)<br>
Raspberry Pi Pico W now supports dual-mode Bluetooth - Noah.

- [**NIST Selects ‘Lightweight Cryptography’ Algorithms to Protect Small Devices - NIST**](https://www.nist.gov/news-events/news/2023/02/nist-selects-lightweight-cryptography-algorithms-protect-small-devices)<br>
The National Institute of Standards and Technology (NIST) is narrowing down which cryptographic algorithms should be used by IoT devices.
