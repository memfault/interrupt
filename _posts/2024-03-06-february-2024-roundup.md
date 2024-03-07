---
title: "What we've been reading in February (2024)"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

Here are the articles, videos, and tools that we've been excited about this
February. 

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).


## News & Announcements
- [**Memfault's Launch Week**](https://go.memfault.com/memfault-launch-week-march-2024)<br>
We have been hard at work building new features to make it easier to identify and address issues while your devices are in the field. Join Memfault's week-long celebration of our newest release, which will be marked by a webinar and accompanying videos and blog posts. Check out the agenda and join our newsletter to stay up to date on all release-related activities!  

- [**STMicro ST60A3H0 and ST60A3H1 60 GHz transceiver ICs aim to replace USB cables - CNX Software**](https://www.cnx-software.com/2024/02/22/stmicro-st60a3h0-and-st60a3h1-60-ghz-transceiver-ics-aim-to-replace-usb-cables/?amp=1)<br>
60GHz wireless USB transceivers from ST Micro. - Noah

- [**Welcome, Zephyr 3.6! - Zephyr Project**](https://www.zephyrproject.org/welcome-zephyr-3-6/)<br>
Zephyr 3.6 is here! [Click here to read the full release notes](https://docs.zephyrproject.org/latest/releases/release-notes-3.6.html) and transition to 3.6 using the accompanying [migration guide](https://docs.zephyrproject.org/latest/releases/release-notes-3.6.html). 


## Articles & Learning
- [**oss-security - Out-of-bounds read & write in the glibc's qsort()**](https://www.openwall.com/lists/oss-security/2024/01/30/7)<br>
A tricky out-of-bounds access vulnerability in glibc `qsort` implementation that has been lurking since at least 1992! There are some really great details on why this problem exists and how it can easily be overlooked thanks to subtraction overflows. - Eric

- [**Visual overview of a custom malloc() implementation — The silent tower**](https://silent-tower.net/projects/visual-overview-malloc#fnref:2-Mmap)<br>
An approachable overview of how a `malloc` implementation works with some accompanying visuals. - Eric

- [**Microarch Club**](https://microarch.club/)<br>
Microarch Club: The art, science, and history of processor design - a new podcast from our friend Daniel Mangum at [Golioth](https://golioth.io/). - Eric

- [**Reverse-engineering an encrypted IoT protocol | @smlx's blog**](https://smlx.dev/posts/goodwe-sems-protocol-teardown/)<br>
An interesting project overview that decodes the encrypted protocol used by GoodWe smart meters and solar inverters to send metrics to the cloud, and then uses a Prometheus exporter to allow local monitoring of metrics. - François

- [**Changing an ESP32 partition table over the air | by Florian Loitsch | The Toit Take**](https://blog.toit.io/changing-an-esp32-partition-table-over-the-air-276c86feeba8)<br>
One solution for a very thorny problem with ESP-IDF: updating a partition table! - Eric

- [**Doom running on Silicon Labs & Sparkfun Microcontrollers: A Quick Look**](https://community.silabs.com/s/share/a5UVm00000002Y5MAI/doom-running-on-silicon-labs-sparkfun-microcontrollers-a-quick-look?language=en_US&source=Email&detail=Newsletter&cid=eml-new-blu-022224&mkt_tok=NjM0LVNMVS0zNzkAAAGRcTWjktUpLJn8KTdKnqQPvKeWCtC_jHippIHL15RT0YNDIggH6jJYLXt44qTMDUsFieDGewcR4YbpO-WLcDfmY63FZBSRbMQZHMD9KZaFSJRCJTc)<br>
Who doesn’t love a “We put Doom on an embedded device” post? - Eric

- [**M0AGX / LB9MG - Abusing reserved interrupt vectors on Cortex-M for metadata**](https://m0agx.eu/cortex-m-reserved-vectors-as-metadata.html)<br>
Follow this example of using a simple, small, single CRC32 checksum appended to a raw binary file for easy verification in bare-metal bootloaders, as opposed to the elaborate file formats of OS executables.

- [**The State of Developer Ecosystem in 2023 Infographic | JetBrains**](https://www.jetbrains.com/lp/devecosystem-2023/embedded/)<br>
JetBrains published their 2023 developer survey, and there's a way to filter it for embedded engineers. I love to use CLion, and I'm glad they are focusing more on embedded!

- [**USB in a NutShell - Making sense of the USB standard**](https://www.beyondlogic.org/usbnutshell/usb1.shtml)<br>
Oldie but a goodie; it is probably the best USB resource I’ve used that’s not digging into the standard. - Eric

- [**Board Porting Guide — Zephyr Project Documentation**](https://docs.zephyrproject.org/latest/hardware/porting/board_porting.html#transition-to-the-current-hardware-model)<br>
There’s a new Zephyr change for how users define their hardware that will be in v3.7 LTS, and this migration guide helps move to these new features. - Eric

- [**Get up to speed with partial clone and shallow clone - The GitHub Blog**](https://github.blog/2020-12-21-get-up-to-speed-with-partial-clone-and-shallow-clone/)<br>
A good explainer of partial and shallow clones, how they work, and when to use different forms. Great to help speed up downloading large vendor HALs. - Eric

- [**What is Name Mangling in C++? – Abstract Expression**](https://abstractexpr.com/2023/01/03/what-is-name-mangling-in-cpp/)<br>
Great explanation of name mangling (encoding a name and a signature to a symbol name) and how to demangle.

- [**Making the most of your Shells History on Linux and macOS – Abstract Expression**](https://abstractexpr.com/2024/02/18/making-the-most-of-your-shells-history-on-linux-and-macos/)<br>
Awesome tips on utilizing powerful history features that will help you to become much faster with a shell.

- [**A journey of improvements to Neurosity’s Brain Operating System | by Bruno de Carvalho**](https://medium.com/@biasedbit/a-journey-of-improvements-to-neurositys-brain-operating-system-8ef6f9af11ac)<br>
A fun exploration of an embedded software project running on Rust for mental health wearables.

- [**In Rust we trust? White House Office urges memory safety - Stack Overflow**](https://stackoverflow.blog/2024/03/04/in-rust-we-trust-white-house-office-urges-memory-safety/)<br>
Programming languages such as C and C++ are under scrutiny from the [White House](https://www.whitehouse.gov/oncd/briefing-room/2024/02/26/memory-safety-fact-sheet/) for memory vulnerability reasons - is Rust the answer? 

- [**New Hardware Model / Zephyr Meetup Cologne - YouTube**](https://www.youtube.com/watch?v=lt-bioPbZgw)<br>
At a recent Zephyr Meetup, Johann Fischer from Nordic Semiconductor discusses the challenges of setting up hardware for specific architectures, tackling issues with multiple boards, and advocates for a more adaptable and structured approach.

- [**Live community Q&A - Zephyr 3.6 Release // Zephyr Tech Talk #013 - YouTube**](https://www.youtube.com/watch?v=ay22XeIlWA0&t=2s)<br>
Catch the highlights of Zephyr's 3.6 release, including new features and improvements, with the release managers and some of the maintainers and contributors. 


## Projects & Tools

- [**meshtastic/firmware: Meshtastic device firmware**](https://github.com/meshtastic/firmware)<br>
An open-source, decentralized mesh network that lets you use LoRa radios as a long-range off-grid communicator for areas without reliable cellular service.


## Upcoming Events
- [**Memfault Webinar - Measure Embedded Device Quality in the Field with Ease**](https://hubs.la/Q02mg8Tc0)<br>
Join Memfault's product launch webinar on [**Tuesday, March 12th**](https://hubs.la/Q02mg8Tc0) led by François Baldassari as he shares the three essential metrics to monitor once your device is in the field: software stability, battery health, and connectivity. You don't want to miss this product release!

- [**Silicon Valley Firmware Meetup - Thu, Mar 21, 2024**](https://www.eventbrite.com/e/silicon-valley-firmware-meetup-tickets-846305822497?aff=oddtdtcreator)<br>
You're invited to join our upcoming [**Silicon Valley Firmware Meetup**](https://www.eventbrite.com/e/silicon-valley-firmware-meetup-tickets-846305822497?aff=oddtdtcreator) on **Thursday, March 21** from 6:00-9:00pm PT!  Whether you're a seasoned engineer or just getting started in the field, come join us and connect with community members over technical talks and refreshments.  
 
- [**Berlin Firmware Meetup - Thu, Apr 4, 2024**](https://www.eventbrite.com/e/berlin-firmware-meetup-tickets-848601318387?aff=oddtdtcreator)<br>
Join us on **Thursday, April 4th** from 18:00 - 21:00 CET for our [**Berlin Firmware Meetup**](https://www.eventbrite.com/e/berlin-firmware-meetup-tickets-848601318387?aff=oddtdtcreator) to kick off the Embedded World festivities! This is the perfect opportunity to connect with other local engineers and learn about the latest developments in the embedded space.

- [**Embedded World 2024: Visit Memfault at Booth 4-238 in Hall 4**](https://hubs.la/Q02kgvHP0)<br>
[Embedded World](https://www.embedded-world.de/en), the world's leading conference for embedded systems, will be back in Nuremberg, Germany from 9-11 April. Come meet the Memfault team at **Booth 4-238 (Hall 4)** for a live demo of Memfault's embedded observability platform and newest product features, grab some limited edition swag, and enter our daily raffle where we're giving away cool prizes like Panic Playdates. We will have special items for Interrupt community members! [Click here to reserve your swag](https://share.hsforms.com/1YlErHzpVT-avI4zPg2SRPA53an2) and let us know you'll be swinging by the booth.

Still need a ticket to Embedded World? Registration is on us: just use Memfault's voucher code **ew24518306** using [**this link**](https://hubs.ly/Q02nrD5R0). Be sure to swing by **Booth 4-238**!


