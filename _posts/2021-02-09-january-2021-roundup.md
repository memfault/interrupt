---
title: "What we've been reading in January (2020)"
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

- [The most thoroughly commented linker script (probably)](https://blog.thea.codes/the-most-thoroughly-commented-linker-script/) by Stargirl<br>
Title says it all.
- [Ending the Embedded Software Dark Ages: Let’s Start With Processor Fault Debugging!](https://embeddedartistry.com/blog/2021/01/11/hard-fault-debugging/) by Phillip Johnston at Embedded Artistry<br>Phillip dives deep into the fault handling routines of the ARM Cortex-M and investigates how a developer can go beyond manual debugging by using coredumps (so exciting!).
- [A 27th IOCCC Winner - Best of show - abuse of libc](https://www.ioccc.org/2020/carlini/index.html) by Nicholas Carlini<br>
TIL that `printf` is Turing complete. In this IOCCC entry, the author implements Tic-Tac-Toe in a single `printf` statement! 
- [Better Embedded System Software e-Book & Paperback](https://betterembsw.blogspot.com/2021/02/better-embedded-system-software-e-book.html?m=1) by Phil Koopman<br>
Phil has updated his popular embedded systems book with what he calls a v1.1 edition. The hardcover was prohibitivly expensive for some (but worth it), and this new version comes in both a paperback book and digital version, costing $25 and $10 respectively, which is a steal. The group in the Interrupt Slack channel say it's a worthwhile purchase for hte new price.
- [Thoroughly Commented Cortex M0+ Micro Trace Buffer interface (Twitter)](https://twitter.com/theavalkyrie/status/1353925165072207872?s=21) by Stargirl<br>
The Micro Trace Buffer on the Cortex-M0+ (and the ETB) are some notoriously difficult modules to get set up, and they are rarely included by chip vendors in a package. A few of Stargirl's hardware products under the brand [Winterbloom](https://winterbloom.com/) use the SAMD21, which *does* include the MTB! In the Tweet above, she links to the MTB initialization code in the firmware.
- [Linux and Zephyr, Sitting in a tree (slides)](http://christopher.biggs.id.au/talk/2021-01-24-linux-zephyr/) by Christopher Biggs<br>
Tips, tricks, and lessons learned from using Zephyr on a Raspberry Pi. Christopher covers why someone would want to use an RTOS instead of Embedded Linux, how to get things building and flashing, and gives some final tips based on a real-world project.
- [VSCode, Dev Containers and Docker: moving software development forward](https://blog.feabhas.com/2021/01/vscode-dev-containers-and-docker-moving-software-development-forward/) by Niall Cooling<br>
Niall covers how to get started unit testing with Ceedling, GoogleTest, and VScode using VSCode's built-in Docker container integration. Be sure to check out his [following post](https://blog.feabhas.com/2021/01/github-codespaces-and-online-development/) on Github Workspaces.
- [Schematic Design Review Checklist](https://blog.stratifylabs.co/device/2021-01-05-Schematic-Design-Review-Checklist/) by Stratify Labs<br>
You can tell this article (and the [PCB Layout Checklist](https://blog.stratifylabs.co/device/2021-01-05-PCB-Layout-Design-Review-Checklist/)) are both written from true experience. Check these out if you are in the process of designing or building your own boards, whether they are mounted on Legos or not.
- [Unit Testing with PlatformIO: Part 1. The Basics](https://piolabs.com/blog/insights/unit-testing-part-1.html) by Valerii Koval<br>
A thorough walkthrough of how to use the built-in [Unity](http://www.throwtheswitch.org/unity) unit testing functionality with PlatformIO and its VSCode plugin.

## Neat Projects

- [Winterbloom - Castor & Pollux Firmware](https://github.com/theacodes/Winterbloom_Castor_and_Pollux)<br>
As mentioned above, this is the Github repo for a Winterbloom oscillator, complete with everything you could ask for! Factory scripts, hardware and software code, and documentation. A very good resource for someone wanting to look at an open-source Cortex-M0+ based firmware.
- [SPIFFS (SPI Flash File System)](https://github.com/pellepl/spiffs)<br>
A file system built for SPI NOR flash devices. It might be a good alternative to [LittleFS](https://github.com/littlefs-project/littlefs).
- [bytefield-svg](https://bytefield-svg.deepsymmetry.org/bytefield-svg/1.5.0/intro.html)<br>
An SVG library to help build byte-field diagrams, such as those showing network protocols and memory layouts. A similar package to [WaveDrom](https://wavedrom.com/).
- [PCB Brick Mount](https://lab.jamesmunns.com/projects/brick-mount.html#brick-mount) by Kerry Scharfglass<br>
Ever thought layout out multiple PCB's together was a pain? James Munns did too, and he came up with a Lego-like system to mount PCB's onto. Check out [his Github repo](https://github.com/jamesmunns/brick-mount) which contains KiCAD footprints and the various boards James has designed himself.
- [Siliconpr0n](https://siliconpr0n.org/map/)<br>
A collection of die images of a plethora of chips.
- [aosp-riscv](https://github.com/T-head-Semi/aosp-riscv)<br>
Android running on RiscV!? Neat.
- [Watchy Pebble-like Smartwatch with E-paper display, ESP32 processor launched on Crowd Supply](https://www.cnx-software.com/2021/02/04/watchy-pebble-like-smartwatch-with-e-paper-display-esp32-processor/)<br>
Watchy, an ESP32-based E-paper smartwatch board that is being sold on [CrowdSupply](https://www.crowdsupply.com/sqfmi/watchy). I love this because it means that I might finally get to wear something Pebble-like on my wrist again!

## News

- [Raspberry Pi Pico](https://www.raspberrypi.org/products/raspberry-pi-pico/)<br>
A fascinating new announcement from the group behind the Raspberry Pi, the Pico! The RP2040 features a dual-core ARM Cortex-M0+, 264kB of RAM, and 16MB of flash. Did I mention it's $4 and it runs MicroPython? Sign me up. I imagine this will be a very popular prototyping board for new hardware projects.
- [NXP published MCUXpresso SDK 2.9.0 on GitHub](https://mcuoneclipse.com/2021/01/24/nxp-published-mcuxpresso-sdk-2-9-0-on-github/) by Erich Styger<br>
NXP has realized that providing .zip files of their SDK is outdated and cumbersome to use. Yay! They've also published it under a permissible license, BSD-3. Double yay! I wish all vendors take notes.
- [EdgeLock 2GO - Secure, flexible IoT service platform ](https://www.nxp.com/products/security-and-authentication/secure-service-2go-platform/edgelock-2go:EDGELOCK-2GO)<br>
NXP announced their IoT platform, EdgeLock 2GO. I'd love to know who's using this. (Do reach out if you are, I'm very curious.)
- [This Bluetooth Attack Can Steal a Tesla Model X in Minutes](https://www.wired.com/story/tesla-model-x-hack-bluetooth/) by Andy Greenberg<br>
It wouldn't be an Interrupt blogroll post without a new Tesla hack over Wi-Fi or Bluetooth.