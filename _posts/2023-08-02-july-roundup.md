---
title: "What we've been reading in July"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

Here are the articles, videos, and tools that we've been excited about this
July. 

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).


## Events
- [**Memfault Webinar - How to De-Risk IoT Product Launches with Device Reliability Engineering**](https://register.gotowebinar.com/register/5913767702979690334?source=Memfault)<br>
Join this IoT Central webinar on August 10th with Memfault CEO & Interrupt Co-Creator, François Baldassari, and discover how utilizing device reliability engineering (DRE) techniques minimizes risks during product launches and ensures timely shipment. You'll learn about:
  - Reducing the NPI (New Product Introduction) timeline
  - Separating software and hardware timelines, utilizing test-driven development, Day-0 updates, and more
  - Incorporating performance metrics, debugging features, and reliable OTA into IoT devices

- [**Memfault Webinar - How to Debug Android Devices in Production**](https://go.memfault.com/how-to-debug-android-devices-production)<br>
Join us on Thursday, August 17th for a deep dive into the intricacies of Android crash analysis and how to expedite the process, led by Chris Hayes (Android Solution Engineer). Chris will go over:
  - Out-of-the-box AOSP tools readily available and ways to augment them with other tools and techniques
  - How to catch kernel panics, java exceptions, native crashes, ANRs, and more to root cause high-impact faults quickly
  - How to use Memfault to collect comprehensive crash and non-fatal error data from Kernel to App 

- [**San Diego Firmware Meetup by Memfault**](https://www.eventbrite.com/e/san-diego-firmware-meetup-tickets-679334737487)<br>
Team Memfault would love to host you on Wednesday, September 6th for another evening of technical talks, demos, and networking over food & drinks! We had a great turnout for our first San Diego Meetup back in June and expect these to continue to grow. Get your free ticket [here](https://www.eventbrite.com/e/san-diego-firmware-meetup-tickets-679334737487). 

- [**Berlin Firmware Meetup by Memfault**](https://www.eventbrite.com/e/berlin-firmware-meetup-tickets-685436237237)<br>
Team Memfault is excited to share that our Meetups are expanding abroad! We will host the first Berlin Firmware Meetup on Thursday, September 14th at [MotionLab.Berlin](https://motionlab.berlin/). This event is a great opportunity to meet other local engineers of all technical levels and learn about the latest embedded developments. Sign up [here](https://www.eventbrite.com/e/berlin-firmware-meetup-tickets-685436237237) for your free ticket.

- [**Boston Firmware Meetup by Memfault**](https://www.eventbrite.com/e/boston-firmware-meetup-tickets-678359811457)<br>
Team Memfault welcomes you to join us at the next Boston Firmware Meetup on Wednesday, September 20th to connect with other local embedded engineers and meet members of the Boston office. We have some exciting lightning talks lined up and of course, food and drink will be provided. Register [here](https://www.eventbrite.com/e/boston-firmware-meetup-tickets-678359811457) for your free ticket.



## Articles & Learning
- [**FloatZone: Accelerating Memory Error Detection using the Floating Point Unit**](https://download.vusec.net/papers/floatzone_sec23.pdf)<br>
Use floating point exceptions to speed up runtime memory error checking. - Noah

- [**Teardown of the TM4313 GPS Disciplined Oscillator | Electronics etc…**](https://tomverbeure.github.io/2023/07/09/TM4313-GPSDO-Teardown.html)<br>
Originally found on Hacker News, teardown of a GPS-corrected oscillator. - Noah

- [**Optimizing Firmware: Shipping IoT Devices on Time - DZone**](https://dzone.com/articles/optimizing-firmware-the-key-to-shipping-iot-device)<br>
A review of key milestones to pay attention to when using a New Product Introduction (NPI) timeline and four suggestions on how to decouple your hardware and software to save time and ensure deadlines are met.

- [**NULL pointer protection with ARM Cortex-M MPU - Miro Samek**](https://www.embeddedrelated.com/showarticle/1546.php)<br>
This post from Embedded Related details how to set up the ARM Cortex-M MPU to protect your code against NULL-pointer dereferencing.

- [**M0AGX / LB9MG - Practical comparison of ARM compilers**](https://m0agx.eu/practical-comparison-of-ARM-compilers.html)<br>
If you're looking for a practical comparison of ARM compilers (GCC, Clang, and IAR), then look no further.

- [**GNU Linker Wizardry: Wrapping printf() with Timestamps | MCU on Eclipse**](https://mcuoneclipse.com/2023/07/22/gnu-linker-wizardry-wrapping-printf-with-timestamps/)<br>
Good article on how to add timestamps to `printf`. Adding timestamps to printf (as well as debug, info, warning, etc.) is always one of the first things we either add to printf or to the serial console wrapper (usually PySerial's miniterm).

- [**IoT devices that last: The role of device reliability engineering**](https://www.iottechnews.com/news/2023/jul/27/iot-devices-last-role-device-reliability-engineering/)<br>
François explains the ways Device Reliability Engineering (DRE) allows developers to accelerate IoT and edge device delivery that prioritizes reliability and security while minimizing risks.

- [**Memfault Webinar - Employing Coredumps to Debug Your Embedded Devices (Recording)**](https://go.memfault.com/employing-coredumps-debug-embedded-devices)<br>
In this recording, Eric Johnson (Firmware Solution Engineer), provides an overview of coredumps, how to use GDB with a coredump, and how to use coredumps in practice with Zephyr.



## Projects & Tools
- [**rp-rs/pio-rs: Support crate for Raspberry Pi's PIO architecture.**](https://github.com/rp-rs/pio-rs)<br>
Very cool inline assembler for rp2040 PIO written in Rust. - Blake

- [**jcmvbkbc/linux-xtensa: Linux port for xtensa architecture. None of these branches are stable.**](https://github.com/jcmvbkbc/linux-xtensa)<br>
Run Linux on an esp32! We do not ask why we do these things, only if we can. - Noah

- [**Intro to Bluetooth Low Energy [Second Edition] - 2023**](https://novelbits.io/intro-bluetooth-low-energy-version-2/)<br>
Learn the basics of Bluetooth Low Energy in a single weekend from the experts at [Novel Bits](https://novelbits.io/) in this updated e-book.



## Announcements & News
- [**Nordic nRF52 Series and nRF53 Series now have a fully qualified stack for Bluetooth v5.4**](https://www.linkedin.com/posts/nordic-semiconductor_bluetoothlowenergy-activity-7081939797803433984-01-a/)<br>
The nRF52 series and nRF53 series are now certified for BLE v5.4 with the latest version of nRF Connect SDK v2.4.0. Read more about Nordic's BLE products [here](https://www.nordicsemi.com/Products/Bluetooth-Low-Energy). 

- [**Zephyr Project Developer Summit @ EOSS 2023 - YouTube**](https://www.youtube.com/playlist?list=PLbzoR-pLrL6rQLZttVSF_DwzncObtwyM3)<br>
All Zephyr Project Developer Summit talks from this year's Embedded Open Source Summit (EOSS) are now on YouTube.
