---
title: "What we've been reading in November (2020)"
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


- [The Hackers Guide to Hardware Debugging (video)](https://www.youtube.com/watch?v=hWYzgw0WhYU) by Matthew Alt<br>A 2-hour long video from the Hackaday Remoticon 2020 about reverse engineering hardware products in general, but the video covers reverse engineering an Xbox One controller. [Matthew's blog](https://wrongbaud.github.io) is also worth a look as he has some great posts, such as reverse engineering [JTAG](https://wrongbaud.github.io/posts/jtag-hdd/) and [SWD](https://wrongbaud.github.io/posts/stm-xbox-jtag/).
- [Bits On The Wire](https://www.tbray.org/ongoing/When/201x/2019/11/17/Bits-On-the-Wire) by Tim Bray<br>In the Interrupt Slack, we've been chatting a bit about binary protocols, and we found this post while digging into the topic. Tim gives a high-level overview of JSON vs binary encoding protocols and then goes into the popular libraries out today.
- [IoT Unravelled Part 1: It's a Mess... But Then There's Home Assistant](https://www.troyhunt.com/iot-unravelled-part-1-its-a-mess-but-then-theres-home-assistant/) by Troy Hunt<br>The first of five **technical** posts on the state of IoT for consumer products. They are all fresh off the press and were published last week. 
- [Eachine H8 Open Source Firmware with Acro Mode](https://dronegarageblog.wordpress.com/2016/01/06/eachine-h8-open-source-firmware-with-acro-mode/) by Drone Garage Blog<br>A post about installing open-source firmware on the Eachine H8 Mini drone (quadcopter) using an ST-link V2.
- [Getting Started with ESP32 and NuttX](https://medium.com/the-esp-journal/getting-started-with-esp32-and-nuttx-fd3e1a3d182c) by Sara Monteiro<br>A simple zero-to-something post using the ESP32 and NuttX by an employee at Espressif, complete with some NuttX tips at the end.
- [Metal.GDB: Controlling GDB through Python Scripts with the GDB Python API](https://embeddedartistry.com/blog/2020/11/09/metal-gdb-controlling-gdb-through-python-scripts-with-the-gdb-python-api/) by Klemens Morgenstern<br>A creative use-case for controlling embedded devices using the GDB Python API.
- [CBOR IoT Data Serialization for Embedded C and Rust](https://www.jaredwolff.com/cbor-for-embedded-c-and-rust/) by Jared Wolff<br>A blog covering encoding data with CBOR in C, and then deserializing it on a Rust server. It is filled with coding examples, so it should serve as a good starting point for projects thinking about using CBOR.
- [Turn $1.5 Blue Pill STM32 board into a Sigrok compatible logic analyzer](https://www.cnx-software.com/2020/11/14/turn-1-5-blue-pill-stm32-board-into-a-sigrok-compatible-logic-analyzer/) by <br>Using the [buck50](https://github.com/thanks4opensource/buck50) firmware on an STM32F103, you can have yourself a test & measurement instrument for $1.50! I'm not going to explain much here, because the [buck50 documentation](https://github.com/thanks4opensource/buck50) everything so well.
- [Nordic Semiconductor nRF52840 Tools Comparison](https://skybluetrades.net/projects/nrf52840-tools-comparison/) by Ian Ross<br>In a similar spirit to our [Best and Worst MCU SDKs]({% post_url 2020-11-10-the-best-and-worst-mcu-sdks %}), Ian Ross covers all the different development environments that one can use while working on the nRF52, including PlatformIO, Zephyr, SEGGER Embedded Studio, and straight GCC & Makefiles. 
- [Reading and writing firmware on an STM32 using the serial bootloader](https://cybergibbons.com/hardware-hacking/reading-and-writing-firmware-on-an-stm32-using-the-serial-bootloader/) by Andrew Tierney<br>A technical post investigating the STM32 serial bootloader and how to load new images using it. A neat semi-documented feature!
- [STM32 & OpenCM3 5: Debugging & Fault Handlers](https://rhye.org/post/stm32-with-opencm3-5-fault-handlers/) by Ross Schlaikjer<br>The last post of the 5-part series on using an STM32 board with OpenCM3. The series covers alternate functions and USART, SPI, DMA, the memory layout, CAN bus, memory layout, and debugging using the fault handlers. 
- [What is a System-on-Chip (SoC), and Why Do We Care if They are Open Source?](https://www.bunniestudios.com/blog/?p=5971)<br>Written by one of the creators of the [Precursor SoC](https://www.crowdsupply.com/sutajio-kosagi/precursor), this post dives into what a system-on-a-chip actually is, why they were originally created, and what the story behind the Precursor SoC is. This blog detailing the story behind Precursor is worth a read too!


## Neat Projects

- [Embedded Virtual Machine Core Project](https://github.com/embvm/embvm-core) by Phillip Johnston of Embedded Artistry<br>An ambitious project with the goal of providing an "embedded virtual machine" on top of various hardware platforms. There is already an [nRF52 port](https://github.com/embvm/nordic), which is worth checking out. The project also primarily uses the Meson build system and could serve as a good learning experience.
- [kriswiner/MPU6050](https://github.com/kriswiner/MPU6050) - A great reference on IMU sensor fusion.
- [Map File Browser](http://www.sikorskiy.net/prj/amap/) - A GUI-based tool to help analyze `.map` files and investigate the code and data usage. Works on Mac, Windows, and Linux.
- [AtomicObject/Heatshrink](https://github.com/atomicobject/heatshrink) - An open-source compression library for embedded systems by the development shop [Atomic Object](https://atomicobject.com/). It came up in the Interrupt Slack group as a production-ready solution for compression with as little as a 50 byte memory buffer!


## News

- [ESP32-C3 WiFi & BLE RISC-V processor is pin-to-pin compatible with ESP8266](https://www.cnx-software.com/2020/11/22/esp32-c3-wifi-ble-risc-v-processor-is-pin-to-pin-compatible-with-esp8266/) - The most exciting news to happen this month. We are thrilled to see how Espressif can expand into the RISC-V space. If they keep their IDF compatible and make the transition smooth for everyone, I see this being a huge success for them.
- [Nordic Semiconductor expands into Wi-Fi](https://www.nordicsemi.com/News/2020/11/Nordic-Semiconductor-expands-into-WiFi) - An exciting turn of events as Nordic Semiconductor pushes into the Wi-Fi market, probably to compete with Espressif and the ESP32 lineup.
- [Alexa Voice Service: New Reference Design for STM32 Embedded Systems](https://blog.st.com/alexa-voice-service/) - ST announced an Alexa reference platform, utilizing an STM32H743, two microphones, Wi-Fi chip, and a speaker.
- [This Bluetooth Attack Can Steal a Tesla Model X in Minutes](https://www.wired.com/story/tesla-model-x-hack-bluetooth) - Another day, another Bluetooth attack vector. Thankfully Tesla can patch their vehicles easily with OTA updates.
