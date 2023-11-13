---
title: "What we've been reading in October (2023)"
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

- [**UBfuzz: Finding Bugs in Sanitizer Implementations**](https://shao-hua-li.github.io/files/2024_ASPLOS_UBFUZZ.pdf)<br>
Detecting bugs in sanitizers with a fuzz- based approach. The caption below stuck with me. - Noah
>GCC -O2 optimizes away the UB code, thus ASan cannot discover the UB.

- [**Buffer overflow detection help with GCC | Red Hat Developer**](https://developers.redhat.com/articles/2021/06/25/use-source-level-annotations-help-gcc-detect-buffer-overflows)<br>
Nice tips for annotating C function buffer parameters. - Noah

- [**Post Series: ELF Format and Runtime Internals + Heap Exploitation and GlibC Internals**](https://blog.k3170makan.com/p/series.html)<br>
This blog has a nice series on the internals of ELF files. The author does a nice job of highlighting the raw hex and outlining what each byte belongs too. There’s also some cool applications to reverse engineering and exploits. - Eric

- [**OneVariable : interrupts is threads**](https://onevariable.com/blog/interrupts-is-threads/)<br>
Post on how embedded rust models interrupts as threads (and how that translates into thread-safety guarantees at compile time). Worth a read even if you don't use Rust, as the core concepts are the same in C/C++. Written by Interrupt community member [James Munns](https://jamesmunns.com/blog/). 

- [**789 KB Linux Without MMU on RISC-V**](https://popovicu.com/posts/789-kb-linux-without-mmu-riscv/)<br>
The title says it all! - Noah

- [**Diving into JTAG. Part 3 — Boundary Scan | by Aliaksandr Kavalchuk | Oct, 2023 | Medium**](https://medium.com/@aliaksandr.kavalchuk/diving-into-jtag-part-3-boundary-scan-17f9975ecc59)<br>
Part 3 of a JTAG deep dive series which explores JTAG Boundary-Scan, written by Aliaksandr Kavalchuk; use this approach for testing interconnects on PCBs and internal IC sub-blocks.

- [**Getting Started With Zephyr: Devicetree Overlays - Mohammed Billoo**](https://www.embeddedrelated.com/showarticle/1583.php)<br>
Gain insight into Devicetree overlays and their utilization within Zephyr with this tutorial from Memfault Ambassador [Mohammed Billoo](https://mab-labs.com/).

- [**CI/CD for Embedded with VS Code, Docker and GitHub Actions | MCU on Eclipse**](https://mcuoneclipse.com/2023/10/02/ci-cd-for-embedded-with-vs-code-docker-and-github-actions/)<br>
 Another great post from MCU on Eclipse. Discover how to set a CI/CD pipeline using Visual Studio Code, Docker and GitHub actions with the NXP LPC55S16-EVK as an example.

- [**Programming embedded systems: input-driven state machines - Embedded.com**](https://www.embedded.com/programming-embedded-systems-input-driven-state-machines/)<br>
Confused about state machines? Check out this comprehensive article from the folks over at [Quantum Leaps](https://www.state-machine.com/). If you prefer to watch instead of read, you're in luck: view the [corresponding video](https://youtu.be/E2Im7jLDDG4).

- [**Leveraging Your Toolchain to Improve Security - Embedded Artistry**](https://embeddedartistry.com/blog/2023/09/20/leveraging-your-toolchain-to-improve-security/)<br>
Our friends over at [Embedded Artistry](https://embeddedartistry.com/) released a helpful guide for incorporating security early on in the development process by leveraging your toolchain.


## Projects & Tools
- [**USB-C cable tester - C2C caberQU**](https://caberqu.com/home/20-42-c2c-caberqu-746052578813.html#/26-case-without_case)<br>
Neat board for testing USB C-C cables! - Noah

- [**Linkable Loadable Extensions (LLEXT) — Zephyr Project Documentation**](https://docs.zephyrproject.org/latest/services/llext/index.html)<br>
Pretty sweet new feature just landed in Zephyr - runtime loadable code. Details [here](https://docs.zephyrproject.org/latest/services/llext/index.html) and [sample shell](https://docs.zephyrproject.org/latest/samples/subsys/llext/shell_loader/README.html), recommended by Interrupt community member Jonathan Beri of [Golioth](https://golioth.io/).


## News & Announcements
- [**Introducing Zephyr 3.5 - Zephyr Project**](https://www.zephyrproject.org/introducing-zephyr-3-5/)<br>
Zephyr 3.5.0 is now generally available! This [Zephyr Tech Talk #004](https://www.youtube.com/watch?v=D9Ukvuyp2XU) covers many of the new features and improvements. There is also a new [Migration Guide](https://docs.zephyrproject.org/latest/releases/migration-guide-3.5.html) to help you migrate your application from 3.4 to 3.5.



## Upcoming Events

- [**2023 IoT Online Conference (November 14-16)**](https://www.iotonlineconference.com/)<br>
The IoT Online Conference is back from November 14-16 and will feature talks from Memfault's Gillian Minnehan on GDB Deep Dive and Tyler Hoffman on Objectively Measuring the Reliability of IoT Devices. If you register now using our promo code, you will save 50% on registration fees! Use promo code: **MEMFAULTIOT** to drop the price from $190 to $95. [Register here.](https://www.iotonlineconference.com/register.php)

- [**Memfault Webinar - NXP + Memfault + Golioth: Bringing Observability and Device Management to IoT Devices (November 16)**](https://hubs.la/Q026Gsz30)<br>
Join us on Thursday, November 16th with [NXP](https://www.nxp.com/) and [Golioth](https://golioth.io/) for a joint webinar, as we dive into the future of IoT device management and observability. Can't make it? Register [here](https://hubs.la/Q026Gsz30) to get the recording emailed to you afterward. 

- [**Berlin Firmware Meetup by Memfault (November 15)**](https://go.memfault.com/berlin-firmware-meetup-q42023)<br>
Team Memfault is hosting another Berlin Firmware Meetup on Thursday, September 14th at [MotionLab.Berlin](https://motionlab.berlin/). Join us for a fun evening of building connections with other like-minded engineers and exploring the latest trends in embedded systems. Sign up [here](https://go.memfault.com/berlin-firmware-meetup-q42023) for your free ticket.

- [**Boston Firmware Meetup by Memfault (November 16)**](https://go.memfault.com/boston-firmware-meetup-q42023)<br>
Team Memfault would be thrilled to host you on Thursday, November 16th for food, beer, and to talk firmware! Come hang out with team members from the Boston office and make new connections with local embedded engineers. Get your free ticket [here](https://go.memfault.com/boston-firmware-meetup-q42023).

- [**San Diego Firmware Meetup by Memfault (November 29)**](https://go.memfault.com/sandiego-firmware-meetup-q42023)<br>
Join Team Memfault on Wednesday, November 29th at the San Diego Firmware Meetup, which will include technical talks, demos, and networking over food & drinks! Whether you’re a seasoned engineer or just starting out in the field, come connect with like-minded individuals. Register for your [free ticket](https://go.memfault.com/sandiego-firmware-meetup-q42023).

- [**Connectivity Standards Alliance Members Meeting | Geneva, Switzerland (November 13-16)**](https://csa-iot.org/event/alliance-member-meeting-geneva-switzerland/)<br>
Are you a member of the Connectivity Standards Alliance? We'll be at the Members Meeting in Geneva from November 13-16 with a tabletop exhibition that you can swing by for an interactive demo and neat swag - [come say hi](https://csa-iot.org/event/alliance-member-meeting-geneva-switzerland/))!
