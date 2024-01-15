---
title: "What we've been reading in December (2023)"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->
🎉 Happy New Year! 🎉 Here's to making 2024 the best year yet. 

2023 was an exciting year for Interrupt, with 36 new articles, 13 new external contributors, 12 community Meetups, and a partridge in a pear tree. Thanks for being a part of it. 

Here are the articles, videos, and tools that we've been excited about this
December. 

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).


## Articles & Learning
- [**Just about every Windows and Linux device vulnerable to new LogoFAIL firmware attack | Ars Technica**](https://arstechnica.com/security/2023/12/just-about-every-windows-and-linux-device-vulnerable-to-new-logofail-firmware-attack/)<br>
Many UEFI packages can be exploited by modifying the boot image. - Eric

- [**Lessons Learned by a Software Guy Venturing into Hardware – SidecarT – Atari ST/STE/Mega cartridge emulator on Raspberry Pi Pico steroids**](https://sidecartridge.com/blog/2023/11/01/lessons-learned-by-software-guy-venturing-into-hardware/)<br>
This fun article details a software engineer's journey to building hardware. They built a ROM cartridge emulator using the Raspberry Pi RP2040.

- [**3GPP specification series: 36series**](https://www.3gpp.org/dynareport?code=36-series.htm)<br>
A convenient link to all the 3GPP documents in the 36 series (the series on LTE). - Gillian

- [**LTE-M vs NB-IoT Field Test: How Distance Affects Power Consumption - Blogs - Nordic Blog - Nordic DevZone**](https://devzone.nordicsemi.com/nordic/nordic-blog/b/blog/posts/ltem-vs-nbiot-field-test-how-distance-affects-power-consumption)<br>
Neat results from an experiment comparing how distance from an eNB affects power consumption for devices using LTE-M vs. NB-IoT. - Gillian

- [**My personal C coding style as of late 2023**](https://nullprogram.com/blog/2023/10/08/)<br>
A somewhat 🌶️ blogpost detailing a C style that had some interesting but maybe impractical suggestions. Discussing it internally though revealed how some very important language constructs like `const` matter so much more in embedded environments than others. - Eric

- [**Using Zig to Unit Test a C Application · mtlynch.io**](https://mtlynch.io/notes/zig-unit-test-c/)<br>
A relatively simple example of wrapping a C library with Zig and then using Zig’s built-in testing features to test. Still a long way from MCU targets but it feels like we’re getting glimpses. - Eric

- [**Exploring the section layout in linker output | MaskRay**](https://maskray.me/blog/2023-12-17-exploring-the-section-layout-in-linker-output)<br>
Article covering how the linker section layout works with dynamic loading. Specific to x86-64, but is a good read nonetheless.


## Projects & Tools
- [**Zeus WPI | Unveiling secrets of the ESP32: creating an open-source MAC Layer**](https://zeus.ugent.be/blog/23-24/open-source-esp32-wifi-mac/)<br>
The ESP32 is one of the most popular and cheapest WiFi chips out there. This article dives into creating an open-source MAC layer for the ESP32.

- [**cnlohr/rv003usb: CH32V003 RISC-V Pure Software USB Controller**](https://github.com/cnlohr/rv003usb)<br>
Bit-banged USB on a very cheap microcontroller. - Noah

 - [**floppy.cafe**](https://floppy.cafe/)<br>
Bit-banging a floppy drive (in rust!)! Find the project code [here](https://github.com/SharpCoder/floppy-driver-rs). - Noah

- [**EYE on NPI - Analog Devices LTC4332 Point-to-Point Rugged SPI Extender #EYEonNPI @DigiKey @ADI_News - YouTube**](https://www.youtube.com/watch?v=D6MxyfDtWqE)<br>
Cool SPI range extender chip, up to 1200m over twisted pair. Neat tristate speed select using 2 pins, provides 8 speed options from strapping configuration.

- [**Forget Houses! Check Out This Gingerbread ESP32 Dev Board - Hackster.io**](https://www.hackster.io/news/forget-houses-check-out-this-gingerbread-esp32-dev-board-bf265ab25b3d.amp)<br>
A gingerbread esp32! - Noah

- [**AtomVM**](https://www.atomvm.net//)<br>
And Erlang-based VM for embedded devices, including the ESP32. I personally don't write in Erlang, but maybe someone out there does!

- [**The Berry Script Language**](https://berry-lang.github.io/)<br>
A scripting language for embedded devices, using as little as 40K code size and 4K heap. Feels very similar to the various JavaScript engines that are running on Cortex-M devices!


## News & Announcements
- [**Quintauris: advancing the adoption of RISC-V globally**](https://www.quintauris.eu/)<br>
RISC-V is going forward. :) - Martin Lampacher, Interrupt Author
 

## Upcoming Events
- [**FOUNDERS & FRIENDS at CES 2024**](https://www.mistywest.com/founders-friends/ces-2024/)<br>
Will you be at CES 2024? Request an invite to attend [FOUNDERS & FRIENDS](https://www.mistywest.com/founders-friends/ces-2024/) on Thursday, January 11th, hosted by Misty West and Memfault. Enjoy cocktails and appetizers while networking with some of the hardware industry’s key decision-makers, and get an exclusive preview of some of the latest advancements in cutting-edge technology.
