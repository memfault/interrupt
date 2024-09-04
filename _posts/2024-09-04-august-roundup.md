---
title: "What we've been reading in August (2024)"
description:
  Here are the articles, videos, and tools we've been excited about in August
  2024.
author: gminn
tags: [roundup]
---

<!-- excerpt start -->

Here are the articles, videos, and tools that we've been excited about this
August.

<!-- excerpt end -->

What have you been reading? Share in the comments or
[on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).

## Articles & Learning

- [**CNCF, Linux Foundation, and top 30 open source project velocity | CNCF**](https://www.cncf.io/blog/2024/07/11/as-we-reach-mid-year-2024-a-look-at-cncf-linux-foundation-and-top-30-open-source-project-velocity/)<br>
  A neat blog post on open source project velocity over the last year. Zephyr is
  #4 in Linux Foundation project, and top 30 in open source projects over all!

- [**Bazel for Embedded: Pigweed SDK Launches with Native Bazel Support | Bazel Blog**](https://blog.bazel.build/2024/08/08/bazel-for-embedded.html)<br>
  Google's Pigweed team has been working hard on making Bazel the best build
  system for embedded devices. Read their blog post to learn why! -- Tyler

- [**Multiprocessing - Python's best kept secret | M0AGX / LB9MG**](https://m0agx.eu/multiprocessing-pythons-best-kept-secret.html)<br>
  This post breaks down how to use Python’s multiprocessing module to sidestep
  the GIL and supercharge your data processing, going from 80 to 1400
  files/second. Definitely worth a read if you’re working with big data!

- [**Matt Madison, meta-tegra, and what makes a good Yocto BSP | The TMPDIR podcast**](https://tmpdir.org/034/)<br>
  A great listen if you're into Yocto and Nvidia’s Jetson devices! Matt Madison
  dives into what makes a solid BSP, his work on meta-tegra, and tips from his
  experience as a Yocto/Embedded Linux pro. -- Pat

- [**Will Rust be alive in 10 years? | Tweede golf**](https://tweedegolf.nl/en/blog/128/rust-in-ten-years)<br>
  This article explores Rust's growing adoption across industries, especially
  due to its ability to solve memory safety issues. Read the blog to find out
  whether or not Rust is here to stay.

- [**Rust Reviewed: the Current Trends and Pitfalls of the Ecosystem | InfoQ**](https://www.infoq.com/articles/rust-ecosystem-review-2023/)<br>
  This article takes a deep dive into the growing Rust community, how the
  industry is adopting it, and what folks love (and don’t) about the ecosystem.
  Despite a few challenges with debugging and profiling, Rust continues to prove
  itself with solid performance and memory safety.

- [**What MCU manufacturers can do to accelerate AI adoption | Embedded Computing Design**](https://embeddedcomputing.com/technology/ai-machine-learning/ai-dev-tools-frameworks/what-mcu-manufacturers-can-do-to-accelerate-ai-adoption)<br>
  This post examines the different nature of AI software development versus
  traditional embedded control systems.

- [**Tips and tricks for Linux OS updating | Foundries.io Blog**](https://foundries.io/insights/blog/tips-trick-linux-updating/)<br>
  Check out some factors affecting updating an embedded Linux operating system
  in this write-up.

- [**Air Con: $1697 for an on/off switch | Hopefully Useful Blog**](https://blog.hopefullyuseful.com/blog/advantage-air-ezone-tablet-diy-repair/)<br>
  Reverse engineering an Android HVAC controller to avoid an outrageous
  replacement charge for a faulty controller. Exactly the kind of hacking I
  love! -- Noah

## Projects & Tools

- [**dakhnod/ESP32-Bluetooth-USB-dongle | GitHub**](https://github.com/dakhnod/ESP32-Bluetooth-USB-dongle)<br>
  Neat idea - use an ESP32 as a generic HCI Bluetooth controller via bluez. --
  Noah

- [**project-ocre/ocre-runtime | GitHub**](https://github.com/project-ocre/ocre-runtime)<br>
  New webassembly "container" runtime for Zephyr. -- Jonathan Beri of Golioth
  and Interrupt Community Member

- [**Mr-Bossman/pi-pico2-linux | GitHub**](https://github.com/Mr-Bossman/pi-pico2-linux)<br>
  Linux on the RP2350! -- Noah

## News & Announcements

- [**Raspberry Pi Pico 2, our new $5 microcontroller board, on sale now | Raspberry Pi**](https://www.raspberrypi.com/news/raspberry-pi-pico-2-our-new-5-microcontroller-board-on-sale-now/)<br>
  New chip day- Raspberry PI RP2350 announced: dual core Cortex-M33 or dual core
  RISC-V- selectable at processor boot time! - Noah

- [**A closer look at Raspberry Pi RP2350's HSTX high-speed serial transmit interface | CNX Software**](https://www.cnx-software.com/2024/08/15/raspberry-pi-rp2350-hstx-high-speed-serial-transmit-interface/)<br>
  In the flurry of RP2350 press, this one caught my eye- the HSTX block supports
  up to 300Mbps output per pin (up to 8 pins), primarily intended for displays.
  -- Noah

- [**Rust Embedded Working Group 2024 survey**](https://www.surveyhero.com/c/uenp3ydt)<br>
  The Embedded Rust Working Group is running an official survey. If you've ever
  used Rust on a microcontroller or used it as part of your work for embedded
  systems, your feedback is appreciated! The survey is anonymous, run by the
  Rust Survey team, and will be open until September 19th! -- James Munns,
  friend of Memfault and Interrupt Community Member

## Upcoming Events

- [**Wednesday, September 11: Boston IoT Security Panel | Memfault**](https://www.eventbrite.com/e/the-path-forward-when-to-build-security-into-your-iot-device-boston-tickets-966341191517)<br>
  Join us in Boston for an engaging in-person panel discussion on when to build
  security into your IoT device. We’ll dive deep into the best practices for
  securing IoT devices, from firmware to system-level protections. Whether
  you're designing from the ground up or enhancing existing security measures,
  this event will provide actionable insights to help you safeguard your
  products and business.
  [**RSVP →**](https://www.eventbrite.com/e/the-path-forward-when-to-build-security-into-your-iot-device-boston-tickets-966341191517)

- [**Tuesday, September 17: Webinar - How New IoT Security Regulations Will Shape the Industry's Future | Memfault**](https://go.memfault.com/how-new-iot-security-regulations-will-shape-the-industrys-future)<br>
  The landscape of IoT security is shifting fast, with new regulations like the
  Cyber Resilience Act, Product Security Telecommunications Infrastructure, and
  Cyber Trust Mark reshaping the industry. Join Memfault’s Co-Founders François
  Baldassari and Chris Coleman as they explore how these changes will impact
  device development and deployment across the globe. Don’t miss this chance to
  stay ahead of the curve on IoT security compliance.
  [**Register →**](https://go.memfault.com/how-new-iot-security-regulations-will-shape-the-industrys-future)

- [**Tuesday, October 8 - Thursday, October 10: Embedded World North America**](https://www.embedded-world.de/en/embedded-world-wide/embedded-world-north-america)<br>
  Embedded World is coming to the U.S. for the first time, and Memfault will be
  there! Visit us at Booth #2434 in Austin, TX, from October 8-10. If you're an
  IoT developer,
  [**schedule a time to connect with us**](https://go.memfault.com/embedded-world-usa-2024)
  -- we would love to hear what you're working on! Whether you're deep into
  hardware and software design or just curious about the latest in M2M
  communication, this event is your gateway to the future of embedded systems --
  you won't want to miss it.

- [**Wednesday, October 9: Memfault's Embedded World USA After Party | Memfault**](https://www.eventbrite.com/e/memfaults-embedded-world-usa-after-party-tickets-1002979447527)<br>
  After a full day at Embedded World USA, come unwind with us at Full Circle
  Bar! Enjoy drinks, skee-ball, and friendly competition while mingling with
  fellow embedded community members. Take advantage of this opportunity to
  relax, network, and compete for bragging rights! This event is for those 21+
  and conference attendees.
