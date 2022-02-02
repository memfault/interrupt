---
title: "What we've been reading in July and August (2021)"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

> You may have noticed there was no July roundup. We've been busy improving [Memfault](https://memfault.com), building out our partnerships with [Nordic Semiconductor](https://memfault.com/news/monitoring-debugging-and-updating-nordic-powered-iot-products-made-easy-with-device-observability-platform-partnership/) and [Laird](https://memfault.com/news/laird-connectivity-partners-with-memfault-to-offer-device-observability-platform-for-its-connected-low-power-cellular-bluetooth-product-line/), and enjoying the summer months. Thanks for understanding.

Here are the articles, videos, and tools that we've been excited about this
July and August.

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).

## Articles & Learning

- [CMake Part 3 - Source File Organisation - Sticky Bits](https://blog.feabhas.com/2021/08/cmake-part-3-source-file-organisation/) by Martin Bond<br>
  A great introduction series to using CMake to cross-compile a firmware for an STM32F4 MCU. This post covers how to structure source files and build them into object and static files. Be sure to check out Part 1 and Part 2!
- [Rust on Espressif chips](https://mabez.dev/blog/posts/esp-rust-espressif/) by Scott Mabin<br>Scott recounts his experiences of working with ESP32 chips and Rust. There's a [ESP-IDF hello world Rust application](https://github.com/ivmarkov/rust-esp32-std-hello) that connects to Wi-Fi, has a GUI, and using many of the ESP-IDF standard library functions, all using Rust. I also learned about [esp-rs](https://github.com/esp-rs), a project to help bring Rust to the Espressif ecosystem.
- [Exploring Clang/LLVM optimization on programming horror](https://blog.matthieud.me/2020/exploring-clang-llvm-optimization-on-programming-horror/) by Matthieu<br>Matthieu dives into the deep end and investigates how LLVM (and GCC fails to) optimize a simple looping function into a constant time algorithm that is even shorter in assembly than in C(++).
- [Resolving I2C Address Conflicts](https://embeddedartistry.com/blog/2021/08/02/resolving-i2c-address-conflicts/) by Phillip Johnston<br>
  I2C address conflicts occur when two parts communicate over I2C using the same address. Phillip provides a simple and succinct list of options for how to work around them.
- [From Stolen Laptop to Inside the Company Network | Dolos Group](https://dolosgroup.io/blog/2021/7/9/from-stolen-laptop-to-inside-the-company-network)<br>
  The bit I learned from this article is that you can listen over SPI and capture a Bitlocker decryption key using the Saleae Logic analyzer, [bitlocker-spi-toolkit](https://github.com/FSecureLABS/bitlocker-spi-toolkit). Fascinating.
- [The Monkey Island PC-Speaker music player](https://www.thanassis.space/monkeyisland.html) by Athanasios Tsiodras<br>
  Athanasios encoded all the songs from Monkey Island using Huffman compression, stored them on an ATtiny85, and played them over a tiny speaker. A nice mixture of C Arduino code and a bunch of Pythons scripting!
- [RISC-V: A Baremetal Introduction Using C++](https://philmulholland.medium.com/modern-c-for-bare-metal-risc-v-zero-to-blink-part-1-intro-def46973cbe7) by Phil Mulholland<br>
  An 8 part series on Modern C++ RISC-V bare-metal programming.
- [WebSerial - Read from and write to a serial port](https://web.dev/serial/) by François Beaufort<br>
  A series of code snippets explaining how to use the browser's WebSerial API to read and write to and from a device's serial port (think UART). I believe there is huge potential in interfacing with devices using WebUSB and WebSerial, especially around automated testing and debugging. Reach out if you've done anything you think is fun!
- [Reverse Engineering WiFi on RISC-V BL602](https://lupyuen.github.io/articles/wifi) by Lee Lup Yuen<br>
  The Bouffalo Labs BL602 comes pre-loaded with a closed-source Wi-Fi driver. In this novel of a post, Lee Lup Yuen reverse engineers the BL602 Wi-Fi driver to confirm it's performing as expected. It turns out it contains a lot of open-source code which can be found in various projects, such as AliOS, the Rockchip RK3399 Wi-Fi driver, LWIP, mbedTLS, and FreeRTOS.
- [Semantic Versioning on Github | Stratify Labs](https://blog.stratifylabs.co/device/2021-07-12-Semantic-Versioning-and-Github)<br>
  A best-practices article on how to branch and version for firmware using Git. It describes cherry-picking, semantic versions, and changelogs.
- [What Every C Programmer Should Know About Undefined Behavior #1/3 | The LLVM Project Blog](https://blog.llvm.org/2011/05/what-every-c-programmer-should-know.html) by Chris Lattner<br>
  Chris goes over various ways to cause an LLVM-compiled program to crash and produce undefined behavior, such as integer overflow, dereferencing a null pointer, etc. I believe it's good to know what _not_ to do as well as _what to do_.
- [Embedded Cross-Compiled Test Driven Development with CGull | lack of focus](https://www.louissimons.com/embedded-cross-compiled-tdd) by Lou Simons<br>
  One of the annoying parts of embedded unit testing is that the libraries and frameworks usually don't come with a "runner". You have to craft your own using Make, CMake, or use Ceedling (but it's Ruby, which I don't love). Lou talks about how to test code using Unity and CGull as the runner.

## Tools & Projects

- [martin-ger/esp32_nat_router: ESP32 NAT Router | GitHub](https://github.com/martin-ger/esp32_nat_router)<br>
  The projects people build on ESP32's are getting out of hand 😜. I had never heard of someone building a router on such a limited device before this.
- [reproducible-builds/disorderfs - FUSE filesystem that introduces non-determinism](https://salsa.debian.org/reproducible-builds/disorderfs)<br>
  The [Reproducible Builds](https://reproducible-builds.org/) group is a heavy advocate of exactly that. This project hosts your source files in a FUSE-compatible file system and randomizes the order in which they are returned to the compiler to ensure that you aren't accidentally relying on the filesystem layout for reproducible builds. I love the creativity here.
- [walmis/blackmagic-espidf: Blackmagic Wireless SWD Debug probe hosted on ESP-IDF SDK | GitHub](https://github.com/walmis/blackmagic-espidf)<br>
  With this project, you can flash your ESP8266, debug it, and retrieve logs over Wi-Fi and GDB.
- [Manawyrm/fxIP: fxIP - TCP/IP for Casio fx-9860 graphical calculators](https://github.com/Manawyrm/fxIP)<br>For a brief moment, this website was hosted on a Casio graphic calculator using this webserver. Since then, the webserver has since been taken down but it's now on the [Web Archive](http://web.archive.org/web/20210712193555/http://fxip.as203478.net/).
- [superlou/cgull: Minimal tool for testing embedded C using Python and Unity](https://github.com/superlou/cgull)<br>
  Mentioned above in the articles section, but placed this here as well because I really like it!

## Random

- [C Programming - The State of Developer Ecosystem in 2021 Infographic | JetBrains](https://www.jetbrains.com/lp/devecosystem-2021/c/)<br>
  A fun survey of developers done by the JetBrains team about favorite languages, tools, compilers, and package managers. I've linked the C survey, but check the others out!
- [WE ARE UPDATING YOUR SHOES | Twitter](https://twitter.com/avagliano_alex/status/1387095139508703233)<br>
  Are you all ready for your shoes to have firmware updates? Because it's happening. First with Nike. Now with Under Armour.
- [ZeroVer: 0-based Versioning](https://0ver.org/)<br>
  This is too funny but all too real. Some projects have been around for _years_ and will likely never increment to version 1.0.
- [Remotely debug, monitor, and update Nordic IoT devices with Memfault | Nordic Semiconductor](https://webinars.nordicsemi.com/remotely-debug-monitor-and-update-5)<br>
  Nordic's MCU devices can now integrate with Memfault out of the box. This webinar covers how to remotely debug, monitor, and ship OTA updates to Nordic-powered devices.
- [Remote Debugging & Device Observability: How Memfault & Diamond Kinetics worked together to fix firmware bugs | Memfault](https://memfault.com/webinars/remote-debugging-device-observability-how-memfault-diamond-kinetics-worked-together-to-fix-firmware-bugs/)<br>
  Memfault's François Baldassari and Diamond Kinetic's CTO, Mike Ressler, talk about how Diamond Kinetic improved their monitoring using Memfault.
