---
title: "What we've been reading in October (2021)"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

Here are the articles, videos, and tools that we've been excited about this
October.

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).

## Articles & Learning

- [#390 - Irresponsible at the Time with Tyler Hoffman (Podcast) - Embedded.fm ](https://embedded.fm/episodes/390)<br>
  Elecia White, Chris White, and I talk about the issues of building and shipping embedded devices at scale. We talk about how the hardest part is not building the product, but supporting and monitoring the product.

- [Best practices for debugging Zephyr-based IoT applications - Embedded.com](https://www.embedded.com/best-practices-for-debugging-zephyr-based-iot-applications/) by Luka Mustafa and Chris Coleman<br>
  How to debug Zephyr applications using standard methods and Memfault, written by Luka from [IRNAS](https://www.irnas.eu/).

- [When Updating an Untested Module, Take the Opportunity to Add Tests](https://embeddedartistry.com/blog/2021/10/25/when-updating-an-untested-module-take-the-opportunity-to-add-tests/) by Phillip Johnston<br>
No one likes to try and update a brittle and untested module in a code base. But when it _needs_ to be updated, this is usually the best time to add unit tests! As a developer who has had to do this plenty of times, if you don't add tests, it will become _your_ module if you update it without adding tests. So do it. Listen to Phillip.

- [The Problem with malloc() - Stratify Labs](https://blog.stratifylabs.co/device/2021-10-28-The-Problem-with-malloc/)<br>
  A nice post describing how a basic heap is implemented and general advice on whether or not using a heap and `malloc()` is a good idea for embedded firmware. At the end of the day, _it depends_.

- [So you want to live-reload Rust - Faster Than Lime](https://fasterthanli.me/articles/so-you-want-to-live-reload-rust) by Amos<br>
  If you have 48 hours or more of free time, Amos can teach you how to reload a dylib live in a Rust program.

- [Announcing ESP32-H2, an IEEE 802.15.4 + Bluetooth 5 (LE) RISC-V SoC - Espressif Systems](https://www.espressif.com/en/news/ESP32_H2)<br>
  RISC-V, Bluetooth 5, and IEEE 802.15.4. This is exciting!

- [Rust on Espressif chips (continued)](https://mabez.dev/blog/posts/esp-rust-18-10-2021/) by Scott Mabin<br>
  Scott works at Espressif, and in this post, he announces that they'll keep [a book](https://esp-rs.github.io/book/) updated on how to get started with Espressif chips and Rust. It includes a [roadmap](https://github.com/orgs/esp-rs/projects/1)! I love it.

- [Bluetooth Development Like a Pro - Adding a Portable CLI to Your Firmware - by Ovyl](https://ovyl.io/blog-posts/bluetooth-development-like-a-pro-adding-a-portable-cli-to-your-firmware) by Nick Sinas<br>
  Nick talks about building a CLI for their nRF52-based product.

- [Building an IoT Product — The Product(ion) Feedback Loop - tado°](https://medium.com/tado-product-development-blog/building-an-iot-product-the-product-production-feedback-loop-c040e87c8a59) by Matthias Bösl<br>
  Matthias talks about collecting data during the manufacturing process of a consumer hardware product and using it to make decisions. I've been enjoying the articles coming out of this blog!

- [c - Bubble sort slower with -O3 than -O2 with GCC - Stack Overflow](https://stackoverflow.com/questions/69503317/bubble-sort-slower-with-o3-than-o2-with-gcc)<br>
  One of those gems you find on Stack Overflow.

- [Implementing Hash Tables in C](https://www.andreinc.net/2021/10/02/implementing-hash-tables-in-c-part-1) by
  Andrei Ciobanu<br>
  A nice and clean post on building a hash table in C. I enjoy a good post on data structures in C.

- [A tale of two toolchains and glibc](https://www.collabora.com/news-and-blog/blog/2021/09/30/a-tale-of-two-toolchains-and-glibc/) by Adrian Ratiu<br>
  This article talks about having a code base support two compilers, LLVM/Clang and GCC.

- [Tiny ELF Files: Revisited in 2021](https://nathanotterness.com/2021/10/tiny_elf_modernized.html) by Nathan Otterness<br>
  Nathan wanted to learn what it took to make a tiny ELF program that ran on Linux, so he gave it a whirl. At one point, he used a spreadsheet to map out all the instructions used 😛.

- ["Static Linking Considered Harmful" Considered Harmful - Gavin D. Howard](https://gavinhoward.com/2021/10/static-linking-considered-harmful-considered-harmful/) by Gavin Howard<br>
  Gavin covers in detail why he believes static linking is far superior to dynamic linking. Less related to embedded, but a fun read nonetheless.

- [The Fascinating Reason Why The Garmin FR945 & Fenix 6 No Longer Shows Pool Temperature - DC Rainmaker](https://www.dcrainmaker.com/2021/10/the-fascinating-reason-why-the-garmin-fr945-fenix-6-no-longer-shows-pool-temperature.html)<br>
  Garmin recently published an update to one of their watches which disables the water temperature sensor during _indoor swims only_. Why? _we have discovered that having the barometer-thermometer (a combined electronic part) powered on during exposure to chlorine dramatically accelerates the failure of the part_. I love when a software update fixes hardware issues.

- [The LLVM Project Blog](https://blog.llvm.org/posts/2021-10-01-generating-relocatable-code-for-arm-processors/) by Pavel Loktev<br>
  This post covers the problem where you load similarly compiled firmware into different regions of ROM but want to be able to execute from either of them (such as in A/B partitioning schemes for firmware updates). Pavel talks about how to solve this problem using LLVM.

- [Unexpectedly low floating-point performance in C - ESP32 Forum](https://www.esp32.com/viewtopic.php?f=14&t=800)<br>
  A fun 5-year-long forum thread on floating-point performance in C on an ESP32, complete with measurements, sample code, and more.

## Tools & Projects

- [madler/crcany: Compute any CRC, a bit at a time, a byte at a time, and a word at a time.](https://github.com/madler/crcany)<br>
  A generic CRC library in C by the infamous Mark Adler, creator of zlib and gzip.

- [atomicobject/heatshrink: data compression library for embedded/real-time systems](https://github.com/atomicobject/heatshrink)<br>
  A small compression library based on LZSS and claims to function well using 100-500 bytes of memory. Slick!

- [lvgl/lv_sim_vscode_sdl](https://github.com/lvgl/lv_sim_vscode_sdl)<br>
  The best way to test your embedded code is probably to not test it on your embedded system, but rather on a host machine or in a simulator/emulator. I learned this week that LVGL has this simulator, and it's super easy to work with!

- [rokath/trice: fast and tiny embedded device C printf-like trace code and real-time PC logging (trace ID visualization)](https://github.com/rokath/trice)<br>
  A clever tracing library implemented in Go. It mentions the ARM Cortex-M0+ in the docs, so it should be able to be compiled against Cortex-M chips and used.

- [ssciwr/clang-format-wheel: clang-format python wheels](https://github.com/ssciwr/clang-format-wheel)<br>
  Want to install clang-format on your teammates' machines but don't use Conda or Docker? Include this PyPi package in your Python virtual environment and you're off and running.

- [cesanta/elk: A low footprint JavaScript engine for embedded systems](https://github.com/cesanta/elk)<br>
  Yet another Javascript engine for embedded devices.

- [V-USB - A Firmware-Only USB Driver for Atmel AVR Microcontrollers](https://www.obdev.at/products/vusb/index.html)<br>
  Title says it all.

## Random

- [Arm announces Centauri](https://www.arm.com/solutions/iot/project-centauri)<br>
  Arm has announced Centauri, a project(?) that will encompass how to abstract away hardware and some software differences and provide a blueprint for security and software libraries. At least I think. I'm patiently waiting to look into this more.

- [GNU Arm Embedded Toolchain Downloads – Arm Developer](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)<br>
  It's always a good day when Arm releases an updated version of the GNU Arm Embedded Toolchain. This time, it's still 10.3 but it patches a [recent security vulnerability](https://developer.arm.com/support/arm-security-updates/vlldm-instruction-security-vulnerability).

- [Sensor Watch - Crowd Supply](https://www.crowdsupply.com/oddly-specific-objects/sensor-watch)<br>
  A fun Arm Cortex-M0+ upgrade for a Casio watch.
