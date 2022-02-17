---
title: "What we've been reading in September (2020)"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

Here are the articles, videos, and tools that we've been excited about this
August and September.

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments.

## Articles & Learning

- [RIOT Summit 2020](https://www.youtube.com/playlist?list=PLDXXQJiSjPKHpZpiPee7OYaJpUmmfV6Nh)<br>The RIOT-OS RTOS puts on a conference every year, and they just published all the content from this year's gathering.
- [Insights from Writing an Embedded IoT App (Almost) Entirely in Rust](https://embeddedartistry.com/blog/2020/09/21/insights-from-writing-an-embedded-iot-app-almost-entirely-in-rust/) by  Louis Thiery<br>
A post on EmbeddedArtistry detailing the experience and structure of writing an embedded application in Rust.
- [Visualization of stateless UART/SPI exported call sequences](https://lowerstrata.net/post/serial-tracing/) by Aleksandr Koltsoff<br>
A creative post detailing how one can use a spare UART or SPI interface, a simple tracer, and a set of Python scripts to visualize tracing data using Chrome's built-in Trace Event profiling tool.
- [Linux Plumbers Conference 2020 Recordings](https://linuxplumbersconf.org/event/7/page/100-watch-live-free)<br>The Linux Plumbers  from the [2020 Linux Plumbers Conference](https://linuxplumbersconf.org/)<br>
Tons and tons of content from this year's Linux Plumber's conference. Notable recordings relevant to the embedded space are the talk [Using Zephyr, Linux and Greybus for IoT](https://linuxplumbersconf.org/event/7/contributions/814/attachments/672/1284/Using_Linux_Zephyr_and_Greybus_for_IoT_slides.pdf) by Chris Fried and this snippet from one of the GNU Tools Track [talking about GCC's new `-fanalyzer` option](https://youtu.be/oMOH79wpqOw?t=10713) presented by the author of the feature.
- [How to Implement Firmware CI/CD with Docker](https://www.verypossible.com/insights/firmware-ci/cd-with-docker) by Daniel Lindeman<br>
Simple article about using Docker for building and testing by an engineer from Very, an IoT services company behind the [Nerves framework](https://www.nerves-project.org/).
- [What's Wrong with MQTT](https://www.rtautomation.com/mqtt/whats-wrong-with-mqtt/) by John S Rinaldi<br>
John digs into why he believes MQTT isn't the answer that everyone is looking for when it comes to protocols for low power, low bandwidth devices. 
- [A Field Guide to CoAP — Part 1](https://medium.com/@jonathanberi/a-field-guide-to-coap-part-1-75576d3c768b) by Jonathan Beri<br>
Jonathan writes about the Constrained Application Protocol (CoAP) in the first of a four-part series on the topic. A very good overview and is a good follow-up read to the post above.
- [Stopping Bugs: Seven Layers of Defense](https://covemountainsoftware.com/2020/08/16/stopping-bugs-seven-layers-of-defense/) by Matthew Eshleman<br>
A good overview of the layers of the development process where bugs should be *caught*, starting from the requirements and specs to the customer (ouch).
- [Preview PineTime Watch Faces in your Web Browser with WebAssembly](https://lupyuen.github.io/pinetime-rust-mynewt/articles/simulator) by Lee Lup Yuen<br>
Another incredible and detailed post about developing for the PineTime watch. This time, Lee cross-compiles his embedded LVGL-based GUI applications with Emscripten and then into WebAssembly so that he can develop quickly in the browser! The post is complete with a start-to-finish CI workflow built with Github Actions.
- [Introduction to Embedded Linux Security - part 1](https://embeddedbits.org/introduction-embedded-linux-security-part-1/) by Sergio Prado<br>
An excellent overview of Embedded Linux Security fundamentals including encryption, secure boot, and key storage. As mentioned in a previous blogroll post, subscribe to Sergio's blog. It's fantastic.
- [A practical guide for cracking AES-128 encrypted firmware updates](https://gethypoxic.com/blogs/technical/a-practical-guide-for-cracking-aes-128-encrypted-firmware-updates) by Mark Kirschenbaum<br>
A walk-through post about performing a Side Channel Attack (SCA) using [Chipwhisperer Hardware](https://www.newae.com/chipwhisperer). 
- [Cortex-M – Debugging runtime memory corruption](https://m0agx.eu/2018/08/25/cortex-m-debugging-runtime-memory-corruption/)<br>
A great guide on using the Memory Protection Unit (MPU) and Data Watchpoint and Trace Unit (DWT) to catch memory corruption at runtime. The post predates Chris' [post on the topic](https://interrupt.memfault.com/blog/cortex-m-watchpoints).
- [Separation Between WiFi And Bluetooth Broken By The Spectra Co-Existence Attack](https://hackaday.com/2020/08/07/separation-between-wifi-and-bluetooth-broken-by-the-spectra-co-existence-attack/) by Jiska Classen and Francesco Gringoli<br>
A talk given at DEF CON Safe Mode detailing Spectra, a new attack on combo WiFi/Bluetooth chips that can achieve a denial of service attack. 
- [CMU 18-642: Embedded Software Engineering](https://users.ece.cmu.edu/~koopman/lectures/index.html#642)<br>
Completely public course from CMU on embedded software engineering. Worth linking again and again. 
- [DEF CON Safe Mode - Christopher Wade - Beyond Root](https://www.youtube.com/watch?v=aLe-xW-Ws4c&feature=youtu.be)
- [12 things I learned today about linkers](https://jvns.ca/blog/2013/12/10/day-40-12-things-i-learned-today-about-linkers/)
- [Delayed printf for real-time logging](https://www.embeddedrelated.com/showarticle/518.php)
- [Hands-On RTOS with Microcontrollers (book)](https://www.packtpub.com/cloud-networking/hands-on-rtos-with-microcontrollers) by Brian Amos<br>
A new (relatively) book for the embedded ecosystem. The table of contents looks thorough and uses modern software and tools such as an STM32, FreeRTOS, and a JLink.
- [Fun with LCDs and Visual Cryptography](https://justi.cz/security/2020/07/30/lcd-crypto.html) by Max Justicz<br>
- [Mecrisp Forth on STM32 Microcontroller](https://www.youtube.com/watch?v=dvTI3KmcZ7I) by Josh Stone<br>
A video showing Mecrisp Forth running on an STM32F1 purchased from AliExpress for $2. It moves quickly, has a bread board in the bottom left of the screen, and Josh seems to be an Emacs user.
- [CASIO FX-880P Personal Computer Teardown](https://neil.computer/notes/casio-fx-880p-personal-computer-teardown/) by Neil Panchal<br>
A teardown of a classic calculator from the 80's to bring back the nostalgia feelings.
- [Considerations for Updating the Bootloader Over-the-Air (OTA)](https://www.embedded-computing.com/home-page/considerations-for-updating-the-over-the-air-bootloader) by Drew Moseley from Mender<br>
An article discussing high-level concepts for bootloader designs from a simple one, to multi-stage, to Mender's own approach.
- [Using Github Codespaces for Embedded Development](https://www.youtube.com/watch?v=-enIM4x-KPA) by  Benjamin Cabé<br>
A fun video showing how a developer could use the Github Codespaces feature (in beta) to develop for an STM32L4. It looks promising, and if it prevents us all from having to use 20 different IDE's to do our job, that would be a win.


## Neat Open Source Projects

- [Knurling-rs](https://ferrous-systems.com/blog/knurling-rs/) by Ferrous Systems<br>
A new project from the team at Ferrous Systems to bring better tooling to the Embedded Rust ecosystem. I imagine this will shape up to be similar to Platform.io, but targeted towards Rust.
- [microBlocks](https://microblocks.fun/)<br>
MIT's Scratch drag-and-drop coding environment meets embedded. The software development environment works on all major OS's and works on a plethora of boards such as the Adafruit Circuit and Clue, and an ESP32. The repo can be found on [Bitbucket](https://bitbucket.org/john_maloney/smallvm/src/master/).
- [Glasgow Debug Tool](https://github.com/GlasgowEmbedded/glasgow#what-is-glasgow)<br>
"Glasgow is a tool for exploring digital interfaces, aimed at embedded developers, reverse engineers, digital archivists, electronics hobbyists, and everyone else who wants to communicate to a wide selection of digital devices with high reliability and minimum hassle"
- [HeliOS](https://github.com/MannyPeterson/HeliOS)<br>
A simplistic RTOS that places itself between RTOS's and task schedulers. It is small enough to run on the Arduino Uno and can even be installed through the Arduino Library Manager. If you are looking for something smaller that FreeRTOS, this might be it.
- [LVGL](https://lvgl.io/)<br>
A popular graphics library for embedded systems that has it all. Bidirectional text support, internationalization, and subpixel font rendering.
- [PARSEC (Platform AbstRaction for SECurity)](https://github.com/parallaxsecond/parsec)<br>
An attempt to build a common API on top of hardware security and cryptographic services, such as secure elements and HSM's. Contributors include ARM, Docker, and Linaro, adopters include Ranger, Red Hat, and NXP. 
- [mfufont](https://github.com/mcufont/mcufont)<br>
A small and clean font rendering library that is roughly 3kB in code size, has antialiasing and kerning support, and can import .ttf fonts. 

## News

- [Amazon releases more information on Sidewalk Protocol](https://m.media-amazon.com/images/G/01/sidewalk/privacy_security_whitepaper_final.pdf)<br>
Amazon gives us the details on its new Sidewalk (LoRa) network and how it plans to build this network over the millions of Alexa and Ring devices that consumers have purchased. The plan has been in the works for a while because even the 1st-gen Echo Dot (2016) can be a Sidewalk Bridge.
- [Espressif ESP32: Bypassing Encrypted Secure Boot (CVE-2020-13629)](https://raelize.com/posts/espressif-esp32-bypassing-encrypted-secure-boot-cve-2020-13629/)<br>
The final post in Raelize's fault injection research on the ESP32 platform. 
- [Nvidia to buy Arm Holdings from SoftBank for $40 billion](https://www.cnbc.com/2020/09/14/nvidia-to-buy-arm-holdings-from-softbank-for-40-billion.html)<br>We brought our popcorn. We hope you did too. 🍿🍿🍿
- [BLURtooth vulnerability lets attackers overwrite Bluetooth authentication keys](https://www.zdnet.com/article/blurtooth-vulnerability-lets-attackers-overwrite-bluetooth-authentication-keys/)<br>
A new man-in-the-middle attack exploit affecting BLE versions 4.2-5.0.
- [Lattice Semiconductor Embraces Open Hardware, Launches “Community Sourced” Portal](https://abopen.com/news/lattice-semiconductor-embraces-open-hardware-launches-community-sourced-portal/)<br>
Lattice Semiconductor publicly supports open hardware and has launched a [page on their website](https://www.latticesemi.com/en/Solutions/Solutions/SolutionsDetails01/CommunitySourced) showcasing boards built by the community. 
