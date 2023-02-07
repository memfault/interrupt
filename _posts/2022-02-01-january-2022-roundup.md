---
title: "What we've been reading in January (2022)"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

Here are the articles, videos, and tools that we've been excited about this
January.

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).

## RTL Jobs

[RTL Jobs](https://www.rtljobs.com) is a new job site for people who specialize in FPGA and RTL design created by [Nash Reilly](https://interrupt.memfault.com/authors/nash/). Follow the job site on Twitter at [@RTLjobs](https://twitter.com/RTLjobs).

## Articles & Learning

- [**Library order in static linking - Eli Bendersky's Website**](https://eli.thegreenplace.net/2013/07/09/library-order-in-static-linking) by Eli Bendersky <br>
This remains one of our favorite articles among us firmware engineers at Memfault. It details the order in which the linker will pull objects into the final firmware. - Tyler

- [**Pay attention to WebAssembly - Harshal Sheth**](https://harshal.sheth.io/2022/01/31/webassembly.html) by Harshal Sheth<br>
Good overview of where WebAssembly is useful. In the embedded world, I see huge potential for it to implement sandboxed plugin systems at near-native speed and memory footprint. And yes, “WebAssembly” is a misnomer… - Heiko

- [**So you want to build an embedded Linux system?**](https://jaycarlson.net/embedded-linux/) by Jay Carlson<br>
A fantastic and detailed article on what makes up an embedded Linux system, from the hardware up, from the amazing Jay Carlson! - Noah

- [**Raspberry Pi microSD card performance comparison (2019)**](https://www.jeffgeerling.com/blog/2019/raspberry-pi-microsd-card-performance-comparison-2019) by Jeff Geerling<br>
Jeff Geerling tests a number of popular SD cards for performance on a Raspberry Pi  - Noah

- [**Linux on an 8-bit micro? - Dmitry.GR**](https://dmitry.gr/index.php?proj=07.+Linux+on+8bit&r=05.Projects) by Dmitry<br>
Another old but gold article showing Linux running on an Atmega 🤩. - Noah

- [**The (8501) Embedded Controller and Its Legacy**](https://8051enthusiast.github.io/2021/07/05/001-EC_legacy.html)<br>
A run through of the embedded controller on a modern laptop (in case the link didn't spoil it already, spoiler! it's an 8051 😅). - Noah

- [**"Fast Kernel Headers" Tree -v1: Eliminate the Linux kernel's "Dependency Hell"**](https://lwn.net/ml/linux-kernel/YdIfz+LMewetSaEB@gmail.com/)<br>
Huge effort to reduce the include graph for Linux kernel headers (2000+ commits!), has an impressive improvement to build times. - Noah

- [**A Strategy for Reporting Version Information from Bootloaders - Embedded Artistry**](https://embeddedartistry.com/blog/2022/01/20/a-strategy-for-reporting-version-information-from-bootloaders/) by Phillip Johnston<br>
Outline of a strategy Embedded Artistry uses to access versions in their software.

- [**Debugging an Arduino project with GDB on Classic ATtiny and Small ATmega MCUs - CodeProject**](https://www.codeproject.com/Articles/5321801/Debugging-an-Arduino-project-with-GDB-on-classic-A) by Bernhard Nebel<br>
A tutorial for people who finally want to debug their Arduino projects that run on AVR MCUs using the GNU project debugger GDB. - Colleen

- [**Adding Unit Tests To Your Firmware Using Ceedling + GitHub Actions CI in Bluetooth Development - Ovyl**](https://ovyl.io/blog-posts/adding-unit-tests-to-your-firmware-github-actions-ci-in-bluetooth-development) by Nick Sinas<br>
An article from a member of the Interrupt community about using Github actions and Ceedling to test Bluetooth code. - François

- [**Rust on Apache NuttX OS**](https://lupyuen.github.io/articles/rust2) by Lee Lup Yuen<br>
A getting started post with Rust, Apache NuttX, the Pinedio LoRa module, a RISC-V board, and more. - Tyler

- [**Encode Sensor Data with CBOR on Apache NuttX OS**](https://lupyuen.github.io/articles/cbor2) by Lee Lup Yuen<br>
An incredibly detailed article about encoding data with TinyCBOR. If you haven't done much data encoding with C, it's a great place to start, as it's never quite as trivial as it would be in higher level languages. - Tyler

- [**Making nice-looking and interactive diagrams for your PCBs**](https://blog.honzamrazek.cz/2021/06/making-nice-looking-and-interactive-diagrams-for-your-pcbs/)<br>
A tool that takes KiCAD files and produces interactive HTML (highlights + descriptions of components and pinouts) for your PCB that you can embed in your own pages. - Heiko

- [**Dumping Firmware With a 555 - In Which Jarrett Builds**](https://jrainimo.com/build/2022/01/dumping-firmware-with-a-555/) by Jarrett<br>
Post about voltage glitching with an STM8 in order to bypass the read-out protection and dump the firmware, one byte at a time. - Tyler

- [**On Proebsting's Law - zeux.io**](https://zeux.io/2022/01/08/on-proebstings-law/)<br>
An interesting look at how LLVM compilation time and generated code performance has changed over time. From [LLVM Weekly](https://llvmweekly.org/). - Noah

- [**VolksEEG: Rust Development On Adafruit nRF52840 Feather Express - EmbeddedRelated**](https://www.embeddedrelated.com/showarticle/1437.php) by Steve Branam<br>
A great play-by-play on getting an nRF52840 set up to develop and debug Rust with all the fixins, such as VSCode, Docker, and serial output. - Tyler

- [**Hacking a VW Golf Power Steering ECU - Part 1**](https://blog.willemmelching.nl/carhacking/2022/01/02/vw-part1/) by Willem Melching<br>
An in-depth firmware reverse engineering project on the VW power steering ECU to enable self-driving on older VW models. - François

- [**Multi-threaded Singleton Access in embedded C++ - Stratify Labs**](https://blog.stratifylabs.dev/device/2022-01-27-Multithread-Singleton-Access-in-embedded-cpp-copy/)<br>
Using C++, you can automatically and cleanly control access to resources in a multi-threaded environment using singletons. We've all been in those C codebases where mutex unlocks and locks were forgotten at times. - Tyler

- [**Project Zero: One font vulnerability to rule them all #1: Introducing the BLEND vulnerability (2015)**](https://googleprojectzero.blogspot.com/2015/07/one-font-vulnerability-to-rule-them-all.html?m=1) by Mateusz Jurczyk<br>
Legendary exploit taking advantage of TrueType/PostScript fonts, which operate a kind of stack-based VM, which can be broken out of. - Noah

## Tools & Projects

- [**combustion-inc/combustion-ios-ble: Bluetooth Low Energy framework for communicating with Combustion Inc. Predictive Thermometers**](https://github.com/combustion-inc/combustion-ios-ble)<br>
Combustion Inc., a company building a BLE-based multi-point thermometer, open-sourced their iOS BLE communication framework which uses Apple‘s Combine framework. - Marcel

- [**Mahlet-Inc/hobbits: A multi-platform GUI for bit-based analysis, processing, and visualization**](https://github.com/Mahlet-Inc/hobbits)<br>
Interesting tool for analyzing data packets or fragments at a very low level, both interactivity through a GUI or via a command line interface. Supports custom plugins/filters for enhancing decode. - Noah

- [**Aircoookie/WLED: Control WS2812B and many more types of digital RGB LEDs with an ESP8266 or ESP32 over WiFi!**](https://github.com/Aircoookie/WLED)<br>
Nicely featured LED strip controller using ESP8266/ESP32. I particularly enjoyed this part of the quickstart guide _"If everything worked the first thirty LEDs will light up in bright orange to stimulate courage, friendliness and success!"_ 😀 - Noah.

- [**Ovyl/tabouli: TUI for sending CLI commands to your firmware and devices**](https://github.com/Ovyl/tabouli)<br>
They made a TUI specifically designed for interfacing with a serial port on an embedded device. It stores command history, runs a suite of test automation commands, and more! Written in Go.

- [**Pulp - Playdate**](https://play.date/pulp/)<br>
We're huge fans of the Playdate at Memfault, and they just released a new online game designer.

## News, Updates, and Random Tidbits

- [**Windows WSL2 is getting USB support? (Twitter)**](https://twitter.com/beriberikix/status/1487127732190212102?s=20&t=NQVa27qvOqPi2uGz6pJNRA) by Jonathan Beri<br>
Looks like WSL2 is getting USB support soon! Very exciting, for setups that need native Windows (e.g. for Altium), having access to an efficient Linux layer with USB access enables some nice development processes (e.g. building embedded Linux, better filesystem, and caching performance). - Noah

- [**Introducing MicroShift - Red Hat Emerging Technologies**](https://next.redhat.com/2022/01/19/introducing-microshift/) by Ricardo Noriega De Soto<br>
Redhat is building a Kubernetes distribution for the edge. - François

- [**Framework - Open Sourcing our Firmware**](https://frame.work/blog/open-sourcing-our-firmware) by Nirav Patel<br>
The team behind the modular laptop design “Framework” published their firmware for the “Embedded Controller”. This is exciting as it’s another step towards a consumer product you truly own (it currently doesn’t seem to support coreboot).

- [**Announcing Rust 1.58.0 - Rust Blog**](https://blog.rust-lang.org/2022/01/13/Rust-1.58.0.html)<br>
Captured identifiers in print strings! - Noah

- [**Improving LLVM Infrastructure - Part 1: Mailing lists - The LLVM Project Blog**](https://blog.llvm.org/posts/2022-01-07-moving-to-discourse/)<br>
LLVM Project is moving to Discourse! This usually wouldn't make a list like this but we all know how crawling over mailing lists is a huge pain, so maybe this is a new trend!

- [**Research log: PCB stepper motor**](https://kevinlynagh.com/pcb-stepper/)<br>
Amazing hardware post starting from "I think that's a cool idea" to "After a few tries, I think I finally got it". Watch the embedded videos at the very least, and if you're craving more, learn how to move magnets across a PCB using electrical currents.

- [**Raspberry Pi Direct: buy RP2040 in bulk from just $0.70 - Raspberry Pi**](https://www.raspberrypi.com/news/raspberry-pi-direct-buy-rp2040-in-bulk-from-just-0-70/)<br>
A microcontroller that can be bought in 2022. Sigh.

- [**Locked Out of ‘God Mode,’ Runners Are Hacking Their Treadmills - WIRED**](https://www.wired.com/story/nordictrack-ifit-treadmill-privilege-mode/) by Matt Burgess<br>
There is currently a big debate in the industry about user ownership vs. user experience. Nordictrack clearly is on one hand of that conversation, and it's hitting the mainstream news.
