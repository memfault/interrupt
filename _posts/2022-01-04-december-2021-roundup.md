---
title: "What we've been reading in December (2021)"
author: tyler
tags: [roundup]
---

<!-- excerpt start -->

🎉 Happy 2022 everyone! 🎉

Here are the articles, videos, and tools that we've been excited about this
December.

<!-- excerpt end -->

We hope you enjoy these links, and we look forward to hearing what you've been
reading in the comments or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).

## Articles & Learning

- [Writing a simple 16 bit VM in less than 125 lines of C - andreinc](https://www.andreinc.net/2021/12/01/writing-a-simple-vm-in-less-than-125-lines-of-c) by Andrei Ciobanu<br>
  This is a thorough yet straightforward walkthrough on how to build a VM in C. VMs can be a great tool for embedded engineers when 3rd party scripting is a requirement. - François

- [Ask HN: Best Engineering Blog Posts? - Hacker News](https://news.ycombinator.com/item?id=29758396&source=techstories.org)<br>
  I love a good compilation of websites from different communities. This time, good engineering blogs from the readers of Hacker News. - Tyler

- [WebAssembly and Back Again: Fine-Grained Sandboxing in Firefox 95 - Mozilla Hacks](https://hacks.mozilla.org/2021/12/webassembly-and-back-again-fine-grained-sandboxing-in-firefox-95/) by Bobby Holley<br>
  While not exactly meant for embedded, it describes a neat approach to running WASM code on embedded devices. By compiling pretty much _any language_ to WASM, then to C, and then compiling that C code into target code, you can get isolation guarantees enforced by the WASM memory and API model! - Heiko

- [LongUsername comments on Embedded Industry in the next 10 years starting 2022 - Reddit /r/embedded](https://old.reddit.com/r/embedded/comments/ru4cjn/embedded_industry_in_the_next_10_years_starting/hqyh6bg/)<br>
  There's not a single statement I would disagree with here. - Heiko

- [Why is my Rust build so slow?](https://fasterthanli.me/articles/why-is-my-rust-build-so-slow) by Amos<br>
  Another super deep dive article, going into detail on what causes a Rust build to be slower than expected. Fascinating read covering a lot of different aspects of the problem! - Noah

- [More Modern CMake](https://hsf-training.github.io/hsf-training-cmake-webpage/aio/index.html)<br>
  A good collection of links for using CMake. - Noah

- [A curated list of project-based tutorials in C - GitHub](https://github.com/rby90/project-based-tutorials-in-c)<br>
  A list of tutorials that work towards the making of a complete project in C. We found it because it mentions François' [Zero to main() series]({% tag_url zero-to-main %}). - Colleen

- [What's up with these 3-cent microcontrollers? - Jay Carlson](https://jaycarlson.net/2019/09/06/whats-up-with-these-3-cent-microcontrollers/) by Jay Carlson<br>
  From the author of the $1 microcontroller article, Jay writes about the latest 3-cent Padauk microcontroller. It's fascinating because it contradicts a [Hackaday article from 2019](https://hackaday.com/2019/04/26/making-a-three-cent-microcontroller-useful/) which said these chips were _terrible_. Jay thinks otherwise! - Tyler

- [All spectral-norm problem benchmarks - Programming Language and Compiler Benchmarks](https://programming-language-benchmarks.vercel.app/problem/spectral-norm)<br>
  These types of benchmarks are probably not useful metrics for any purpose (but I kinda find them fun 😅). What's also cool is that Zig produces impressively efficient binaries, considering the amount of time put into that compiler vs. the others 😮. - Noah

- [UART FIFO with DMA on STM32 - Stratify Labs](https://blog.stratifylabs.dev/device/2021-12-30-UART-FIFO-with-DMA-on-STM32/)<br>
  A to-the-point post. The author details how to use a FIFO buffer with the DMA on an STM32 and it includes a full code example at the bottom of the post. - Tyler

- [Fixing a Tiny Corner of the Supply Chain - bunnie's blog](https://www.bunniestudios.com/blog/?p=6274)<br>
  Bunnie is a famous hardware hacker who's building a nifty mobile device, the [Precursor](https://www.crowdsupply.com/sutajio-kosagi/precursor). This article describes how difficult it was to secure a critical component in the current age of supply chain woes! - Noah

- [IOActive Labs: A Practical Approach To Attacking IoT Embedded Designs](https://labs.ioactive.com/2021/02/a-practical-approach-to-attacking-iot.html) by Ruben Santamarta<br>
  Embedded network stacks are a very vulnerable spot (especially in systems with a single address space). This article shows some practical examples of exploits for several popular embedded networking stacks/chips. - Noah

- [Light Years Ahead - The 1969 Apollo Guidance Computer (video)](https://www.youtube.com/watch?v=B1J2RMorJXM) by Robert Wills<br>
  Content is already a bit dated but I still like it as the 6 identified core design principles (time code 47:19) are still industry best-practices for reliable embedded software with 6/6 being “telemetry” something we at Memfault deeply believe in – it’s also a well rounded and fun presentation

- [4 Detect Breaking Changes - Buf Documentation](https://docs.buf.build/tour/detect-breaking-changes)<br>
  Protobuf has serialization guarantees for forward/backward compatibility, but not for the message format itself. It looks like this tool provides this! Super pain point at a previous job, so it's nice to see someone working on it! - Noah

- [Making a CAN bus module work with a Raspberry Pi – LinuxJedi's /dev/null](https://linuxjedi.co.uk/2021/12/01/making-a-can-bus-module-work-with-a-raspberry-pi/) by Andrew Hutchings<br>
  Disappointed that his CAN module doesn't work by default with the Raspberry Pi's GPIO pins due to voltage differences, Andrew modifies the module by removing a chip and mounting a new one that is 3.3V compatible. - Tyler

- [How SQLite Is Tested - SQLite](https://www.sqlite.org/testing.html)<br>
  SQLite is a pretty hard-used database, needs to be resilient to a number of fault conditions. this type of mutation and corpus/instrumented guided fuzzing is pretty nice for hardening the library! - Noah

## Tools & Projects

- [DavidBuchanan314/ambiguous-png-packer: Craft PNG files that appear completely different in Apple software](https://github.com/DavidBuchanan314/ambiguous-png-packer)<br>
  Apple devices use a common, broken PNG decoder. This exploit shows a different image on Apple vs. non-Apple devices! - Noah

- [willmcgugan/rich: Rich is a Python library for rich text and beautiful formatting in the terminal.](https://github.com/willmcgugan/rich)<br>
  I've always wanted to use some Python CLI-based-GUI libraries before but still haven't. This one looks like a cool one and was mentioned in the embedded.fm Slack channel and was used as a GUI for manufacturing dashboards and tests. - Tyler

- [Advantage360 Split Ergonomic Keyboard - Kinesis](https://kinesis-ergo.com/keyboards/advantage360/)<br>
  Kinesis, the maker of my [favorite keyboard](https://kinesis-ergo.com/shop/advantage2/), recently announced the new Advantage 360 which uses Zephyr based operating system, [ZMK](https://zmk.dev/). I will admit I'm a little upset they removed the F key row, but I believe it was due to cost reasons, which is fair. - Tyler

- [cesanta/mjson: C/C++ JSON parser, emitter, JSON-RPC engine for embedded systems](https://github.com/cesanta/mjson#json-rpc-example)<br>
  Nice little JSON library suitable for embedded- looks very well tested (with sanitizers!). Includes a minimal RPC feature that looks quite useful. - Noah

- [Tiny Emulators](https://floooh.github.io/tiny8bit/)<br>
  A neat collection of legacy 8-bit system emulators. Did you ever want to run your Forth code on a Z1013? You're in luck! - François

## News & Updates

- [CLion 2021.3 includes Multi-threaded RTOS debugging](https://blog.jetbrains.com/clion/2021/12/clion-2021-3-remote-debugger-docker/#debugger_updates_for_embedded_development) by Anastasia Kazakova<br>
  In similar spirits to VSCode's addition of embedded debugging, CLion also introduced deeper support for embedded targets running FreeRTOS and Zephyr, including introspection into data structures and runtime objects. I'm excited to see these IDE's become better for firmware development. Check out the [documentation as well](https://www.jetbrains.com/help/clion/rtos-debug.html). - Tyler

- [Embedded Software Development in Visual Studio - Microsoft C++ Team Blog](https://devblogs.microsoft.com/cppblog/visual-studio-embedded-development/)<br>
  VSCode now has native support for debugging embedded devices. Previously, you had to install [Cortex-Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug). They also have built-in blocks for showing RTOS primitives, such as heaps, byte pools, and threads. Bye Eclipse! - Tyler

- [Bootstrap your dev environment with vcpkg artifacts - Microsoft C++ Team Blog](https://devblogs.microsoft.com/cppblog/vcpkg-artifacts/) by Marc Goodner<br>
  VSCode announced a new developer environment manager, **vcpkg**. It's similar to the bootstrap scripts you'd find in the Zephyr or Pigweed projects. Honestly, I hope Zephyr switches to this because we all know we are scared to run their initialization script. - Tyler

- [Improving GitHub code search - The GitHub Blog](https://github.blog/2021-12-08-improving-github-code-search/)<br>
  Pretty much everyone at Memfault - Searching for code on GitHub can be pretty clunky, this looks like a very nice upgrade!

- [0.9.0 Release Notes - The Zig Programming Language](https://ziglang.org/download/0.9.0/release-notes.html#musl-122)<br>
  Clever hack the zig compiler team uses to ship a multi-arch libmusl, by cross-compiling (zig cc) libmusl for multiple architectures then extracting the common and arch-specific output and combining it into a single file - Noah

- [Rust Embedded Working Group](https://blog.rust-embedded.org/this-year-in-embedded-rust-2021/)<br>
  End of year roundup of many interesting active embedded rust projects! - Noah
