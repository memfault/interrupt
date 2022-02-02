---
title: "What we've been reading in November (2021)"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

Here are the articles, videos, and tools that we've been excited about this
November.

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).

## Articles & Learning

- [How to monitor, debug & update your Embedded Android devices Webinar](https://go.memfault.com/how-to-monitor-debug-update-your-android-os-devices-webinar) by Ryan Case<br>
  Memfault put on a webinar last month covering all the tools and infrastructure you'll likely need to monitor, debug, and update embedded Android devices in the field. If you're working in the Android ecosystem, it's definitely worth the watch.
- [5 Reasons Why I Dread Writing Embedded GUIs - Benjamin Cabé](https://blog.benjamin-cabe.com/2021/10/21/5-reasons-why-i-dread-writing-embedded-guis) by Benjamin Cabé<br>
  Writing GUI's in firmware is usually painful, and Benjamin agrees. I had to write a few user interfaces in the Pebble firmware during the 3.0 rewrite, and although it was fun and rewarding, it took way longer than it should have due to the lack of tooling as mentioned in the article. <br>
  Want your mind truly blown? Go watch [Heiko Behrens](https://www.youtube.com/watch?v=UC-ru1P3GVo) present his "demo" at Evoke 2017 on an OG Pebble.
- [Porting Doom to an nRF52840-based USB Bluetooth-LE Dongle – next-hack.com](https://next-hack.com/index.php/2021/11/13/porting-doom-to-an-nrf52840-based-usb-bluetooth-le-dongle/)<br>
  Continuing the graphics theme, this novel-post details the efforts taken to get Doom to run on a USB dongle based on a Nordic nRF52840. If you have a spare hour, read this.
- [Hi, I'm working on GDBFrontend debugger, what features would you like to see? - Reddit /r/embedded](https://old.reddit.com/r/embedded/comments/qzx7ds/hi_im_working_on_gdbfrontend_debugger_what/)<br>
  A GDBFrontend developer was asking the /r/embedded community for feature request. Some good comments in here.
- [Ask HN: What are some great engineering blogs? - Hacker News](https://news.ycombinator.com/item?id=29237308#29237920)<br>
  Looking for a new engineering blog to follow? It's probably listed in here. Interrupt made it into the top comment! ❤️
- [Learning Computer Architecture Through a Game - Embedded Artistry](https://embeddedartistry.com/blog/2021/11/29/learning-computer-architecture-through-a-game/) by Phillip Johnston<br>
  Phillips talks about [Turing Complete](https://turingcomplete.game/), a game on Steam, which teaches from literally zero about computer science and how computers work on a fundamental level.
- [Using Singletons in Embedded C++ - Stratify Labs](https://blog.stratifylabs.dev/device/2021-11-29-Using-Singletons-in-embedded-cpp/) by Stratify Labs<br>
  A post on how to use Singletons to wrap and use hardware peripherals in C++.
- [Ada on any ARM Cortex-M device, in just a couple… - The AdaCore Blog](https://blog.adacore.com/ada-on-any-arm-cortex-m-device-in-just-a-couple-minutes) by Fabien Chouteau<br>
  If you've been itching to run Ada on your devices, it sounds like now is a good time. They are working diligently on getting it to run on Cortex-M devices, as evidenced by this blog post and another one - [An Embedded USB Device stack in Ada](https://blog.adacore.com/an-embedded-usb-device-stack-in-ada)
- [Up and Running with ZephyrRTOS on Conexio Stratus IoT Kit - Hackster.io](https://www.hackster.io/piyareraj/up-and-running-with-zephyrrtos-on-conexio-stratus-iot-kit-4661a3)<br>
  A getting started post using Conexio and ZephyrRTOS. I don't have personal experience with Conexio, but they are working on integrating Memfault into their platform, which is a good sign in my book.
- [A survey of concurrency bugs - Cove Mountain Software](https://covemountainsoftware.com/2021/03/01/a-survey-of-concurrency-bugs/) by Matthew Eshleman<br>
  A post on common embedded software concurrency bugs and their mitigations.
- [Real Time For the Masses goes multi-core - Embedded in Rust](https://blog.japaric.io/multicore-rtfm/) by Jorge Aparicio<br>
  RTFM, or Real Time For the Masses, which is a concurrency library for Rust tailed for embedded devices, has been updated to support Cortex-M multi-core chips. This post covers the new API and details how you can get started.
- [GPIO handling in ARM Cortex-M - Sergio R. Caprile](http://www.scaprile.com/2021/10/28/gpio-handling-in-arm-cortex-m/) by Sergio R. Caprile<br>
  What actually happens when you set a GPIO port.
- [Reducing an LTO Linux kernel bug with cvise - Nathan Chancellor](https://nathanchance.dev/posts/cvise-lto-kernel-bug/) by Nathan Chancellor<br>
  Such a fun post. There's an art to reporting bugs, and that usually involves coming up with the absolute minimal reproducible issue that the maintainer can replay on their own machine. This is what Nathan did with a compiler bug related to LTO. During this post, I learned about [C-Reduce](https://embed.cs.utah.edu/creduce/).
- [A Close Look at a Spinlock – Embedded in Academia](https://blog.regehr.org/archives/2173) by John Regehr<br>
  A short and sweet post about spinlocks, a concurrency primitive.
- [I2C Communication Protocol: Understanding I2C Primer PMBus and SMBus - Analog Devices](https://www.analog.com/en/analog-dialogue/articles/i2c-communication-protocol-understanding-i2c-primer-pmbus-and-smbus.html) by Mary Grace Legaspi and Eric Peňa<br>
  A wonderful overview of I2C, maybe the best I've found online.
- [Learning Rust For Embedded Systems - EmbeddedRelated.com](https://www.embeddedrelated.com/showarticle/1432.php) by Steve Branam<br>
  Steve was told to go evaluate Rust for an embedded project and here tells about his journey. The post contains a wealth of resources and links to various books, pages, and posts about Embedded Rust. His answer is, use Rust!

## Conference Content

- [On Hubris and Humility: developing an OS for robustness in Rust - Open Source Firmware Conference 2021](https://talks.osfc.io/osfc2021/talk/JTWYEH/)<br>
  The team at [Oxide Computer](https://oxide.computer/) gave a talk on their new embedded RTOS written in Rust, Hubris, and debugging setup, Humility. This is all incredibly exciting and I can't wait to see how it progresses.
- [2021 IoT Online Conference](https://www.iotonlineconference.com/)<br>
  The 2021 edition of the IoT Online Conference kicks off on December 8th.
- [NoHat 21 - LimitedResults](https://limitedresults.com/2021/11/nohat-21/)<br>
  LimitedResults, known for exploiting the nRF52 and ESP32 series of chips, is at it again, this time with Silicon Labs EFM32 Gecko chips. He details the exploit which enables the reactivation of the ARM SWD debug port after being explicitly and "permanently" disabled by the Authentication Access Port, or AAP.

## Tools & Projects

- [elfshaker/elfshaker: elfshaker stores binary objects efficiently](https://github.com/elfshaker/elfshaker)<br>
  An incredibly neat tool that takes many binary files and compresses them effeciently in a custom format. Since binary ELF files don't change often with rebuilds, and because normal compression like gzip doesn't work well on binaries, this tool can produce amazing results. They claim a 0.01% compression ratio when it's snapshotting many binaries at once! To prove their point, they also have [elfshaker/manyclangs](https://github.com/elfshaker/manyclangs), which is a GitHub repo of all LLVM builds ever, stored in 100MB. Impressive!
- [nadavrot/memset_benchmark](https://github.com/nadavrot/memset_benchmark)<br>
  `memcpy` and `memset` implements which outperform the folly and glibc versions.
- [Dirbaio/rzcobs](https://github.com/dirbaio/rzcobs)<br>
  A spin on [rCOBS](https://github.com/Dirbaio/rcobs) that can compress data a bit further if it's known to have a lot of `0x00` bytes.
- [silvergasp/bazel-embedded: Tools for embedded/bare-metal development using bazel](https://github.com/silvergasp/bazel-embedded)<br>
  A from-zero setup based on the [Bazel](https://bazel.build/) build system that is tailored for embedded systems and development.
- [q3k/m16c-interface: A Serial IO programmer for Renesas M16C, includes security PIN bypass](https://github.com/q3k/m16c-interface)<br>
  A quick-and-dirty tool to load and dump the flash contents of a Renesas M16C.

## Random

- [Redis storage · ccache/ccache Wiki](https://github.com/ccache/ccache/wiki/Redis-storage)<br>
  At a previous employer, we always wanted to set up a distributed [ccache](https://ccache.dev/) cache. Unfortunately, it was always too complicated for us firmware engineers. I was surprised to find out that you can just use Redis (ideally hosted within a company VPC) and you can be off and running! Amazing.
- [Renesas introduces sub 50 cents FPGA family with free Yosys-based development tools - CNX Software](https://www.cnx-software.com/2021/11/22/renesas-50-cents-fpga-forgefpga-yosys-development-tools/?amp=1)
- [Nuking most of my .vimrc and just using LunarVim](https://fnune.com/2021/11/20/nuking-most-of-my-vimrc-and-just-using-lunarvim/) by Fausto Núñez Alberro<br>
  I had never heard of LunarVim, so I wanted to share this post. Fausto is an employee at Memfault. [Come join him and us. We're hiring!](https://jobs.lever.co/memfault)
- [RTOS revolution: SEGGER embOS-Ultra enhances application performance with Cycle-resolution Timing](https://www.segger.com/news/rtos-revolution-segger-embos-ultra-enhances-application-performance-with-cycle-resolution-timing/)<br>
  SEGGER released a new version of embOS which uses a tickless operation by default, which should lower power consumption.
- [STMP157-OLinuXino-LIME2 Open Source Hardware Industrial Grade Linux computer is in mass production with 4 variants - Olimex](https://olimex.wordpress.com/2021/10/29/stmp157-olinuxino-lime2-open-source-hardware-industrial-grade-linux-computer-is-in-mass-production-with-4-variants/)
- [badssl.com](https://badssl.com/)<br>
  Need to test whether your embedded software can cope with various SSL configurations and misconfigurations. Point it at badssl.com and choose your own adventure.
- [https://httpbin.org/](https://httpbin.org/)<br>
  Similar to the above, if you need to test your embedded software HTTP stack and various return values, encodings, and request types, httpbin is your friend!
