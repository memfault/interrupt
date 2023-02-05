---
title: "What we've been reading in August (2022)"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

Here are the articles, videos, and tools that we've been excited about this
August

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).

## Articles & Learning

- [**Webinar: How to Build and Maintain IoT Management Systems for Scale - Memfault**](https://hubs.la/Q01kh_bT0)<br>
  Chris and I are summarizing a number of things hardware companies need to think about and address before they start trying to ship thousands of devices to end customers. Trust us, it can get hard quickly, and a company's success in shipping might make fighting fires and debugging that much more difficult. Tune in! - Tyler

- [**Virtual Panel: Debugging Embedded Devices in Production - Memfault**](https://hubs.la/Q01kj0GW0)<br>
  I had a blast talking to Phillip Johnston from Embedded Artistry and Alvaro Prieto from Sofar Ocean about how we debug hardware and firmware in production. Alvaro has some great stories!

- [**Debugging with GDB Introduction - Azeria Labs**](https://azeria-labs.com/debugging-with-gdb-introduction/)<br>
A nice tutorial on debugging with GDB on ARM and [gef](https://github.com/hugsy/gef). - Francois

- [**Building a Panel out of e-ink Electronic Shelf Labels**](https://rbaron.net/blog/2022/07/29/Daisy-chaining-multiple-electronic-shelf-labels.html) by rbaron<br>
A cool reverse engineering task to build something new out of discarded e-ink supermarket shelf labels - Francois.

- [**WCH CH569 RISC-V SoC Offers USB 3.0, Gigabit Ethernet, High-Speed SERDES & HSPI Interfaces - CNX Software**](https://www.cnx-software.com/2020/07/21/wch-ch569-risc-v-soc-offers-usb-3-0-gigabit-ethernet-high-speed-serdes-hspi-interfaces/) by Jean-Luc Aufranc<br>
Very interesting dev board with USB 3.0 and Gigabit Ethernet, based on a RISC-V SoC - Noah

- [**C99 doesn't need function bodies, or 'VLAs are Turing complete'**](https://lemon.rip/w/c99-vla-tricks/) by lemon<br>
An exercise stretching the C99 VLAs to the limits of their spec - Heiko.

- [**Phillip Johnston of Embedded Artistry on Engineering Approaches — Embedded.fm**](https://embedded.fm/episodes/423)<br>
Phillip Johnston joins Embedded.fm this time to talk about how engineer approaches change over time.

- [**Debugging bare-metal STM32 from the seventh level of hell - A Modicum of Fun**](https://jpieper.com/2022/08/05/debugging-bare-metal-stm32-from-the-seventh-level-of-hell/) by Josh Pieper<br>
Interesting journal of some severe heisen-debbugging session using techniques many programmers will hardly ever use. In the end, a reminder that programming against hardware is, well… hard - Heiko.

- [**The case against a C alternative - Handmade Network**](https://c3.handmade.network/blog/p/8486-the_case_against_a_c_alternative) by Christoffer Lernö<br>
I like this piece as a discussion point – all too often I see the hype around recent languages that claim to be the successor of C by looking only a singular aspect (memory handling, no surprises, etc). The author also reminded me of the “The Killer Feature” effect that once made me buy a Ruby book 😱 – I have yet to see the equivalent for Rust et al.

- [**kingyoPiyo/Pico-10BASE-T: 10BASE-T from Raspberry Pi Pico**](https://github.com/kingyoPiyo/Pico-10BASE-T)<br>
Cool hack and a nice README! - Noah

- [**Bringing Rust to Space - Setting up a Rust ecosystem for the VA108XX MCU family - robs blog**](https://robamu.github.io/post/rust-ecosystem/) by Robin Mueller<br>
A developer was trying to get Rust working on satellite-grade Vorago MCU's. It turns out there were no pre-existing libraries for such MCU's (duh) - Tyler.

- [**Using the i.MX RT600 Audio EVK as a Digital Audio Engineering Test Bed**](https://community.nxp.com/t5/Blogs/Using-the-i-MX-RT600-Audio-EVK-as-a-Digital-Audio-Engineering/ba-p/1514120)<br>
Write up about extending the RT685 EVK with a custom daughter card for use as an audio test system (one highlight: needing to bypass the level shifters for the application to work properly. level shifters are a classic point of failure!) - Noah

- [**Running GDB in the Browser**](https://blog.wokwi.com/running-gdb-in-the-browser/) by Uri Shaked<br>
Neat write-up about how wokwi got gdb to run in a web browser. - Noah

- [**Protobuffers Are Wrong :: Reasonably Polymorphic**](https://reasonablypolymorphic.com/blog/protos-are-wrong/) by Sandy Maguires<br>
Pretty harshly worded but interesting analysis of some disadvantages of Google's Protobuf data format - Noah

- [**How Golioth uses Hardware-in-the-Loop (HIL) Testing: Part 2 - Golioth**](https://blog.golioth.io/golioth-hil-testing-part2/) by Nick Miller<br>
Title says it all. A good post written by our friends at Goliath.

- [**Don't try this at home: overclocking RP2040 to 1GHz - Raspberry Pi**](https://www.raspberrypi.com/news/dont-try-this-at-home-overclocking-rp2040-to-1ghz/) by Liz Upton<br>
Wow! Raised the voltage from 1.1V to 3V and kept the temperature at -40°C. 

- [**Using GoogleTest and GoogleMock frameworks for embedded C - CodeProject**](https://www.codeproject.com/articles/1040972/using-googletest-and-googlemock-frameworks-for-emb) by Michael Pan<br>
Good reference on using the GoogleTest unit testing framework for embedded projects - Francois

- [**Regular Expression Matching Can Be Simple And Fast**](https://swtch.com/~rsc/regexp/regexp1.html) by Russ Cox<br>
Fascinating article looking at the performance characteristics in regex implementations - Noah

- [**Developing and testing Bluetooth Low Energy products on nRF52840 in Renode and Zephyr - Zephyr Project**](https://www.zephyrproject.org/developing-and-testing-bluetooth-low-energy-products-on-nrf52840-in-renode-and-zephyr/) by Michael Gielda<br>
Title says it all, some of our favorite technologies come together in this article - Noah.

- [**Your Makefiles are wrong**](https://tech.davis-hansson.com/p/make/) by Jacob Davis-Hansson<br>
A good set of recommendations for writing Makefiles - Francois.

- [**C++ Weekly - Ep 337 - C23 Features That Affect C++ Programmers**](https://www.youtube.com/watch?v=jOFrKN54M5g) with Jason Turner<br>
Video overview covering the new features of C23.

## Projects & Tools

- [**rokath/tcobs: COBS framing with implicit run-length-encoding, optimized for data containing statistically a bit more 0 and FF bytes in a row, as data often carry 16, 32 or 64 bit numbers with small values.**](https://github.com/rokath/tcobs) by Thomas Höhenleitner<br>
A chunking or framing protocol that extends upon the [COBS protocol](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing). This is from the same author as [TRICE](https://github.com/rokath/trice).

- [**Orbcode – Better Embedded**](https://orbcode.org/)<br>
Really nice looking combination debug probe/trace device, supports JTAG+SWD+SWO+Trace, works with PyOCD, OpenOCD. Lots of interesting guides + examples on the blog: https://orbcode.org/blog/ . Definitely keeping an eye on this project! - Noah

- [**verifast/verifast: Research prototype tool for modular formal verification of C and Java programs**](https://github.com/verifast/verifast)<br>
A slick formal verification framework for C and Java - Noah.

- [**INTI-CMNB/KiBot: KiCad automation utility**](https://github.com/INTI-CMNB/KiBot#usage-of-github-actions)<br>
A very useful set of utilities for automation / CI for KiCad EDA projects. I especially appreciate the sane-defaults GitHub actions plugin they provide 🙌. - Noah

- [**tobermory/faultHandling-cortex-m: Generating, exporting and analyzing CPU fault conditions on Arm Cortex-M series microcontrollers.**](https://github.com/tobermory/faultHandling-cortex-m)<br>
An Interrupt Slack user posted their library to decode fault registers on Arm Cortex-M. They are building devices that go down thousands of meters into the Pacific Ocean and need to know how they are crashing. Fun! - Tyler.

## Announcements & News

- [**C23 is Finished: Here is What is on the Menu - The Pasture**](https://thephd.dev/c23-is-coming-here-is-what-is-on-the-menu) by JeanHeyd Meneide<br>
Some seriously cool improvements coming in C23 - Francois. Heiko is most excited for the first iterations of `auto` and `constexpr`.

- [**Introducing multitasking to Arduino - Arduino Blog**](https://blog.arduino.cc/2022/08/02/introducing-multitasking-to-arduino/) by Alessandro Ranellucci<br>
Arduino is getting multitasking! The blog post itself has very little information, but you can find the primary discussion [in a GitHub discussion](https://github.com/arduino/language/discussions/2). 

- [**Nordic Semiconductor announces its first Wi-Fi chip, the dual-band Wi-Fi 6 nRF7002 - Nordic Semi**](https://www.nordicsemi.com/News/2022/08/Nordic-Semiconductor-announces-its-first-WiFi-chip)<br>
It's a "companion IC" so you don't write firmware for it, as it's meant to be used directly alongside one of Nordic's other devices to add Wi-Fi connectivity. 