---
title: "What we've been reading in July (2020)"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

Here are the articles, videos, and tools that we've been excited about this
July.

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments.

## Articles & Learning

- [Hands-On RTOS with Microcontrollers (book)](https://www.packtpub.com/cloud-networking/hands-on-rtos-with-microcontrollers) by Brian Amos<br>A new (relatively) book for the embedded ecosystem. The table of contents looks thorough and uses modern software and tools such as an STM32, FreeRTOS, and a JLink.
- [Build PineTime Firmware in the Cloud with GitHub Actions](https://lupyuen.github.io/pinetime-rust-mynewt/articles/cloud) by LEE Lup Yuen<br>Step-by-step post documenting the process to build the PineTime firmware using Github Actions, complete with screenshots, code snippets, and configuration files. Applicable to any firmware project.
- [Boost your beautiful log messages with instant crash analysis](http://www.cyrilfougeray.com/2020/07/27/firmware-logs-with-stack-trace.html) by Cyril Fougeray<br>Using just an nRF52 a UART, and [CrashCatcher](https://github.com/adamgreen/CrashCatcher), Cyril writes a script using the Python serial library, [pySerial](https://pythonhosted.org/pyserial/), to automatically generate a stack trace whenever a device crashes.
- [Espressif ESP32: Bypassing Secure Boot using EMFI](https://raelize.com/posts/espressif-systems-esp32-bypassing-sb-using-emfi/) by [Realize](https://raelize.com/)<br>A post (and soon-to-be-series of posts) on how the team at Realize was able to bypass the ESP32 Secure Boot and Flash Decryption on earlier revisions of the boards.
- [Exploiting the Preprocessor for Fun and Profit](https://embeddedartistry.com/blog/2020/07/27/exploiting-the-preprocessor-for-fun-and-profit/) by [Klemens Morgenstern](https://klemens.dev/)<br>A standard but complete "preprocessor fun" post.
- [Why I prefer C++: “RAII All The Things!”](https://covemountainsoftware.com/2019/11/26/why-i-prefer-c-raii-all-the-things/) by Matthew Eshleman<br>A post about C++ and its ability to follow the RAII (Resource Acquisition Is Initialization) idiom. Don't forget that you can do this in C with GCC's [`cleanup` attribute](https://jooojub.github.io/jooojub/posts/gcc/attribute_cleanup/2019-06-16-gcc-attribute-cleanup/).
- [Debugging Arm-based microcontrollers in (Neo)vim with gdb](https://chmanie.com/post/2020/07/18/debugging-arm-based-microcontrollers-in-neovim-with-gdb/) by Christian Maniewski<br>I personally love tools, and I know everyone has questions about how to set up their VSCode/Vim/IDE editor with GCC/Clang and GDB/Jlink/Openocd. A straight-forward post on setting up NeoVim with GDB for embedded development.
- [Concurrency Patterns in Embedded Rust](https://ferrous-systems.com/blog/embedded-concurrency-patterns/) by Ferrous Systems<br>A post detailing how to handle concurrency in a Rust `no_std` application. Covers threads, interrupts, and async/await.
- [A tale of Ghosts'n Goblins'n Crocodiles](https://fabiensanglard.net/cpc/index.html) by Fabian Sanglard<br>Old video game systems are like embedded systems, right? A story about the game Ghosts'n Goblins (Capcom, 1985) and its history on the Amstrad CPC, an 8-bit French computer system.
- [The tiniest C sort function?](https://www.cs.dartmouth.edu/~doug/tinysort.html)<br> The title should pique your interest enough.
- [The Anatomy of a Race Condition](https://covemountainsoftware.com/2020/06/21/the-anatomy-of-a-race-condition/) by Matthew Eshleman<br>A walk-through how to code review and debug a race condition.
- [CAN Priority Inversion](https://kentindell.github.io/2020/06/29/can-priority-inversion/) by Dr. Ken Tindell<br>An in-depth article on how the common "Priority Inversion" problem using CAN and Zephyr to demonstrate the problem.
- [Apple Lightning](https://nyansatan.github.io/lightning/) - Not quite the normal Interrupt material, but a fascinating repository of everything a few individuals could gather about Apple's Lightning cables.

## Neat Open Source Projects

- [Nest Labs Embedded Libraries](https://github.com/nestlabs)<br>We found some great C/C++ libraries in the Nest Labs Github organization, including an assert library, unit testing library, FSM generator, and a custom RTOS.
- [adamgreen/CrashCatcher](https://github.com/adamgreen/CrashCatcher)<br>A library that is inserted into the fault handlers of an ARM Cortex-M firmware. Will generate crash dumps (or core dumps) for post-mortem debugging. Similar to what Memfault provides, but the repo dates back to 2014! A gem of a project.
- [adamgreen/mri - Monitor for Remote Inspection](https://github.com/adamgreen/mri)<br>Enables GDB to debug Cortex-M devices over a UART connection using the Debug Monitor. Wondering how it works? Check out our recent post [on the topic]({% post_url 2020-07-29-cortex-m-debug-monitor %}).
- [ZMK Firmware](https://zmkfirmware.dev/)<br>A Zephyr-based firmware for keyboards. This project is in its early days, but if you are looking to learn more about Zephyr, this is a great reference point.
- [ficl - Forth Inspired Command Language](http://ficl.sourceforge.net/)<br>Ficl is a programming language interpreter designed to be embedded into other systems as a command, macro, and development prototyping language. We stumbled across this because you can actually [compile it into NuttX](https://cwiki.apache.org/confluence/display/NUTTX/Configuration+Variables#CONFIG_INTERPRETERS_FICL)! 
- [cjhdev/wic  - WebSockets in C](https://github.com/cjhdev/wic)<br>A fresh project that implements WebSockets in C99 C code with embedded systems in mind (it doesn't use malloc!).
- [ESP-Hosted](https://github.com/espressif/esp-hosted) by Espressif<br>This project allows you to hook up an ESP32 to a host machine, such as a Linux device, and use TCP/IP and Bluetooth HCI protocol as normal but all communications are passed through to the ESP32.
- [M2OS](https://m2os.unican.es)<br>M2OS is an RTOS written in Ada and released this March 1st, 2020. It can run on an Arduino Uno and an STM32F4. Ada is a language I did not expect to see in a modern RTOS.
- [TinyGo](https://github.com/tinygo-org/tinygo)<br>We've linked TinyGo before, but their recent changelogs are impressive. RISC-V, Teensy 3.6, Particle boards, and Go Module support. I also just realized they support WebAssembly as a target. Neat.

## News

- [Nvidia in Advanced Talks to Buy SoftBank’s Chip Company Arm](https://www.bloomberg.com/news/articles/2020-07-31/nvidia-said-in-advanced-talks-to-buy-softbank-s-chip-company-arm)<br>I'm sure you've heard the news, but it has to be included here. What will happen if Arm is purchased, made public, or goes down with Softbank? No one knows.
- [Project Connected Home over IP (December 2019)](https://zigbeealliance.org/news_and_articles/connectedhomeIP/) - We missed this news, but this is a large project between Amazon, Apple, Google, and Zigbee (and more) to develop a standard for smart home device connectivity. Follow along in the Github Repo at [project-chip/connectedhomeip](https://github.com/project-chip/connectedhomeip).
