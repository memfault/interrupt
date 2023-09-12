---
title: Separating Unique Parameters from Firmware Binaries
description: Post covering how to provision unique parameters, such as serial numbers or public and private encryption keys, onto firmware-based devices.
author: amundas
tags: [toolchain, build-system]
---

When writing firmware for embedded systems, the firmware is usually meant to run
on many identical devices. These devices often have their own serial
number, public and private keys, or some other unique parameters. These
parameters need to be provisioned per device, either by generating unique
firmware binaries or by writing the values to the device's persistent storage, like
external flash or the EEPROM.

During the firmware development phase, most of us start out by hardcoding these
parameters and recompiling the code every time a new device needs to be
programmed. While this is fine during the early development stages, it doesn't
scale as the number of devices grows.

<!-- excerpt start -->

In this post, we take a look at a method for separating unique parameters such
as keys and IDs from firmware. This enables us to program these parameters
separately from the firmware, making our setup much more scalable and suitable
for use in production.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Setup

Before we get started let's write down our requirements and describe our setup.

### Requirements

Our goal is simple:
1. We want a general method to separately load firmware and unique parameters.
   onto devices
2. We want to keep the number of programming steps to a minimum and rely on
   widely available tools.
3. We want to use off the shelf tools such as Make, Python, and shell scripts
   rather than build a custom solution.

### Hardware and Software

This blog post was written using a SAMR35-based LoRaWAN node.

