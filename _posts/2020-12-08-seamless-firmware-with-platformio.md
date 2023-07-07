---
title: Seamless firmware development with PlatformIO
description:
    An overview of PlatformIO, as well as a getting started example on STM32.
author: francois
tags: [build-system, toolchain]
---

A few weeks ago, [I wrote about MCU SDKs]({% post_url
2020-11-10-the-best-and-worst-mcu-sdks %}) and how frustrating I sometimes found
it to be confronted with one Eclipse-based IDE after another. If you haven't
read my latest updates to that post, check it out!

But what if I told you that there exists an embedded project management tool
which ties you neither to a toolchain, nor to a development environment? Did I
mention it is available for Windows, Linux, and Mac?

<!-- excerpt start -->

In this post, I'd like to introduce PlatformIO. I will go over what PlatformIO
is, how you can use it for your project, and what it is good at.  I also
highlight a few shortcomings worth keeping in mind.

<!-- excerpt end -->

While I am not ready to give PlatformIO a full-throated endorsement for all
projects, it works very well for specific platforms and RTOS-es. Most
importantly, it proposes an approach to library and toolchain management for
embedded that is the best I've seen yet. I hope you will give it a whirl!

{% include newsletter.html %}

{% include toc.html %}

## What is PlatformIO?

<a href="https://platformio.org/" target="_blank">PlatformIO</a> is a set of open source tools targetted at professional embedded
developers.  Foremost of those tools is the PlatformIO Plugin for VSCode which
is used by hundreds of thousands of embedded developers to create, compile,
debug, and test embedded projects.

At its core, PlatformIO is made up of the following subsystems:
1. Project management: a tool to create, configure, and manage projects
2. Package management: a registry and a client to host libraries for embedded
   systems and easily add or remove them from a project
3. Build system: a build configuration and execution system. PlatformIO replaces
   make, cmake, and others.
4. Workflow automation: automation and integrations to debug, run unit tests,
   run static analyzers, and other tools for your project.

When it works, PlatformIO is magical. Want to start a project for STM32
STM32Cube, compiled with ARM GCC 5.2.1? No need to go find an SDK online, Simply
tweak a few configurations files, run a few CLI commands, and you're off too the
races. We do this in the next section of this article.

However, the set of supported targets can be hit or miss.

### Supported Targets

PlatformIO defines targets as a combination as three components:
1. The "platform", which is the MCU family. For example, both the STM32 and the
   nRF52 are supported platforms.
2. The "framework", which is the SDK being used on the project. This includes
   vendor SDK, such as the STM32Cube, as well as RTOS-es such as Zephyr or
FreeRTOS, true frameworks such as Arduino, and alternative APIs such as
libopencm3 and CMSIS.
3. The "board" which defines the specific board you are using. PlatformIO comes
   with hundreds of board configuration for popular development boards like the
STM32 Discovery boards.

The headline numbers are impressive: 41 platforms, 23 frameworks, and 924 boards
as of this writing.

However, reality is a bit less rosy: while some MCU families are extremely well
covered, others have very little support.

One important example is official vendor SDK. PlatformIO only supports the
vendor SDKs for the following MCUs, which it considers individual "frameworks":
* Espressif family (ESP8266, ESP32)
* Kendryte K210
* STM32
* Gigadevice GD32V
* SiFive FE310 and FU540

If you are able to piggy back on an RTOS, things look a bit better. For example,
you can create a Zephyr project with PlatformIO for Atmel SAM, Freescale
Kinetis, Nordic nRF5, NXP iMX RT, NXP LPC, Silabs EFM32, and STM32.

You can use the PlatformIO client to list supported frameworks, platforms, and
boards. The list changes regularly, as the project is very active. Here's the
command you want to run:

```
  # List platforms
  $ pio platform search

  # List frameworks
  $ pio platform frameworks

  # List boards
  $ pio boards
```

### Package Management

PlatformIO hosts a registry of embedded libraries which can be added to every
project. Popular libraries like the lwIP networking stack, the nanopb protocol
buffer implementation, or the mbedTLS SSL library are all available in one form
or another.

Here again, it is magical when it works. Simply add a library to your project by
calling `pio lib install` and rebuild your project. PlatformIO will fetch the
correct bundle, compile a static library, link it into your project, and expose
the header files.

However, the same issue with supported targets gets in the way of using
libraries. Each library lists frameworks and platforms it is compatible with,
and if yours is not listed you cannot use it. Of the more than 10,000 libraries
in the registry, only about 139 of them are compatible with the STM32 +
STM32Cube combo.

Anyone can publish a library to the repository, by following the excellent
online documentation[^library_docs].

Looking for packages is done with the `pio lib search` command. Here are a few
examples:

```
# Look for all libraries that support STM32 + STM32Cube
$ pio lib search -p stm32 -f STM32Cube
...
# Look for mbedtls
$ pio lib search -n mbedtls
...
```

Installing them is done with the `pio install` command.

### Build System

