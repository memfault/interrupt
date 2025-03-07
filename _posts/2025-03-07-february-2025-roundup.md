---
title: "What we've been reading in February (2025)"
description:
  Here are the articles, videos, and tools we've been excited about in February
  2025.
author: gminn
tags: [roundup]
---

<!-- excerpt start -->

Here are the articles, videos, and tools that we've been excited about this
February.

<!-- excerpt end -->

What have you been reading? Share in the comments or
[on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).

## Articles & Learning

- [**FreeRTOS to Zephyr Porting Overview (WP560) | AMD**](https://docs.amd.com/r/en-US/wp560-rtos-zephyr-porting/Abstract)<br>
  FreeRTOS-to-Zephyr porting guide, including a table with links to the common
  features in each platform (tasks/threads, mutexes, etc.). － Noah

- [**Breaking into dozens of apartment buildings in five minutes on my phone – Eric Daigle**](https://www.ericdaigle.ca/posts/breaking-into-dozens-of-apartments-in-five-minutes/)<br>
  A walk through of the shockingly easy hacking of an apartment's access control
  system. －François

- [**It is not a compiler error (2017) | Hacker News**](https://news.ycombinator.com/item?id=43112187)<br>
  Great embedded debugging anecdotes in this thread. Remote debugging is always
  hard (unless you have Memfault 😉), but local debugging can be hard
  too! － Noah

- [**Coredump 03: Pebble’s Code is Free: Three Former Pebble Engineers Discuss Why It's Important (Part 1)**](https://go.memfault.com/coredump-pebble)<br>
  This popular Memfault webinar features former Pebble engineers discussing the
  open-sourcing of Pebble's firmware, its technical innovations, and its
  significance for the embedded community.

- [**Lessons from Pebble OS Memfault Webinar | Magpie Embedded**](https://magpieembedded.co.uk/content/2025_02_13_pebble_os_recap.html)<br>
  I really enjoyed the
  [PebbleOS talk hosted by Memfault](https://go.memfault.com/coredump-pebble)
  and I've written up my notes from it, sprinkled with plenty of links. Thanks
  to all those involved in making it happen, it is great to have such honest
  discussions out in the open! － Tim Guite, Interrupt Community Member

- [**The Miserable State of Modems and Mobile Network Operators**](https://blog.golioth.io/the-miserable-state-of-modems-and-mobile-network-operators/)<br>
  This was a fun bug to figure out. Ah the joys of binary blobs! － Jonathan
  Beri of Golioth, Interrupt Community Member

## Projects & Tools

- [**binsider.dev | Analyze binaries like a boss**](https://binsider.dev/)<br>
  CLI tool for ELF binary analysis. Static/dynamic inspection, hexdump, linked
  libs, and string extraction. －François

- [**wader/fq: `jq` for binary formats**](https://github.com/wader/fq)<br> A
  command-line tool for parsing, analyzing, and transforming binary formats
  using `jq`-style expressions. － François

- [**nopnop2002/esp32-idf-sqlite3: sqlite3 for esp-idf**](https://github.com/nopnop2002/esp32-idf-sqlite3)<br>
  Someone actually did it - sqlite on the esp-idf. － Jonathan Beri of Golioth,
  Interrupt Community Member

## News & Announcements

- [**Release 2.1.1 · raspberrypi/pico-sdk · GitHub**](https://github.com/raspberrypi/pico-sdk/releases/tag/2.1.1)<br>
  The RP2040 now supports a 200MHz clock, up from the previous official maximum
  of 125MHz. － Noah

- [**esp-hal 1.0.0 beta announcement · Espressif Developer Portal**](https://developer.espressif.com/blog/2025/02/rust-esp-hal-beta/)<br>
  `esp-hal`, which enables writing rust for ESP32 series chips, releases v1.0
  beta. － Noah

## Interrupt Live

- [**Friday, Mar 7 | Interrupt Live with Steve Noonan**](https://www.youtube.com/live/dwL-PI7TuDY)<br>
  Tyler Hoffman catches up with Interrupt author Steve Noonan to unpack the
  journey and reasoning behind his latest Interrupt contribution:
  [Why std::this_thread::sleep_for() is broken on ESP32](https://interrupt.memfault.com/blog/why-sleep-for-is-broken-on-esp32).
  Join us on [**YouTube Live**](https://www.youtube.com/live/dwL-PI7TuDY) at 9AM
  PT | 12PM ET | 6PM CET.

- [**Friday, Mar 28 | Interrupt Live with Mark Schulte**](https://www.youtube.com/live/aeCQiL1e75Y)<br>
  On this episode of Interrupt Live, Tyler is joined by Mark Schulte, author of
  [A Schematic Review Checklist for Firmware Engineers](https://interrupt.memfault.com/blog/schematic-review-checklist).
  They’ll discuss key aspects of firmware reliability, including architecture,
  unit testing, hardware-in-the-loop (HIL) testing, automated testing, firmware
  rollouts, monitoring, and user feedback. Join us on
  [**YouTube Live**](https://www.youtube.com/live/aeCQiL1e75Y) at 9:30 AM PT |
  12:30 PM ET | 5:30 PM CET.

> 📞 Ask our speakers a question! Join the Interrupt Slack and post your
> question in the
> [#interrupt-live](https://theinterrupt.slack.com/archives/C08B0TLSZPV)
> channel.

## Events

- [**March 11-13 | Embedded World 2025 – Don’t Miss Tyler Hoffman’s Talk**](https://www.embedded-world.de/en)<br>
  Embedded World, the world’s leading embedded systems conference, returns to
  Nuremberg, Germany, from March 11-13. Join Memfault’s VP of Field Engineering
  and Interrupt author, Tyler Hoffman, on **March 12 at 10:30 AM** for his talk,
  _From Defense to Offense – A Paradigm Shift in Error Handling_. Learn how
  offensive programming can help you track down elusive firmware bugs, improve
  security, and debug efficiently in production. Memfault will also hold demos
  at the Zephyr Booth (#4-170) - come say hi! Demo schedule:

  - Tuesday, March 11, 12:00 - 15:00
  - Wednesday, March 12, 15:00 - 18:00
  - Thursday, March 13, 9:00 - 12:00

- [**March 19 | Coredump 05: The Current Realities of Cellular IoT**](https://go.memfault.com/coredump-cellular-iot)<br>
  Join Memfault founders and Fabian Kochem, Director of Product Strategy at
  1NCE, as they explore the evolving landscape of cellular IoT, including
  technologies like NB-IoT, 5G, eSIM, and iSIM, offering insights into
  successful adoption strategies.
  [Register today.](https://go.memfault.com/coredump-cellular-iot)

- [**April 15 | Coredump 06: Pebble’s Code is Free: Three Former Pebble Engineers Discuss Why It's Important (Part 2)**](https://go.memfault.com/coredump-pebble-part-2)<br>
  Back by popular demand! The discussion continues with former Pebble engineers
  François Baldassari, Thomas Sarlandie, and Brad Murray, delving deeper into
  Pebble OS's technical innovations, debugging strategies, and lessons
  applicable to modern embedded development.
  [RSVP to get the recording sent to you.](https://go.memfault.com/coredump-pebble-part-2)

- [**April 23-24 | Hardware Pioneers Max 2025 – Visit Memfault at Stand #C4**](https://www.hardwarepioneers.com/)<br>
  Hardware Pioneers is the UK’s largest exhibition and conference dedicated to
  cutting-edge technologies, solutions, and tools for innovation-driven
  engineering teams. Memfault will be exhibiting from April 23-24 at **Stand
  #C4**. Use the promo code **SPEX50** for a 50% discount applicable to core
  tickets. [Get your tickets](https://www.hardwarepioneers.com/buy-tickets).
