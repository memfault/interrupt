---
title: "What we've been reading in February (2022)"
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

- [**Proactive Debugging with Offensive Programming Webinar - Memfault**](https://go.memfault.com/proactive-debugging-with-offensive-programming-webinar) by Tyler Hoffman<br>
  I gave a webinar on what is probably my favorite topic nowadays - using the ideas of Offensive Programming and a robust remote debugging solution (this time Memfault) to rapidly speed up development, testing, and the too-often "Something is broken in production. We need to fix it now!" cycles.

- [**Running C unit tests with pytest - The Hub of Heliopolis**](https://p403n1x87.github.io/running-c-unit-tests-with-pytest.html) by Gabriele Tornetta<br>
  Unit testing in C/C++ is annoying. Even the simplest things like using lists or hash maps for fixtures and fakes aren't trivial. The people I've talked to in the past who have used Python to test C sing praises of the technique. Here's a blog post explaining how you could go about doing this!

- [**How is git commit sha1 formed - GitHub Gist**](https://gist.github.com/masak/2415865) by Carl Mäsak<br>
  Nice breakdown of what components are included when `git` computes a commit hash - Noah

- [**moreutils: the utilities package every UNIX/Linux/Mac OS developer should know - Clean Coding**](https://rentes.github.io/unix/utilities/2015/07/27/moreutils-package/)<br>
  A nice description of some more exotic but compelling UNIX utilities - Noah

- [**Precursor: From Boot to Root - bunnie studios**](https://www.bunniestudios.com/blog/?p=6336)<br>
  A one-hour presentation on how the [Precursor](https://www.crowdsupply.com/sutajio-kosagi/precursor) hardware and software stacks look from the lowest levels.

- [**Matter security model. Espressif Matter Series #7 - The ESP Journal**](https://blog.espressif.com/matter-security-model-37f806d3b0b2) by Guo Jiacheng<br>
  The seventh installment in a series about the Matter protocol, this time about the security model.

- [**Working with Strings in Embedded C++ - Sticky Bits**](https://blog.feabhas.com/2022/02/working-with-strings-in-embedded-c) by Niall Cooling<br>
  Everything you could possibly want to know about working with strings in C and C++ on embedded devices. Some implementations allocate space on the heap, the stack, or in read-only memory, and it's important to know what happens under the hood.

- [**Listening to devices with libudev - usairb devlog #1**](https://fnune.com/devlog/usairb/2022/02/05/listening-to-devices-with-libudev-usairb-devlog-1) by Fausto Núñez Alberro<br>
  Development log of usairb, a new project Fausto, a Memfault employee, is building to learn embedded Linux. The goal is to transform any embedded Linux device with access to the Internet into a multiplexing transmitter for USB hubs.

- [**Simple and Exponential Moving Averages in embedded C++ - Stratify Labs**](https://blog.stratifylabs.dev/device/2022-03-02-Simple-Moving-Average-and-Exponential-Moving-Average-in-embedded-Cpp)<br>An article about how to implement moving averages in C++ complete with the mathematical formulas and code.

- [**One Simple Trick to Improve Your Firmware Module Logging - Ovyl**](https://www.ovyl.io/blog-posts/one-simple-trick-to-improve-your-firmware-module-logging) by Nick Sinas<br>
  A simple article talking about using macros to prefix or set logging levels. If you don't have similar knobs in your firmware today, definitely add some!

- [**The Myth of Three Capacitor Values - Signal Integrity Journal**](https://www.signalintegrityjournal.com/articles/1589-the-myth-of-three-capacitor-values) by Eric Bogatin, Larry Smith, and Steve Sandler<br>
  A nice rundown about using MLCC capacitors for decoupling- turns out, using three different values can provide worse performance than a single one. - Noah

- [**On finding the average of two unsigned integers without overflow - The Old New Thing**](https://devblogs.microsoft.com/oldnewthing/20220207-00/?p=106223) by Raymond<br>
  Always interesting to re-think seemingly trivial problems (here: avg of two unsigned ints) – especially in the context of overflows which are a common source of errors in C. Also some nice discussion of compiler output - Heiko

- [**Remote Device Debugging and Monitoring using Memfault - Conexio Stratus**](https://www.rajeevpiyare.com/posts/stratus-to-memfault/) by Rajeev Piyare<br>
  A (seriously) non-sponsored post about using Memfault with Conexio Stratus devices to help remotely monitor and debug Status devices.

## Tools & Projects

- [**Remote Zephyr development using Segger tunnel and a Raspberry Pi - Golioth**](https://blog.golioth.io/remote-zephyr-development-using-segger-tunnel-and-a-raspberry-pi/) by Vojislav Milivojević<br>
  It's always been a little-known secret that the Segger J-Link has the ability to open a port and publicly expose the JTAG port. This article talks about this feature and goes a step further to connect a Raspberry Pi to the J-Link so that you can debug hardware from anywhere!

- [**armink/FlashDB: An ultra-lightweight database that supports key-value and time-series data**](https://github.com/armink/FlashDB)<br>
  A Key-Value database, a Timeseries database, and a Log-series database all rolled into one library and optimized for flash storage on MCU's. Honestly, I like their API. It feels like a modern library, which I don't say often.

- [**seemoo-lab/openhaystack: Build your own 'AirTags' 🏷 today! Framework for tracking personal Bluetooth devices via Apple's massive Find My network.**](https://github.com/seemoo-lab/openhaystack/)<br>
  Reverse engineering of the Apple Find My system.

- [**hanickadot/compile-time-regular-expressions: A Compile time PCRE (almost) compatible regular expression matcher.**](https://github.com/hanickadot/compile-time-regular-expressions) by Hana Dusíková<br>
  Interesting repository that implements compile-type regular expressions in C++ - Noah

- [**nasa-jpl/embedded-gcov: GCC/gcov code coverage data extraction from the actual embedded system, without requiring a file system, an operating system, or standard C libraries**](https://github.com/nasa-jpl/embedded-gcov/)<br>
  A Github repo seemingly out of NASA that details how you can use gcov on your embedded systems.

## Random

- [**Embedded Systems Weekly is back!**](https://embedsysweekly.com/)<br>
  It's back from the dead after seeing no updates since August 2018. Subscribe now!

- [**C-ing the Improvement: Progress on C23 - The Pasture**](https://thephd.dev/c-the-improvements-june-september-virtual-c-meeting)<br>
  Rundown of some of the up-and-coming C23 features. I'm a big fan of the digit separators for literals in Rust, nice to see them coming to C!

- [**Some mistakes Rust doesn't catch**](https://fasterthanli.me/articles/some-mistakes-rust-doesnt-catch) by Amos<br>
  Rust is known as a performant language similar to C and C++ that prevents a lot of the common mistakes in these two languages. This is an expansive article on some of the common developer errors that Rust does not prevent you from making.

- [**Inline assembly - The Rust Reference**](https://doc.rust-lang.org/nightly/reference/inline-assembly.html)<br>
  Inline assembly is stable in rust! You only need to nest three `unsafe` blocks together.

- [**AdaCore and Ferrous Systems Joining Forces for Safety Critical Rust Variant - The AdaCore Blog**](https://blog.adacore.com/adacore-and-ferrous-systems-joining-forces-to-support-rust)<br>
  AdaCore has been writing [some](https://blog.adacore.com/ada-on-any-arm-cortex-m-device-in-just-a-couple-minutes) [cool](https://blog.adacore.com/an-embedded-usb-device-stack-in-ada) [articles](https://blog.adacore.com/ada-on-a-feather) about Ada on embedded devices. We've also been very impressed by the work out of Ferrous Systems and their developments on embedded rust. I'm excited to see where this partnership goes in the future!

- [**Debugging WebAssembly with modern tools - Chrome Developers**](https://developer.chrome.com/blog/wasm-debugging-2020/) by Ingvar Stepanyan<br>
  It's a bit random for Interrupt, but we continue to love the developments on WebAssembly and truly believe that we'll live in a world where we run high-level language software compiled down to WebAssembly on our embedded devices. This would allow us to do most of our testing on host machines for quicker iteration cycles.