PlatformIO is an opinionated system, and as long as you follow its rules
building a project with it is seamless. Simply add your project source under
`src`, your header files under `include`, and your private libraries under
`lib`. Run `pio run`, and you're cooking with gas!

Under the hood, it uses the SCons[^scons] build system. Most of the
time, this is of no consequence to the user as plenty of configuration options
are exposed via the `platformio.ini` file used to configure each project.

For example, you can change compiler and linker flags by adding the
`build_flags` variable to your `platformio.ini`:

```
[env:release]
platform = ststm32
board = disco_f429zi
framework = stm32cube
build_flags = -Wall -Wl,--gc-sections
```

Should you need to customize your build beyond what the `.ini` file provides,
you will need to write SCons scripts which are invoked by the PlatformIO build
environment. SCons scripts are written in Python.

Two hooks are available for those scripts: a PRE hook which runs before the
build executes, a POST hook which runs after the build. You can use these hooks
to change the build instructions for individual files, pre-process some source
code, or post-process the build artifacts[^advanced_scripting].

While some may chafe at the need to learn yet another build system - I for one
still like my Makefiles - SCons is a mature and powerful build system.

## Getting Started with the PlatformIO CLI

My favorite way to use PlatformIO is with the CLI, also known as "PlatformIO Core".
Although some people swear by the VSCode plugin, I prefer my PlatformIO separate
from my IDE so I can use trusty old Vim.

In this next section, I will walk you through installing the CLI, starting a
project, adding some libraries, and compiling your work. All code is available
in the [Interrupt Github repository](https://github.com/memfault/interrupt),
under `example/platformio`.

### Installing the CLI

Installing the PlatformIO CLI is easy. On some platform, you can use your
package manager. For example, on mac:
```
$ brew install platformio
```

For all other platforms, you can use the `get-platformio.py` script, available
on their website[^get-platformio]. Download it and run it with `python
get-platformio.py`.

At this point, you should have `platformio` available as a CLI utility, as well
as a `pio` alias for faster use.

### Creating a project

The `pio` tool comes with project management commands. Let's say we want to
start a new project for our STM32F429zi discovery board.

First, we'll create a folder:
```
$ mkdir my-project
$ cd my-project
```

Then, we'll initialize the project with `pio project init`, and pass it a board
and a framework

```
$ pio project init --board disco_f429zi --project-options "framework=STM32Cube"
...
Project has been successfully initialized! Useful commands:
`pio run` - process/build project from the current directory
`pio run --target upload` or `pio run -t upload` - upload firmware to a target
`pio run --target clean` - clean project (remove compiled files)
`pio run --help` - additional information
```

### Toolchains and linker scripts

On many of my projects, I use a specific version of the GCC toolchain, and to
write my own linker script. You'll have to update your `platformio.ini` to
enable both of those things:

```
[env]
platform = ststm32
board = disco_f429zi
framework = stm32cube
platform_packages =
  ; GCC 4.8.4
  toolchain-gccarmnoneeabi@1.40804.0
board_build.ldscript = STM32F429ZIYX_FLASH.ld
```

The `board_build.ldscript` bit is straightforward: simply point it at the path
to your linker script within the project folder.

The `platform_packages` variable is more complicated. This option lets you
override the version of built-in packages, such as the support for different
Frameworks, specific toolchains, or built-in tools like OpenOCD.

Tool Packages can be downloaded from Github, or from Bintray where the PlatformIO
project hosts many of its binaries. For example, you can the list of available
builds of GCC at
https://bintray.com/platformio/tool-packages/toolchain-gccarmnoneeabi-darwin_x86_64#files.[^gcc-versions]

In my case, I specified "toolchain-gccarmnoneeabi@1.40804.0" which is GCC 4.8.4
(see the "40804" part of the package version?).

Using a custom built toolchain is a whole other can of worm, which requires
creating your own platform package.

### Writing some code

Your code should go under the `src` folder. In my case, I added a `main.c` file
and wrote a simple blinky firmware:

```c
#include "stm32f4xx_hal.h"

#define LED_PIN                                GPIO_PIN_5
#define LED_GPIO_PORT                          GPIOA
#define LED_GPIO_CLK_ENABLE()                  __HAL_RCC_GPIOA_CLK_ENABLE()
void LED_Init() {
  LED_GPIO_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = LED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
}

void SysTick_Handler(void) {
  HAL_IncTick();
}

int main(void) {
  HAL_Init();
  LED_Init();

  while (1)
  {
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
    HAL_Delay(1000);
  }
}
```

That's it! Note that your SDK configuration files (e.g. the `hal_conf.h` from
STM32Cube) should go in your `src` folder as well.

### Building the project

Next, we need to build our project. Here again we use the `platformio` cli tool.

