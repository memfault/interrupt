---
title: The Best and Worst MCU SDKs
description:
  A detailed comparison of ARM Cortex-M Microcontroller SDKs and BSPs, from best
  to worst.
author: francois
tags: [mcu]
---

In 2020, an MCU is much more than a hunk of silicon. Indeed, it comes with a
whole ecosystem including a BSP, integrated third-party libraries, tooling,
field application support,  and more.

As firmware engineers, we are often handed down an MCU selection as a fait
accompli. Cost concerns, peripheral, or pinout requirements often take precedent
over the SDK.

Yet we are allowed to have an opinion. So here for you today is my first post in
a new “comparing MCU SDKs” series.

<!-- excerpt start -->

In this post, I download SDKs for 10 popular Cortex-M4 microcontrollers, and
evaluate how straightforward it is to get a simple example compiling. I include
some step by step instructions to get started, a rating out of 10, and a few
comments.

<!-- excerpt end -->

It goes without saying that this is an opinion based on my own very specific
setup (MacOS, vim). Your mileage may vary!

{% include newsletter.html %}

{% include toc.html %}

## What I’m looking for in a chip SDK

My ideal chip SDK provides a way to build and flash projects using tools of
my choosing. This seems like a low bar, but few meet it. Here are things chip
SDKs should **not** do.

### Don’t choose my laptop OS for me!

I left Windows behind when I worked at Sun Microsystems, and I have not looked
back. Today, my daily driver is a MacBook Air.  Unfortunately, some chip vendors
require that you use their Windows-based tools to set up and build your projects.
Now that most compilers are cross-platform, there is no excuse for it.

### Don’t choose my IDE for me!

I’ve been using `vim` since college, and you can take it from my cold, dead
hands. I love `vim`! I have it configured just so. It’s lightweight, it’s fast,
and modal editing is the way to work (prove me wrong!). So you’ll understand my
dismay at the spate of Eclipse-based IDEs chip vendors want to foist upon me. I
want nothing to do with their boated, Java environments.

Instead of Eclipse-based IDE, I suggest SDKs provide Makefiles. `make` is the
lowest common denominator build system, and is well supported by many tools.
Bonus points for project files for IAR and Keil, since many of you like those
tools.

### Do include some examples

If a picture is worth a thousand words, then a working code example is worth a
million. Give me one example of each of the main use cases for your MCU. Bonus
points if you give me an example for each peripheral.

## Ten Popular Chip SDKs, Ranked

### Nordic Semi nRF5-SDK - 10/10

Nordic semiconductor’s line of Cortex-M4 MCUs includes the nRF52810, nRF52810,
and nRF52840. All feature a 2.4GHz radio, which may deter you if all you want is
an MCU.

The nRF5-SDK (soon to be replaced by the equally excellent nRF-Connect-SDK), is
in my view the best chip SDK out there.

#### Why the rating

* Cross-platform ✅
* Supports armcc/Keil, IAR, and Makefiles ✅
* Lots of bundled examples ✅
* Single zip, no install needed ✅

The nRF5 SDK does everything right. No registration, no install, no online
configurator. It even is distributed under a BSD license!

