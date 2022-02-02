---
title: "What we've been reading in February (2021)"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

Here are the articles, videos, and tools that we've been excited about this
February.

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).

## Articles & Learning

- [ESP32 Secure firmware update Over-The-Air (OTA)](https://www.lab4iot.com/2021/02/21/esp32-secure-firmware-update-over-the-air-ota/) by Refik Hadzialic<br>
A nice end-to-end prototype of updating the ESP32 with an NGINX server, covering all the details from DNS and SSL certificates to the required ESP32 firmware modifications.
- [Explaining Changes post-firmware 2019.16 Regarding Range Loss](https://skie.net/skynet/projects/tesla/view_post/23_Explaining+Changes+post-firmware+2019.16+Regarding+Range+Loss) by wk057<br>
An interesting read that I found crawling Hackernews about why some Tesla battery packs would lose range while the car was sitting idle. Long story short, Tesla had a hardware bug in their battery management system modules, and there were able to perform iterative OTA updates to the car to cleverly mitigate an otherwise drastic issue.
- [Process isolation with Arm FuSa runtime system](https://community.arm.com/developer/tools-software/tools/b/tools-software-ides-blog/posts/process-isolation-with-fusa-rts) by Vladimir Marchenko<br>
An overview of Arm's FuSA (Functional Safety runtime System). Not entirely sure about the quality or usability of the system, but I appreciate the modern features they are providing! Process isolation, task watchdogs [relevant]({% post_url 2020-02-18-firmware-watchdog-best-practices %}), and better fault handling. Happy to see these features are becoming commonplace. 
- [Testing a Hardware Abstraction Layer (HAL)](https://ferrous-systems.com/blog/defmt-test-hal/) by Ferrous Systems<br>
An introduction to testing a HAL using Embedded Rust! The post covers testing GPIO's, a UART, and more. Exciting to see Ferrous Systems wrangling this difficult problem.
- [Dumb Embedded System Mistakes: Running The Wrong Code](https://www.embeddedrelated.com/showarticle/1389.php) by Steve Branam<br>
Steve talks about a common mistake developers make...loading the wrong code on the device. The mitigation strategies taken involve embedded and surfacing versions and tags and making sure developers see them. 
- [How to use custom partition tables on ESP32](https://medium.com/the-esp-journal/how-to-use-custom-partition-tables-on-esp32-69c0f3fa89c8) by Pedro Minatel<br>
An overview of how to partition the flash on an ESP32 to allow for multiple OTA images and other information that you want to keep across reboots, including an example project that you can easily load onto your own board.
- [Emprog ThunderBench - Linker Script Guide](https://www.phaedsys.com/principals/emprog/emprogdata/thunderbench-Linker-Script-guide.pdf)<br>
One of the better linker script guides that we've come across. 
- [Unnamed Reverse Engineering Podcast](https://unnamedre.com/)<br>
Can't believe we haven't linked this podcast before! [Alvaro](https://twitter.com/alvaroprieto) and [Jen](https://twitter.com/rebelbotjen) talk to various people broadly about reverse engineering and firmware. 
- [Teardown of a quartz crystal oscillator and the tiny IC inside ](https://www.righto.com/2021/02/teardown-of-quartz-crystal-oscillator.html) by Ken Shirriff<br>Ken breaks down exactly what is happening underneath the covers of an oscillator.
- [Mars Helicopter Technology Demonstrator](https://trs.jpl.nasa.gov/bitstream/handle/2014/46229/CL%2317-6243.pdf) by J. (Bob) Balaram<br>
A deep-dive into the technical details of the Mars Helicopter that just landed in the last month. It runs embedded Linux and uses Zigbee. So cool.
- [Tutorial - Write a Shell in C](https://brennan.io/2015/01/16/write-a-shell-in-c/) by Stephen Brennan<br>
Stephen covers how one can build a shell from scratch, which, brings me back to the days of a college computer science project. This could be useful for when you inevitably have to include a minimal shell into a new firmware project. 
- [Creating GUIs with LVGL](https://www.arduino.cc/pro/tutorials/portenta-h7/por-ard-lvgl)<br>
How to build a GUI using LVGL and an Arduino Portenta H7 connected to a display with HDMI.
- [A Practical Approach To Attacking IoT Embedded Designs (Pt. 1)](https://labs.ioactive.com/2021/02/a-practical-approach-to-attacking-iot.html) by  Ruben Santamarta<br>
Ruben covers various security vulnerabilities in popular RTOS's and libraries, such as stack and buffer overflows and memory corruption. Do note that all of these issues were patched at the time the article was written. 

## Neat Projects

- [InfiniTime - Pinetime Firmware](https://github.com/JF002/InfiniTime)<br>
We've posted the Pinetime smartwatch here before, but never the InifiTime firmware itself. The firmware now has call notifications, vibrations, heart rate reading capabilities, and LVGL v7. It's really starting to take shape!
- [Whitefield](https://github.com/whitefield-framework/whitefield)<br>
"Whitefield provides a simulation environment for sensor networks by combining realistic PHY/MAC layer simulation with the native mode use of popular IoT stacks/OSes such as Contiki/RIOT/OpenThread/Zephyr/FreeRTOS/OT-RTOS."
- [Wireguard-lwIP](https://github.com/smartalock/wireguard-lwip)<br>
A [WireGuard](https://www.wireguard.com/) implementation for [lwIP](https://www.nongnu.org/lwip/2_1_x/index.html) written in C and optimized for embedded systems. 
- [theacodes/Structy](https://github.com/theacodes/structy)<br>
Don't really want to try to paraphrase the description. "Structy is an irresponsibly dumb and simple struct serialization/deserialization library for C, Python, and vanilla JavaScript". Think Protobuf and friends are too complicated? Check this out.


## Random!

- [Name that Ware (Blog)](https://www.bunniestudios.com/blog/?cat=54) by bunnie<br>
A series, "Name that Ware", in which bunnie posts an image of a circuit board or inside of a device and readers get to guess what the picture is taken of. The articles on the blog date back to 2015, but it seems to have been taking place for much longer.
- [Opinions after a decade of professional software engineering](https://blog.thea.codes/opinions-after-a-decade/) by Stargirl Flowers<br>
It's always helpful to hear from experienced engineers about what they've learned and what they would do differently. This time, Stargirl. My favorite quote: "Code isn't even close to [the] most important thing about software engineering."
- [Procedure Call Standard for the Arm® Architecture (Github)](https://github.com/ARM-software/abi-aa/blob/master/aapcs32/aapcs32.rst)<br>
Unbeknownst to us at Memfault, Arm recently moved their ABI spec to Github!
