---
title: "What we've been reading in December (2020)"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

Happy New Year! Here's to a wonderful 2021. 

Here are the articles, videos, and tools that we've been excited about this
December.

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).

## Articles & Learning

- [ACE: Apple Type-C Port Controller Secrets | Part 1](https://blog.t8012.dev/ace-part-1/) by mrarm and Aun-Ali Zaidi<br>
  Detailed report about reverse engineering the Apple Type-C Port Controller, which happens to include a Cortex-M0. 
- [PokéWalker hacking](http://dmitry.gr/?r=05.Projects&proj=28.%20pokewalker) by Dmitry Grinberg<br>
  Another stellar post by Dmitry, this time reverse-engineering a [PokéWalker](https://nintendo.fandom.com/wiki/Pok%C3%A9walker) device. He learns that he can dump the entire ROM of the device over infrared! Be sure to read the final part about how you can hack around with a PokéWalker using a PalmOS device.
- [Quick Peek of PineCone BL602 RISC-V Evaluation Board](https://lupyuen.github.io/articles/pinecone) by Lup Yuen Lee<br>A good read on why Lup Yuen Lee believes that the PineCone BL602 could be a future player in the MCU world, comparing it to the STM32 Blue Pill, nRF52, ESP32, etc. It is a new device based upon RISC-V from Bouffalo Lab. Among other posts, his most recent one is about him [porting MyNewt to it](https://lupyuen.github.io/articles/mynewt).
- [OSFC 2020 Talks](https://vimeo.com/showcase/osfc-2020)<br>
  The Open Source Firmware Conference has published all of this year's talks!
- [Understanding the SAM D21 clocks](https://blog.thea.codes/understanding-the-sam-d21-clocks/) by Stargirl Flowers<br>
  An introduction to the SAM D21's clock system by the author of one of my favorite Python packages ([nox](https://nox.thea.codes/en/stable/)).
- [Bit Twiddling Hacks](https://graphics.stanford.edu/~seander/bithacks.html) by Sean Eron Anderson<br>
  The standard, CSS-less webpage with HTML gold detailing every little bit twiddling code snippet your heart could dream of.
- [Black Hat Europe 2020 - Debug Resurrection On nRF52 Series](https://limitedresults.com/2020/12/black-hat-europe-2020/) by LimitedResults<br>
  A talk with slides from Black Hat Europe 2020, documenting a complete compromise of the flash readout protection on
  nRF52 series microcontrollers using simple voltage glitching techniques.
- [Trying to test a "ten cent" tiny ARM-M0 MCU](http://nerdralph.blogspot.com/2020/12/trying-to-test-ten-cent-tiny-arm-m0-mcu.html) by Ralph Doncaster<br>
  Upon finding an MCU matching the pinout of the STM8S003 for 10-20 cents, Ralph orders a few of them and details his experience getting them up and running using Openocd.
- [Thread Local Error Contexts in Embedded C++](https://blog.stratifylabs.co/device/2020-12-14-Thread-Local-Error-Context-in-Cpp/) by Stratify Labs<br>
  A great article (and series) about storing global, per thread error state so that errors can be traced back to their original sources. Read the entire series, it's pretty quick!
- [What's in a Firmware Load?](https://www.security-embedded.com/blog/2016/7/11/whats-in-a-firmware-load) by Phil Vachon<br>
  Found a new blog with some great content! This security-focused post is about digging into exactly what a firmware payload looks like for an IoT device based upon a Marvell Cortex-M3 MCU.
- [Take snapshots of current screen](https://blog.lvgl.io/2019-04-25/snapshots) by beibean<br>
  How does one do unit or regression testing with a graphical interface? One strategy is to take "screenshots" of the frame buffer and diff the two images. You can do exactly this with [LVGL](https://lvgl.io/), a popular graphics library for embedded devices, and this post explains how to do it. 
- [How I built my first Azure RTOS GUIX display driver](https://www.olivierbloch.com/post/how-i-built-my-first-azure-rtos-guix-display-driver) by Olivier Bloch<br>
  Using ThreadX's GUIX framework, Olivier documents how he build a display driver for a "flip board" display, like the ones you see in train stations.
- [Foundations of Embedded Systems](https://f-of-e.org/)<br>
  A mishmash of content related to embedded systems and how they interact with the world around them. The site is created and run by [Phillip Stanley-Marbell](https://www.phillipstanleymarbell.org/) of University of Cambridge. I haven't had a chance to check out the content, but it looks pretty good.


## Neat Projects
- [Canable - An Open-Source USB to CAN Adapter](https://canable.io/)<br>
  A firmware and device which can connect to your computer (Mac/Windows/Linux) as either a serial device or as a CAN device.

## Random

- [Cameras and Lenses](https://ciechanow.ski/cameras-and-lenses/) by Bartosz Ciechanowski<br>
  An incredible post explaining how cameras and lenses work! (In no way related to firmware, but had to share it.)