LoRaWAN relies on a few device parameters that have to be unique:
1. the DevEUI (unique device ID)
2. the AppEUI (unique application ID)
3. the AppKey (encryption key used during activation)

 The LoRa nodes for this example are connected to [The Things
Network](https://www.thethingsnetwork.org/) (a.k.a. "TTN"), which has a CLI that we can
use to get the parameters and register new devices.

All of our firmware is written in C and compiled with GCC. We use J-Link
Commander for programming, and a `Makefile` with some bash scripts for
automation.

## Preparing our Firmware

The first thing we have to do is to look for hardcoded identifiers and remove
them from our firmware. For example, it is common to declare a device ID as a
C-array:

```c
const uint8_t device_eui[8] = { 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
```

Instead, let's allocate some storage for those identifiers and point our
firmware to it.

You can choose to put unique parameters in any non-volatile storage
available on the device. If you're lucky enough to have memory-mapped flash
available on your MCU, this is often the simplest place to use.

In this case, we have plenty of flash so we will allocate some addresses towards
the end of our flash and use them to store our identifiers:

```c
#define KEYS_BASE_ADDR          0x18000
const uint8_t *device_eui   =   (uint8_t *) KEYS_BASE_ADDR;
const uint8_t *app_eui      =   (uint8_t *) KEYS_BASE_ADDR + 8;
const uint8_t *app_key      =   (uint8_t *) KEYS_BASE_ADDR + 16;
```

You may also create a section in your linker script to achieve the same result.

> **A Note on Security** In this post, we store encryption keys in standard flash
> memory. Whether or not this is a good idea depends on your application.
> Although most MCUs allow you to enable readback protection, these mechanisms
> have been defeated for a number of popular devices [^nrf51_bug]
> [^ncc_readback]. Secure key storage can instead be done using secure elements
> like the ATECC608A[^atecc608a], or special purpose registers if your device
> includes them.
> More advanced security considerations are a topic for another post.

## Generating Parameter Files

With that done, we need to generate a file that contains our unique parameters.
We need that file to contain the data as well as the address at which it should
be written.

One option would be to create C files containing the parameters before compiling
them to binaries that can be flashed to the correct addresses. Instead, we
settled on creating IHEX files since they exactly meet our requirements, are
simple to generate and come with lots of great tooling.

### Building HEX File Programatically

Hex files consist of lines of hexadecimal data, called records, that specify
both an address and the data to write at this address. The
[Wikipedia entry](https://en.wikipedia.org/wiki/Intel_HEX) on the Intel HEX
format explains the file format quite clearly.

Based on this we can create a Python script that generates our HEX files with
the given parameters at the desired addresses. The Python code below does just
that:

```python
#!/usr/bin/env python3

# generateHexFile.py
# usage: ./generateHexFile.py <DevEUI> <AppEUI> <AppKey> <NodeName>"
import sys
# The hex format puts data at (baseAddr << 4) +  addr, which is why we shift
baseAddr = 0x18000
baseAddr = baseAddr >> 4

hexOut = ""

# Probably not working for all record types.
#  I use it for data, end of file, and extended segment address
def getRecordLine(address, nBytes, recordType, dataString):
   record = "%02X%04X%02X%s"%(nBytes, address, recordType, dataString)
   data = int(record, 16)
   checksum = 0
   for i in range(nBytes + 4):
       checksum += (data >> i*8) & 0xff
   checksum = (~checksum +1) & 0xff
   record = ":%s%02X"%(record,checksum)
   return record

# Write a line setting a base address for subsequent lines
hexOut += getRecordLine(0, 2, 2, "{:04X}".format(baseAddr)) + "\n"

# Write lines placing parameters at desired positions in flash
hexOut += getRecordLine(0     ,  8, 0, str(sys.argv[1])) + "\n"
hexOut += getRecordLine(0 + 8 ,  8, 0, str(sys.argv[2])) + "\n"
hexOut += getRecordLine(0 + 16, 16, 0, str(sys.argv[3])) + "\n"

# End of file record
hexOut += getRecordLine(0, 0, 1, "")

#print to stdout
print(hexOut)
```

### Automating File Generation

Now that we have a file format and a script to generate it, let's create some
provisioning files!

Since we are registering our devices with TTN, we will use the TTN CLI
(ttnctl)[^ttnctl] to register a new device in our selected TTN application and
return the parameters that we want. This is done by calling `ttnctl devices
register [node_name]` where `node_name` is a unique string we use to identify
that node, oftentimes this is simply the serial number of the device.

While the generation of the files and the registration of new devices could be
done just in time when programming new devices, I believe it is better to
generate a bunch of files ahead of time. This makes the production setup more
robust since it would not depend on some external service which may not be
reachable from the factory floor.

To keep things organized, we will store all of our generated artifacts in folders
based on the node name and keep track of which are used and which are free. The
free parameters are stored in `keys/free/<node_name>` and used parameters
in `keys/used/<node_name>`.

In addition to the hex file, we also generate a human-readable file containing
the unique parameters and store it in the same folder. It might come in
handy!

The shell script below registers a device, generates a human-readable file, and
finally calls the Python script to generate a corresponding HEX file. There is a
lot of sed commands here to get hold of the parameters we need, but the general
idea is what's important.

```shell
#!/usr/bin/env bash

# Usage: ./registerNode.sh <node-name>.
# Files ../keys/free/node-name/node-name.key and
#  ../keys/free/node-name/node-name.hex will be created

# The sed command below removes ANSI color formatting
output="$(ttnctl devices register "$1" | sed 's/\x1b\[[0-9;]*m//g')"

AppEUI=$(echo "$output" | sed -n -r '0,/.*AppEUI=([0-9A-F]{16})\s{1,}.*/s//\1/p')
DevEUI=$(echo "$output" | sed -n -r '/.*DevEUI=([0-9A-F]{16})\s{1,}.*/s//\1/p')
AppKey=$(echo "$output" | sed -n -r '/.*AppKey=([0-9A-F]{32})\s{1,}.*/s//\1/p')
DevID=$(echo "$output" | sed -n -r '0,/.*DevID=(.*)/s//\1/p')

# Store keys in a human readable file
mkdir ../keys/free/$DevID
printf "DevID: $DevID\nDevEUI: $DevEUI\nAppEUI: $AppEUI\nAppKey: $AppKey\n" > ../keys/free/$DevID/$DevID.key
# Create hex file with keys
./generateHexFile.py $DevEUI $AppEUI $AppKey $DevID > ../keys/free/$DevID/$DevID.hex
```

Here is an example of the resulting human-readable file
(`keys/free/node_id/node_id.key`), generated by the script:

```
DevID: node_id
DevEUI: 8899AABBCCDDEEFF
AppEUI: 0011223344556677
AppKey: 00112233445566778899AABBCCDDEEFF
```

The content of the corresponding HEX file `keys/free/node_id/node_id.hex` is
below. Although the same data is somewhat readable, you hopefully agree that it
was a good idea to also generate the more easily readable version of the file.

```
:020000021800E4
:080000008899AABBCCDDEEFFDC
:08000800001122334455667714
:1000100000112233445566778899AABBCCDDEEFFE8
:00000001FF
```

> The shell script in this section lets TTN come up with a DevEUI for
> simplicity. Check out
> [this link](https://www.thethingsnetwork.org/forum/t/deveui-for-non-hardware-assigned-values/2093/23)
> to see why you should not do that in production.

## Programming a Device

Loading `hex` files is supported by most loaders and debuggers. I chose to use
JLink's excellent tools[^jlinkexe], but could just as well have used vendor
toolchains such as `nrfjprog` from Nordic or Simplicity Commander from Silicon
Labs.

The JLink command to load a hex file is `loadfile [path].hex\nr\nq`.

Rather than invoke JLink Commander directly, I tend to use a phony Make target
to flash my devices, but if you prefer to use something else that should be
easily achievable. We want our Make target to flash the firmware and the unique
parameters of our device, as well as moving a newly used set of parameters to
the "used" folder.

The Make target generates a J-Link Commander script that loads the
firmware and the keys of a specified device, then the target executes the
script, and moves the keys to the "used" folder if this is the first time they
have been used.

The Makefile I used can be found below.

```make
# Device name. Set develop as default
NODE_NAME=develop

# "used" or "free". Default to reusing a key since develop will most likely already be used
KEY_FOLDER=used

# flashing a free key will automatically place it in the used keys folder
.PHONY: flash
flash:
	@printf "device ATSAMR35J17\nspeed auto\nif SWD\nloadfile ./build/bin/firmware.hex\n" > flashScript.jlink
	@printf "loadfile ./keys/$(KEY_FOLDER)/$(NODE_NAME)/$(NODE_NAME).hex\nr\nq" >> flashScript.jlink
	JLinkExe -CommanderScript flashScript.jlink
ifeq ($(KEY_FOLDER), free))
	mv ./keys/free/$(NODE_NAME) ./keys/used/$(NODE_NAME)
endif
```

Flashing a new device can now be done by running:

```
$ make flash NODE_ID=<nodeid> KEY_FOLDER=free
```

## Sharing Keys Across a Team

We have created a relatively tidy folder structure, but if these files are
stored locally on a single developer's machine or checked into a git repository,
you will still have a mess on your hands!

The solution to this problem is simply to use a cloud-storage synced folder for
storing the keys (Dropbox, Drive, Onedrive, etc). This will also give you a safe
backup of these important files, and an easy way to share them across a team.

If you are sharing sensitive keys, such as private keys used for secure
connections to a cloud provider, it would also be advised to encrypt these files
and folders at rest using a passphrase or to use a secret key management
service such as Hashicorp Vault.

## Final Thoughts

I hope this post has convinced you that it isn't too hard to move away from
hardcoded identifiers. As always, there are many ways to achieve the same thing,
the method described here has been my preferred method after trying a lot of
different solutions with varying success. Feel free to comment or contact me if
you have any ideas related to this that you would like to share!

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^atecc608a]: [Microchip ATECC608A Secure Element](https://www.microchip.com/wwwproducts/en/ATECC608A)
[^ttnctl]: [The Things Network CLI](https://www.thethingsnetwork.org/docs/network/cli/quick-start.html)
[^jlinkexe]: [J-Link Commander](https://www.segger.com/products/debug-probes/j-link/tools/j-link-commander/)
[^nrf51_bug]: [nRF51 readback protection buypass](https://blog.includesecurity.com/2015/11/NordicSemi-ARM-SoC-Firmware-dumping-technique.html)
[^ncc_readback]: [Whitepaper on MCU readback protection buypass](https://research.nccgroup.com/wp-content/uploads/2020/02/NCC-Group-Whitepaper-Microcontroller-Readback-Protection-1.pdf)
<!-- prettier-ignore-end -->