```
$ pio run
Processing release (platform: ststm32; board: disco_f429zi; framework: stm32cube)
------------------------------------------------------------------------------------------------------------
Verbose mode can be enabled via `-v, --verbose` option
CONFIGURATION: https://docs.platformio.org/page/boards/ststm32/disco_f429zi.html
PLATFORM: ST STM32 (10.0.1) > ST 32F429IDISCOVERY
HARDWARE: STM32F429ZIT6 180MHz, 256KB RAM, 2MB Flash
DEBUG: Current (stlink) On-board (stlink) External (blackmagic, cmsis-dap, jlink)
PACKAGES:
 - framework-stm32cubef4 1.25.2
 - tool-ldscripts-ststm32 0.1.0
 - toolchain-gccarmnoneeabi 1.40804.0 (4.8.4)
LDF: Library Dependency Finder -> http://bit.ly/configure-pio-ldf
LDF Modes: Finder ~ chain, Compatibility ~ soft
Found 48 compatible libraries
Scanning dependencies...
No dependencies
Building in release mode
Compiling .pio/build/release/src/main.o
...
Linking .pio/build/release/firmware.elf
Checking size .pio/build/release/firmware.elf
Building .pio/build/release/firmware.bin
Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
RAM:   [          ]   0.0% (used 44 bytes from 262144 bytes)
Flash: [          ]   0.1% (used 1296 bytes from 2097152 bytes)
======================================= [SUCCESS] Took 10.92 seconds =====================================
```

Here's everything PlatformIO does for us here:

1. Download the library and framework files (e.g. chip SDK, RTOS, ...) for our project
2. Build all C, C++, and assembly files in our `src` folder
3. Build all the private libraries in `lib` and create static libraries for them
4. Link everything into an ELF
5. Copy it into a BIN
6. Print out RAM and Flash usage

All with minimal configuration!

### Unit Tests

Regular readers of Interrupt will know that we are [big fans of unit
testing]({% post_url 2019-10-08-unit-testing-basics %}). PlatformIO has built in supports for running tests on your
host or on target.

For a simple example, consider this trivial library:

Header:
```c
// lib/add/src/add .h
#pragma once

int add(int a, int b);
```

C:
```c
// lib/add/src/add.c
#include "add.h"

int add(int a, int b) {
    return a + b;
}
```

We put those two files in our project under `lib/add/src`, and add a local test
target to our platformio.ini

```
[env:native]
platform = native
```

It's also a good idea to add a `default_envs` property at the top of our
`platformio.ini` file to avoid building the `native` environment for regular
builds. Otherwise PlatformIO will try building our firmware for our native
machine when running `pio run` which will fail:

```
[platformio]
default_envs = release
```

We then write a test file in the `test` folder:

```c
// test/test_add.c
#include "add.h"


void test_add(void) {
    TEST_ASSERT_EQUAL(32, add(25, 7));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_add);
    UNITY_END();

    return 0;
}
```

We can then run our test with `pio test -e native`:

```
$ pio test -e native
Verbose mode can be enabled via `-v, --verbose` option
Collected 1 items

Processing test_add in native environment
------------------------------------------------------------------------------------------------------------
Building...
Testing...
test/test_add/test_add.c:13:test_add    [PASSED]

-----------------------
1 Tests 0 Failures 0 Ignored
OK
======================================== [PASSED] Took 1.27 seconds ========================================

Test      Environment    Status    Duration
--------  -------------  --------  ------------
test_add  native         PASSED    00:00:01.265
======================================= 1 succeeded in 00:00:01.265 =======================================
```

## Conclusion

There's a lot to like about PlatformIO. It's an open source cross platform tool
that makes it easy to setup, build, and test an embedded project no matter what
IDE you use or what OS you run on your laptop.

Best of all, having a package manager for embedded project is fantastic and
will I hope further encourage people to collaborate and leverage excellent open
source libraries like mbedTLS.

At the same time, the low number of supported vendor SDKs ("frameworks") makes
PlatformIO impractical for many professional projects. Unless you are on an
STM32 or an Espressif chip, you likely will need to do a lot of extra work to
add support for your platform of choice to PlatformIO.

Last but not least while the nomenclature is a bit wonky, the PlatformIO
documentation is excellent with tons of examples. The engineers behind the
projects are also very active on twitter and on their discussion board. Props to
them for excellent support!

I look forward to watching PlatformIO grow over the next few years, and cannot
wait to use it for a future project. Look our for `platformio.ini` files in
future Interrupt examples!

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^library_docs]: [PlatformIO: Creating a library](https://docs.platformio.org/en/latest/librarymanager/creating.html)
[^scons]: [SCons website](https://scons.org/)
[^advanced_scripting]: [PlatformIO: Advanced Scripting](https://docs.platformio.org/en/latest/projectconf/advanced_scripting.html#build-middlewares)
[^get-platformio]: [get-platformio.py from Github](https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py)
[^gcc-versions]: As of this writing, GCC versions 4.8.4, 5.2.1, 5.4.1, 6.3.1, 7.2.1, 8.2.1, 8.3.1, 9.2.1, and 9.3.1 are available for download.
<!-- prettier-ignore-end -->