#### Compiling a Blinky example
1. Download the nRF5 SDK from [nRF5 SDK downloads - nordicsemi.com](https://www.nordicsemi.com/Software-and-tools/Software/nRF5-SDK/Download#infotabs)
2. Unzip the resulting `DeviceDownload.zip` file
3. Unzip the SDK archive contained within. In my case that’s `nRF5SDK1702d674dde.zip`
4. `cd` to the example you are interested in. Examples are contained within the `example` folder. In my case, `examples/peripheral/blinky/pca10040/blank`.
5. `cd` to the build system folder of your choice, in my case `armgcc`
6. Build the example, in my case by invoking `GNU_INSTALL_ROOT= make`

This will generate a `bin`, `elf`, and `hex` file (among others) under `_build`.

### Texas Instruments TivaWare - 9/10

The Tiva C series is the latest entry in Texas Instruments’s line of Cortex-M
microcontrollers.  I do not have a lot of experience with them, but they seem
like solid microcontrollers with a broad range of peripherals (including USB).

#### Why the rating

* Cross-platform ✅
* Supports armcc/Keil, IAR, and Makefiles ✅
* Lots of bundled examples ✅
* Single zip, no install needed ✅

Like Nordic, Texas Instruments gets a lot right: single-zip download, multi-IDE
support (including Makefiles), and lots of examples. I knocked off a point for
the wonky `exe` file (see below) and the more complicated license.

#### Compiling a Blinky example

1. Download TiWare SDK from [SW-TM4C_2.2.0.295, TI.com](https://www.ti.com/tool/download/SW-TM4C)
2. Rename the downloaded file from `exe` to `zip`
3. Unzip it
4. `cd` to the example you are interested in. In my case `examples/boards/dk-tm4c129x/blinky`.
5. Run `make`, or open the IDE-specific folder of your choice.

You’ll be left with an `axf` (aka an ELF) and a `bin`  file in the `gcc` folder.

### NXP MCUXpresso - 9/10

![]({% img_url best-and-worst-mcu-sdks/E2992448-AC4D-4A31-9B86-E4806F729E51.png %})

NXP Kinetis traces its lineage to Motorola via Freescale. It is one of two
Cortex-M lines from NXP (the other being the LPC).  Like many MCU vendors, NXP
generates their SDK via a configurator and provides an Eclipse-based IDE, both
under the “MCUXpresso” brand.

#### Why the rating

* Cross-platform ✅
* Supports armcc/Keil, IAR, and Makefiles ✅
* Lots of bundled examples ✅
* Single zip, no install needed ⚠️

I found the online MCUXpresso SDK builder a breeze to use. It is snappy,
straightforward, and it keeps track of all your previously configured SDKs.
Great job, NXP! The only downside of the tool is that it requires you to
register on their website.

I still have a preference for the monolithic SDK with all examples and targets
in one place, but you could argue that this generates a smaller, more
streamlined SDK.

Best of all, the SDK uses `cmake` to generate build files. `cmake` is arguably
more portable than `make`, as it is supported natively on Windows.

#### Compiling a hello world example

1. Go to the MCUXpresso SDK builder and create an account: [MCUXpresso SDK Builder](https://mcuxpresso.nxp.com/)
2. Once logged in, click on “Select Board” in the sidebar
3. Enter the name of your MCU. In my case, a Kinetic K21
4. Click on “Build MCUXpresso SDK” on the right
5. Name your project, select a toolchain, and some third party software like FreeRTOS or mbedTLS.
6. Click on “Download SDK” at the bottom
7. After a bit, the SDK will be ready for download
8. Unzip the SDK
9. `cd` to the example of your choice, in my case `boards/twrk21d50m/demo_apps/hello_world/`
10. `cd` to build system folder, in my case `armgcc`
11. Use the `build_debug.sh` build script and specify the`ARMGCC_DIR` environment variable:
e.g. `ARMGCC_DIR=/usr/local/Cellar/arm-none-eabi-gcc/9-2019-q4-major sh build_debug.sh`

This will generate an ELF file.

### STM32 Cube - 8/10

![]({% img_url best-and-worst-mcu-sdks/6DA202F6-ED4E-4FFE-94B7-3A549D0F212F.png %})

ST has gone through multiple iterations of the SDK for the STM32 family of ICs.
The latest is called STM32 Cube, which replaces the venerable Standard
Peripheral Library. While Cube introduces a lot of complexity, it does so for a
good reason: the STM32 family has grown to include 14 distinct series of MCUs
from the very low power L0 to the very high-performance H7.

> Note: Reader Nathan Jones [pointed
> out](https://community.memfault.com/t/the-best-and-worst-mcu-sdks-interrupt/294/12)
> after the initial publication of this post that monolithic SDK downloads do
> still exist for STM32. For example, [here is the SDK for the
> STM32F1](https://www.st.com/content/st_com/en/products/embedded-software/mcu-mpu-embedded-software/stm32-embedded-software/stm32cube-mcu-mpu-packages/stm32cubef1.html)

#### Why the rating

* Cross-platform ✅
* Supports armcc/Keil, IAR, and Makefiles ✅
* Lots of bundled examples ✅
* Single zip, no install needed ❌

While Cube comes with support for many IDEs, and more examples than any other
MCU SDK, it wraps it all in a clunky desktop app. I had a terrible time using
STM32CubeMX: I had to install it on my laptop, it’s slow, it’s large, it’s
clunky. I do not like it.

STM32CubeMX generates a “project” directory based on your configuration. This
means that you won’t have all the example code in one folder, and instead will
need to generate different projects for different examples.

Necessary complexity? Perhaps. But I miss the simpler Peripheral Library which
came as a single archive.

#### Compiling a Hello World example

1. Download and install CubeMX: [STM32CubeMX - STM32Cube initialization code generator - STMicroelectronics](https://www.st.com/en/development-tools/stm32cubemx.html). Note: this requires registration on ST’s website

3. Select “ACCESS TO MCU SELECTOR”
4. Select the part you are using. In my case “STM32F429IE”
5. In the Configuration view, click on the “Project Manager” tab
6. Enter a project name, a path, and select a toolchain. In my case “Makefile”
7. Click on “Generate Code” at the top right
8. `cd` to the generated project directory
9. Compile the project with your build system. In my case with `make`.

###  Atmel START for SAMD - 7/10

![]({% img_url best-and-worst-mcu-sdks/83E37D9A-5975-4815-8937-437DA8675C0D.png %})

Recently acquired by Microchip, Atmel has been making SAM-family MCUs for a long
time. The SAMD21 is well-liked in hobbyist circles and is featured in several
Arduino and Adafruit designs. Atmel’s peripheral library, AXF, went through a
similar transformation to ST’s: it went from a single zip archive to a
configurator.

#### Why the rating

* Cross-platform ✅
* Supports armcc/Keil, IAR, and Makefiles ✅
* Lots of bundled examples ✅
* Single zip, no install needed ❌

Atmel’s configurator is web-based, and a tad more ergonomic than ST’s. However,
the resulting Makefiles are much worse and even feature a bug (I had to fix OS
detection).

#### Compiling a hello world example

1. Go to start.atmel.com
2. Click on “Create New Project”
3. Select your MCU. In my case a SAMD51 Chip.
4. Click on “Export Project” at the top right
5. Give it a name, and tick the check-box for your IDE (for me: Makefile)
6. Click on “Download Pack”
7. Rename resulting file from `.azip` to `.zip`
8. Extract it
9. `cd` to into IDE folder, in my case `gcc`
10. Fix Makefile OS detection:
    ```diff
    @@ -22,7 +22,7 @@ else
                    MK_DIR = mkdir -p
            endif

    +       ifeq ($(shell uname | cut -d _ -f 1), Darwin)
    -       ifeq ($(shell uname | cut -d _ -f 1), DARWIN)
                    MK_DIR = mkdir -p
            endif
     endif
    ```
11. Run `make`

### Silabs Simplicity Studio - 5/10

![]({% img_url best-and-worst-mcu-sdks/3F4AD307-656B-4664-B988-4F5A301BD771.png %})

Silabs Cortex-M MCU comes from its acquisition of Energy Micro who was famous for
the very low power consumption of their MCUs. Silabs now makes a range of
Cortex-M based MCUs, some with 2.4GHz radios.

Like many of the vendors in the lower half of this list, Silabs distributes
its SDK alongside an Eclipse-based IDE. In their case, they call it “Simplicity
Studio”. While Simplicity is the best of those IDEs, it does leave those of us
who do not love Eclipse with few solutions. Here, I chose to use Eclipse to
generate Makefiles.

#### Why the rating

* Cross-platform ✅
* Supports armcc/Keil, IAR, and Makefiles ❌ (only IAR and GCC are supported by simplicity studio)
* Lots of bundled examples ✅
* Single zip, no install needed ❌

Simplicity studio is huge (~1GB), slow, and complicated. Like all Eclipse-based
IDEs, it generates poor Makefiles with hardcoded paths everywhere.

#### Step by step hello world example

1. Register at silabs.com
2. Download and install Simplicity Studio: [Simplicity Studio - Silicon Labs](https://www.silabs.com/products/development-tools/software/simplicity-studio)
3. Setup SDKs on first run
4. Click on File -> New -> Silicon Labs Project Wizard
5. Under “Target Device”, select your MCU
6. Click “Next”
7. Select “Empty C project”
8. Click “Next”
9. Give the project a name, and select “Copy contents” under “With project files”
10. Once the project is created, build it with Project -> Build Project
11. Use the generated makefiles in the project folder


### Infineon XMC4000, Renesas RA4 3/10

Infineon and Renesas MCUs are popular for industrial applications and in the
automotive industry. In both cases, the standard setup is a Windows executable
which installs an eclipse-based IDE.

All is not lost, however, as they also provide a peripheral library which can be downloaded standalone.:
* Infineon calls theirs XMC Lib: http://dave.infineon.com/Libraries/XMCLib/XMC_Peripheral_Library_v2.1.24.zip
* Renesas has the “Example Project Bundle”: [Download Detail Page, Renesas Electronics](https://www.renesas.com/us/en/software/D6004702.html)

Unfortunately, neither of them offers Makefiles, so this is where my
investigation ended. I did give them a few points for offering IAR and Keil.

#### Why the rating

* Cross-platform ❌
* Supports armcc/Keil, IAR, and Makefiles ❌
* Lots of bundled examples ✅
* Single zip, no install needed ❌

### Cypress PSoC Creator, Maxim DARWIN 0/10
Cypress and Maxim both make interesting chips: the former has a popular family
of BLE MCUs under its PSoC brand, the latter makes MCUs with very large flash or
RAM which can be put to good use.

Unfortunately, I could not get anywhere with either. It seems the PSoC creator is
required to set up a PSoC6 project, but the app is windows-only.  Meanwhile, all I
could find from Maxim is a Windows installer for the “ARM Cortex toolchain.

## Conclusion

I hope you came away from this post with a better understanding of the different
setup steps required for different MCU SDKs.

You will note that I only looked at the setup procedures in this post. In the future, I intend
to compare APIs, ecosystem, and tools.

Do you have a favorite MCU SDK? We'd love to hear about it in the comments!

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->
