---
title: "What we've been reading in January (2024)"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

Here are the articles, videos, and tools that we've been excited about this
January. 

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).


## Articles & Learning
- [**Measuring the Power Consumption of an ARM Cortex-M0 MCU: STM32F072**](https://metebalci.com/blog/measuring-the-power-consumption-of-an-arm-cortex-m0-mcu-stm32f072/)<br>
Love this deep-dive into the different power states of the Cortex-M0. This author also covered the [power states of desktop CPUs](https://metebalci.com/blog/a-minimum-complete-tutorial-of-cpu-power-management-c-states-and-p-states/).

- [**We don't need a DAC - ESP32 PDM Audio - by Chris Greening**](https://atomic14.substack.com/p/esp32-s3-no-dac)<br>
This video describes how to overcome the absence of a DAC on the ESP32-S3 by utilizing Pulse Density Modulation (PDM) for audio output. - François

- [**Power Optimization Recommendations using Zephyr - Golioth**](https://blog.golioth.io/power-optimization-recommendations/)<br>
Good article from our friends at Golioth about optimizing power on Zephyr. Essentially - disable what you don't use!

- [**Modern On-Target Embedded System Testing with CMake and CTest | MCU on Eclipse**](https://mcuoneclipse.com/2023/12/18/modern-on-target-embedded-system-testing-with-cmake-and-ctest/)<br>
An end-to-end guide on how to get on-target unit tests up and running with CTest, VS Code, and SEGGER tools.

- [**2023 in Review - Embedded Artistry**](https://embeddedartistry.com/blog/2024/01/01/2023-in-review/)<br>
Reading what our friends at Embedded Artistry accomplished in 2023 is both intriguing and inspiring. 

- [**&quot;Bit-Banging&quot; Bluetooth Low Energy - Dmitry.GR**](https://dmitry.gr/?r=05.Projects&proj=11.%20Bluetooth%20LE%20fakery)<br>
A guide on how to overcome the absence of a DAC on the ESP32-S3 by utilizing Pulse Density Modulation (PDM) for audio output. -François

- [**Exploring Serverless CI/CD for Embedded Devices - Embedded Artistry**](https://embeddedartistry.com/blog/2024/01/15/exploring-serverless-ci-cd-for-embedded-devices/)<br>
Learn how to build a serverless CI/CD workflow to achieve seamless OTA updates using GitHub Actions, AWS tools, and ESP-IDF framework (or just use Memfault, as the author recommends!). - François

- [**Zephyr Devicetree Mysteries, Solved - Marti Bolivar, Nordic Semiconductor - YouTube**](https://www.youtube.com/watch?v=w8GgP3h0M8M&pp=ygURemVwaHlyIGRldmljZXRyZWU%3D)<br>
Marti Bolivar's devicetree talk from the 2022 Zephyr Dev Summit talk is gold! Fun AND informative. A must-watch to understand Zephyr devicetree's "macrobatics". - Gillian

- [**FreeRTOS with Heap Protector | MCU on Eclipse**](https://mcuoneclipse.com/2024/01/28/freertos-with-heap-protector/)<br>
FreeRTOS v11 has a heap protector module to help with detecting corruption in heaps. I also learned that FreeRTOS has a `configHEAP_CLEAR_MEMORY_ON_FREE` which scrubs memory on any `free()` call. Neat!

- [**Fixing a broken smart cat feeder with ESP8266 • pdx.su**](https://pdx.su/blog/2024-01-19-fixing-a-broken-smart-cat-feeder-with-esp32/)<br>
ESP32's to save the day again - this time to fix a broken PetNet cat feeder.


## Projects & Tools
- [**OffBroadway/flippydrive: An open-source ODE modchip for GameCube!**](https://github.com/OffBroadway/flippydrive)<br>
A really cool flash cart mod chip for GameCube using Raspberry Pi Pico. - Blake

- [**Rust Embedded Working Group**](https://blog.rust-embedded.org/embedded-hal-v1/)<br>
This v1.0 of a cool embedded Rust project just released. - Pat

- [**How to reverse-engineer a circuit board — Tube⛄Time / Bluesky**](https://bsky.app/profile/tubetime.bsky.social/post/3kispybsjct2s)<br>
A fun thread walkthrough on reverse-engineering a circuit board. - Eric

- [**atc1441/ATCmiBand8fw: A custom firmware for the Xiaomi Mi Band 8**](https://github.com/atc1441/ATCmiBand8fw)<br>
Someone reverse-engineering the Xiaomi Mi Band 8 and wrote a custom firmware for it. I love these types of projects because they show what a true minimal firmware looks like.

- [**zephyrproject-rtos/jlink-zephyr: J-Link Zephyr RTOS plugin**](https://github.com/zephyrproject-rtos/jlink-zephyr)<br>
There is finally an official JLink RTOS plugin for Zephyr. Now you can reliably get all threads on a running Zephyr system with your JLink debugger connected.

- [**Revisiting Candle-Flicker LEDs: Now with integrated Timer – Tim's Blog**](https://cpldcpu.wordpress.com/2024/01/14/revisiting-candle-flicker-leds-now-with-integrated-timer/)<br>
Inexpensive 8-bit OTP microcontrollers are often used for modern candle-flicker LEDs with integrated timers, but are they as power-efficient as they are cost-effective? Read to find out! - François

- [**n7space/aerugo: Safety-critical applications oriented Real-Time Operating System written in Rust**](https://github.com/n7space/aerugo)<br>
A Rust-based RTOS built for Space on a SAMV71 Cortex-M7 build by [n7space](https://n7space.com/).


## Upcoming Events
- [**Memfault Webinar - Beyond the Launch: Mastering IoT Device Quality**](https://hubs.la/Q02j3b_P0)<br>
On Thursday, February 29th, join Memfault's panel of IoT device makers including experts from Engineering and Customer Experience as they discuss maintaining Product quality post-launch. Register [here](https://hubs.la/Q02j3b_P0) to get the recording sent to you afterward.

- [**Embedded World 2024: Visit Memfault at Booth 4-238 in Hall 4**](https://hubs.la/Q02kgvHP0)<br>
[Embedded World](https://www.embedded-world.de/en), the world's leading conference for embedded systems, will be back in Nuremberg, Germany from 9-11 April! Come meet the Memfault team at **Booth 4-238 (Hall 4)** for a live demo of Memfault's embedded observability platform, grab some limited edition swag, and enter our daily raffle where we're giving away cool prizes like Panic Playdates. We will have special items for Interrupt community members! [Click here to reserve your swag](https://share.hsforms.com/1YlErHzpVT-avI4zPg2SRPA53an2) and let us know you'll be swinging by the booth.
