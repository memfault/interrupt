---
title: "What we've been reading in September (2022)"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

Here are the articles, videos, and tools that we've been excited about this
September

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).

## Articles & Learning

- [**How to Build and Maintain IoT Management Systems for Scale (Memfault Webinar)**](https://go.memfault.com/how-to-build-and-maintain-iot-management-systems-for-scale) by Tyler Hoffman & Chris Coleman<br>
I had a blast doing a joint webinar with Chris talking about how to build engineering teams and data systems to ensure that firmware teams and hardware companies can build reliable products and infrastructure. Sounds like a mouthful, but honestly, the webinar was Chris telling everyone fun stories from the past, which is always a good time.

- [**How to De-Risk Product Launches with Device Reliability Engineering (Memfault Webinar)**](https://go.memfault.com/how-de-risk-product-launches-device-reliability-engineering) by François Baldassari<br>
I feel I learn a lot from every webinar François gives. In this one, he talks about TDD, Day-0 updates, why and how to build a Hardware Abstraction Layer (HAL), and why you should split up the manufacturing firmware build and normal operating firmware. These are all these he's learned throughout his time at Pebble, Oculus, and Memfault.

- [**Open Source Firmware Conference 2022 (Videos)**](https://www.osfc.io/2022/schedule/)<br>
The Open Source Firmware Conference 2022 videos have been posted online. Check them out!

- [**Debugging C on the CANPicod - Dr. Ken Tindell**](https://kentindell.github.io/2022/07/26/canpico-c-debug/)<br>
A quick article detailing how to get up and running debugging the CANPico, which is based upon the Raspberry Pi Pico (RP2040). - Tyler

- [**The Book of CP-System**](https://fabiensanglard.net/cpsb/index.html) by Fabien Sanglard<br>
A book about the hardware and software inside of the coin-op machines of yesterday, such as Street Fighter. - Tyler

- [**Fully Oxidizing `ring`: Creating a Pure Rust TLS Stack Based on `rustls` + `ring` « bunnie's blog**](https://www.bunniestudios.com/blog/?p=6521)<br>
bunnie is trying to build an entirely Rust-based operating system, [Xous OS](https://betrusted.io/xous-book/ch00-00-introduction.html), and this article is about building a TLS stack with Rust.

- [**Embedded Systems And The Volatile Keyword - mbedded.ninja**](https://blog.mbedded.ninja/programming/languages/c/embedded-systems-and-the-volatile-keyword/) by Geoffrey Hunter<br>
An article detailing when and why an embedded engineer would use the `volatile` keyword.

- [**Software Component Names Should Be Whimsical And Crypticd - Better Programming**](https://betterprogramming.pub/software-component-names-should-be-whimsical-and-cryptic-ca260b013de0)<br>
The piece states that software components that are expensive to rename (e.g. package names for a public package manager) should try not to avoid names that explain what they do. Instead, one should treat their name as just a “handle”, but optimized for humans: short, memorable, joyful. - Heiko

- [**How MQTT on Narrowband-IoT Can Ruin Your Project - Embedded Computing Design**](https://embeddedcomputing.com/technology/iot/wireless-sensor-networks/how-mqtt-on-narrowband-iot-can-ruin-your-project) by Fabian Kochem<br>
What happens when you use standard MQTT with NB-IoT. May work well in testing but fail gloriously in the field. If you use a TCP-based protocol on a UDP connection, you're going to have a bad time. There is a [UDP version of MQTT](https://mqtt-udp.readthedocs.io/en/latest/), but we think people should just use CoAP nowadays.

- [**Someone’s Been Messing With My Subnormals!**](https://moyix.blogspot.com/2022/09/someones-been-messing-with-my-subnormals.html)<br>
Amazing article in which the author goes way down the rabbit hole investigating the use of `-ffast-math` across ALL :open_mouth: python packages on PyPi. - Noah

- [**The GDB developer's GNU Debugger tutorial, Part 1: Getting started with the debuggerd - Red Hat Developer**](https://developers.redhat.com/blog/2021/04/30/the-gdb-developers-gnu-debugger-tutorial-part-1-getting-started-with-the-debugger) by Keith Seitz<br>
Series I wish I had read sooner! - Noah

- [**Dumping Tuya firmware**](https://jilles.com/posts/tuya/) by Jilles Groenendijk<br>
The adventure buying cheap hardware and dumping the Tuya-based firmware.

## Projects & Tools

- [**wapiflapi/gxf: Gdb Extension Framework is a bunch of python code around the gdb api.**](https://github.com/wapiflapi/gxf)<br>
From an Interrupt community post, this GDB Extension Framework is a Python library to make writing GDB extensions easier. We're big fans of the GDB Python API at Memfault, so we're glad projects like GFX exist! - François

- [**StateSmith/StateSmith: A state machine code generation tool suitable for bare metal, embedded and more.**](https://github.com/StateSmith/StateSmith)<br>
a finite state machine editor with the ability to generate C code you can include directly in your projects. (via Reddit /r/embedded). - François

- [**tsoding/olive.c: Simple 2D Graphics Library for C**](https://github.com/tsoding/olive.c)<br>
A simple C graphics library with 0 dependencies which could easily be adapted to embedded. - François

- [**Joey Castillo Reboots the Open Book Project with a Raspberry Pi Pico-Powered Redesign - Hackster.io**](https://www.hackster.io/news/joey-castillo-reboots-the-open-book-project-with-a-raspberry-pi-pico-powered-redesign-8bfee0675637)<br>
A very nice little open source ebook reader that uses a Raspberry Pi Pico. - Noah

## Announcements & News

- [**Arm's Dev Summit is now online-only and free**](https://www.arm.com/company/events/devsummit)<br>
I'll be speaking at this year's Arm Dev Summit talking about monitoring millions of devices remotely, and Alvaro Prieto from Sofar Ocean will be talking about debugging devices remotely while they are at sea. I'm sure there are others but these are the ones I know about and can confirm they contain real content for developers.

- [**Announcing Publication of the Draft Ferrocene - The AdaCore Blog**](https://blog.adacore.com/announcing-publication-of-the-draft-ferrocene-language-specification) by Quentin Ochem<br>
Ferrous Systems and Adacore have released their spec for safety-certified Rust.