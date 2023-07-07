---
title: "Programming the ATSAMD21 with IBDAP"
description: "How to flash the Atmel SAM D21 using the IBDAP programmer"
author: francois
tags: [cortex-m]
---

<!-- excerpt start -->
In the process of porting a blog post from the retired [Arduino M0
Pro](https://store.arduino.cc/usa/arduino-m0-pro) to Adafruit's excellent
[Metro M0
Express](https://www.adafruit.com/product/3505), I ran into a few issues and scant amount of documentation. I'm writing this for
the next poor soul wrestling with these systems.
<!-- excerpt end -->

Note: I'm a MacOS user, but the procedure below will work fine on Linux /
Windows with some adjustments.

## Setting up the IBDAP programmer

I had recently picked up an [IBDAP](https://www.adafruit.com/product/2764)
programmer from Adafruit. This is a simple CMSIS-DAP device based on the
LPC11u35 chipset.

In order to work as a CMSIS-DAP device, the dongle needs to be flashed with a
firmware from the supplier. Unfortunately, the company behind the dongle seems to have gone out of business.
Its documentation, firmware binaries, and github repositories have all vanished
from the internet.

I found out via a forum post that mbed's
[SWDAP](https://os.mbed.com/platforms/SWDAP-LPC11U35/) firmware precompiled for
LPC1768 works fine on this board. From there, the steps were relatively
straightforward:

1. Download the pre-built firmware.
```terminal
$ wget https://os.mbed.com/media/uploads/chris/lpc11u35_swdap_lpc1768_if_crc.bin
```
2. Connect the dongle over USB.
3. Put the dongle in DFU mode by pressing both the RESET and ISP buttons. It
   should now appear as a USB drive named "CRP DISABLD".
3. Copy the firmware over to the drive. On my Mac this was best achieved with
   `dd`.
```terminal
$ dd if=lpc11u35_swdap_lpc1768_if_crc.bin of=/Volumes/CRP\ DISABLD/firmware.bin
conv=notrunc
```
Note that simply dragging the firmware over did not work because it fits exactly
within the flash and OSX wants some overhead, erroring out with "not enough space".
4. Reset the dongle by pressing the RESET button.
5. Verify that it now enumerates as a USB drive named "DAPLINK".

You are now the proud owner of a functional IBDAP programmer!

## Installing OpenOCD

Installing OpenOCD with all the right dependencies to do CMSIS-DAP was once
trickier than it ought to be. Today, the Homebrew formula takes care of it all
for you:

```terminal
$ brew install openocd
...
$ openocd --version
Open On-Chip Debugger 0.10.0
Licensed under GNU GPL v2
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html
```
Huzzah!

## Flashing the atsamd21g18

Both the Arduino M0 pro and the Metro M0 Express boards come with fancy
bootloaders that let you [update your firmware over
USB](https://learn.adafruit.com/adafruit-metro-m0-express-designed-for-circuitpython/uf2-bootloader-details).
Most people will be thrilled to use that.

If - like me - you're trying to build your own bootloader, things get a bit more
complicated.

First, it's best to check that you're able to talk to the board over the debug
interface. Use a [SWD Ribbon cable](https://www.adafruit.com/product/1675) to
connect the IBDAP dongle to your board (you want to use the TGT_DBG port on the
dongle). OpenOCD already has config files for CMSIS-DAP and the ATSAMD family,
so you just need to source them and run openocd:

```terminal
$ openocd -c "source [find interface/cmsis-dap.cfg]; source [find
target/at91samdXX.cfg]"
Open On-Chip Debugger 0.10.0
Licensed under GNU GPL v2
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html
Info : auto-selecting first available session transport "swd". To override use
'transport select <transport>'.
none srst_pulls_trst
adapter speed: 400 kHz
cortex_m reset_config sysresetreq
Info : CMSIS-DAP: SWD  Supported
Info : CMSIS-DAP: Interface Initialised (SWD)
Info : CMSIS-DAP: FW Version = 1.0
Info : SWCLK/TCK = 1 SWDIO/TMS = 1 TDI = 0 TDO = 0 nTRST = 0 nRESET = 1
Info : CMSIS-DAP: Interface ready
Info : clock speed 400 kHz
Info : SWD DPIDR 0x0bc11477
Info : at91samd.cpu: hardware has 4 breakpoints, 2 watchpoints
```

So far so good. Let's now write an openocd.cfg file to load our firmware:

```config
# Load the configuration for the MCU over CMSIS-DAP
source [find interface/cmsis-dap.cfg]
source [find target/at91samdXX.cfg]
# Flash the firmware
program your-firmware-file.bin
# Read it back and verify it
verify
# Reset the board
reset
# Close the openocd server
shutdow
```

These are all general openocd command documented [on their
website](http://openocd.org/doc/html/General-Commands.html).

Not so fast...

```terminal
** Programming Started **
auto erase enabled
Info : SAMD MCU: SAMD21G18A (256KB Flash, 32KB RAM)
Error: SAMD: NVM lock error
Error: Failed to erase row containing 00000000
Error: SAMD: failed to erase sector 0
Error: failed erasing sectors 0 to 0
embedded:startup.tcl:476: Error: ** Programming Failed **
```

It turns out our MCU can write protect bootloader pages, so that a firmware bug
does not overwrite the bootloader and brick the device. You can read all about
it in section 9.4 of the
[SAMD20 family datasheet](http://ww1.microchip.com/downloads/en/DeviceDoc/60001504B.pdf).
Typically, boot protection can be overcome with a chip erase, but that did not do the trick
here.

To disable it, we need to zero out the `BOOTPROT` bits (0:2) in the NVM User Row,
which is a fancy name for a non-volatile, 64-bit register at address `0x804000`.

Conveniently, openOCD has a MCU specific command just for this, documented
[here](http://openocd.org/doc-release/html/Flash-Commands.html)! Let's add it to
our script.

```config
# Load the configuration for the MCU over CMSIS-DAP
source [find interface/cmsis-dap.cfg]
source [find target/at91samdXX.cfg]
# Init openocd
init
# Reset & Halt the chip
reset
halt
# Unlock the bootloade regions
at91samd bootloader 0
# Reset the chip
reset
# Flash the firmware
program your-firmware-file.bin
# Read it back and verify it
verify
# Reset the board
reset
# Close the openocd server
shutdow
```

And off we go!

```terminal
** Programming Started **
auto erase enabled
Info : SAMD MCU: SAMD21G18A (256KB Flash, 32KB RAM)
wrote 16384 bytes from file build/minimal/minimal.bin in 3.695003s (4.330 KiB/s)
** Programming Finished **
** Verify Started **
verified 1252 bytes in 0.207606s (5.889 KiB/s)
** Verified OK **
** Resetting Target **
shutdown command invoked
```


