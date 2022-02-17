---
title: "What we've been reading in October (2020)"
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

- [So you want to build an embedded Linux system?](https://jaycarlson.net/embedded-linux/) by Jay Carlson<br>Another thorough post from Jay, the author of [$1 MCU](https://jaycarlson.net/microcontrollers/). This time, he goes into detail about Embedded Linux and then compares and contrasts a number of boards that support it.
- [Optimizing Power Consumption in Battery-Powered Devices](https://embeddedartistry.com/blog/2020/10/26/optimizing-power-consumption-in-battery-powered-devices/) by Silard Gal<br>A nice post about proactive measures you can take to ensure your MCU uses the appropriate (and least!) amount of power possible. 
- [Fast integer scaling on Cortex-M0](https://m0agx.eu/2020/10/04/fast-integer-scaling-on-cortex-m0/) by M0AGX<br>A blog post covering how to simple integer scaling using bit-shifts instead of division, particularly useful when the MCU doesn't support integer division.
- [Potential improvements for Rust embedded abstractions](https://tweedegolf.nl/blog/42/potential-improvements-for-rust-embedded-abstractions) by Lars @ Tweede Golf<br>A brain-dump from an embedded engineer about Rust's embedded abstractions. A good read for those considering joining the embedded Rust train.
- [It Is Never a Compiler Bug Until It Is](http://r6.ca/blog/20200929T023701Z.html) by Russel O'Conner<br>A writeup about a bug in GCC 9 & 10 about when `memcmp` will fail.
- [Bare Metal Programming on Raspberry Pi 3](https://github.com/bztsrc/raspi3-tutorial)<br>A Github project that walks the reader through writing bare-metal firmware in C. It contains 21 parts and could keep me busy for a while.
- [ninja: a simple way to do builds](https://jvns.ca/blog/2020/10/26/ninja--a-simple-way-to-do-builds/) by Julia Evans<br>A short but sweet post about the Ninja build system, which talks briefly about its simplicity and auto-generation concepts.
- [My Favorite ARM Instruction](https://keleshev.com/ldm-my-favorite-arm-instruction/) by Vladimir Keleshev<br>A well-written post about the ARM instruction `LDM`, or *load multiple*, which is one of the instructions which make saving and popping registers in ARM so quick!
- [How to Get Fired Using Switch Statements & Statement Expressions](https://blog.robertelder.org/switch-statements-statement-expressions/) by Robert Elder<br>A fun post about how pushing the limits and readability of switch statements in C.
- [Embedded Programming Without the IDE (2016)](http://reecestevens.me/blog/2016/07/08/embedded-programming-without-ide/) by Reece Stevens<br>Another helpful post for those trying to switch away from vendor IDE's to Make + GCC builds. 
- [My toothbrush streams gyroscope data](https://blog.johannes-mittendorfer.com/artikel/2020/10/my-toothbrush-streams-gyroscope-data) by Johannes Mittendorfer<br>It turns out the new Sonicare toothbrushes stream out gyroscope data over BLE characteristics. Johannes writes about his experience reverse-engineering the protocol. The post even includes a Python library to decipher the data being sent by the toothbrush. 
- [Compile and Flash Micropython Firmware on STM32F7](https://neil.computer/notes/how-to-compile-and-flash-micropython-firmware-on-stm32f7/) by Neil Panchal<br>A short note-to-self post documenting how to build and flash MicroPython on an STM32F7.
- [Definitely not Windows 95: What operating systems keep things running in space?](https://arstechnica.com/features/2020/10/the-space-operating-systems-booting-up-where-no-one-has-gone-before/) by ArsTechnica<br>Ars does some digging on the software that runs on satellites and finds that RTOS's are quite common!
- [Keil Studio Online](https://os.mbed.com/keil/)<br>I didn't see this when it was first announced, but it looks like Keil is going to the cloud! Based upon Eclipse Theia, which was based upon VSCode, it uses WebUSB to interact with local devices and promises multi-platform support.
- [Reddit - temporarily-bricked Ferrari due to no cell reception](https://old.reddit.com/r/Justrolledintotheshop/comments/j914fh/dude_comes_straight_from_the_dealership_for_a/)<br>A hilarious short story about a Ferrari which failed mid-update and was left "broken" in DFU mode unable to update due to lack of cell reception.

## Neat Open Source Projects

- [lupyuen/remote-pinetime-bot](https://github.com/lupyuen/remote-pinetime-bot) by Lee Lup Yuen<br>In another installment in the PineTime series, Lee documents how he built a Telegram bot that remotely flashes his smartwatch.
- [STM32-base](https://stm32-base.org/)<br>A project whose goal is to provide a working base project for STM32 MCU's. If you don't use the project, at least look at their [cheatsheets](https://stm32-base.org/cheatsheets/). They are amazing.
- [ploopy - Open Source mouse running QMK](https://www.ploopy.co/mouse)<br>An open-source mouse running the [QMK firmware](https://docs.qmk.fm/#/), which is soon delivering its pre-orders! You can follow along on [/r/ploopy](https://old.reddit.com/r/ploopy/).
- [Kitspace](https://kitspace.org/)<br>Kitspace is a place to share ready-to-order electronics designs. They also maintain the "1-click BOM" browser extension that helps quickly price out parts on Digikey from an Excel spreadsheet. Where has this been all my life?
- [Three open source Sonos projects: efficient embedded development in Rust ](https://tech-blog.sonos.com/posts/three-open-source-sonos-projects-in-rust/)<br>I'm happy to see Sonos keeping some of the open-source projects alive from [Snips](https://snips.ai/), the company they acquired a bit ago. The projects include an on-device test library, a neural network inference library, and a tool that helps bind C, C++, and Rust.
- [MicroMod by SparkFun](https://www.sparkfun.com/pages/micromod)<br>A modular hardware system built by SparkFun that allows one to switch out "processor boards" on "carrier boards". I'm curious to see where this goes!
- [brainstorm/bbtrackball-rs](https://github.com/brainstorm/bbtrackball-rs)<br>A Rust-based blackberry-style trackball firmware. It also mostly works on OSX! It seems to be more of a learning exercise.
- [FsmPro](https://www.fsmpro.io/#feature-area)*<br>A nice finite state machine generator for Windows and Linux that generates C code from diagrams. Not an open-source project, but felt I should include it.

## News

- [Google and Facebook Select Zephyr RTOS for Next Generation Products](https://www.zephyrproject.org/google-and-facebook-select-zephyr-rtos-for-next-generation-products/)<br>Although we don't have much further information, it appears Google and Facebook are supporting the Zephyr project and will be using it in future products. I guess instead of buying an RTOS, Facebook and Google will just support the already existing one. I support this.
- [Introducing Twilio Microvisor IoT Platform](https://www.twilio.com/blog/introducing-microvisor)<br>Twilio announced what I think is the coolest use-case of ARM's TrustZone. It allows you to debug and flash chips remotely and securely if you are using Twilio's software or boards. There's more information to be uncovered about this.