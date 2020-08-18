---
title: Separating unique parameters from firmware
description:
  Post Description (~140 words, used for discoverability and SEO)
author: amundas
---

When writing firmware for embedded systems, more often than not this firmware is
meant to run on a number of identical devices. These devices often have their
own ID, keys or some other unique parameters. When developing firmware most of
us start out by hardcoding these parameters and recompiling the code every time
a new device needs to be programmed. While this is fine for most of us during
early development, it becomes increasingly harder to keep things tidy as the
number of devices grows.

<!-- excerpt start -->

In this post, we take a look at a method for separating unique parameters such
as keys and IDs from firmware. This enables us to program these parameters
separately from the firmware, making our setup much more scalable and suitable
for use in production.

<!-- excerpt end -->

_Like Interrupt? [Subscribe](http://eepurl.com/gpRedv) to get our latest posts
straight to your mailbox_

{:.no_toc}

## Table of Contents

<!-- prettier-ignore -->
* auto-gen TOC:
{:toc}

## The Goal

The goal of this post is to present a general method for how to make programming
of a common firmware, and some unique parameters two separate things. Since we
do not want to make things more complicated for ourselves when flashing the
devices, we want to keep the number of final programming steps to a minimum. We
will use Make, python and shell scripts to automate as much as possible,
including keeping track of which devices have been programmed.

### Example devices

As an example, we will use SAMR35 based LoRaWAN nodes for our devices. Simple
IoT devices often have identical hardware and firmware but need to have a unique
ID and their own encryption keys. For LoRaWAN devices using Over The Air
Activation (OTAA) these unique parameters are DevEUI (unique device ID), AppEUI
(unique application ID) and AppKey (encryption key used during activation). The
LoRa nodes for this example are connected to
[The Things Network](https://www.thethingsnetwork.org/) (TTN), which has a CLI
that we can use to get the parameters and register new devices. The firmware for
these example devices is written in C. The environment in this example uses
bash, GCC, a makefile, and J-Link Commander for programming.

### A note on security

In this example, we will store an encryption key in standard flash memory.
Whether this is a good idea or not depends on your application, although most
MCUs allow you to enable readback protection, these mechanisms have been
defeated for a number of popular devices. Secure key storage can be done with
secure elements like the ATECC608A, or special purpose registers if your device
includes these. The goal with this post is to show a method for programming
unique parameters separately from the firmware, security considerations is a
topic for another post.

## Preparing the firmware

In order to reach our goal, the first thing we have to do is to replace our
hardcoded values from the firmware. In my case, I used to have some lines in a C
file where the device-specific variables where declared as follows:

```c
const uint8_t device_eui[8] = { 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
```

Now instead, we will tell the firmware where to find the unique parameters, but
we won't hardcode their values. For this, we need some addresses that we know
won't be used for anything else. Here I used an address near the end of the
flash-memory of my MCU. Now the firmware is ready, and the next step is to put
the unique parameters at the addresses that we specified here.

```c
#define KEYS_BASE_ADDR          0x18000
const uint8_t *device_eui   =   (uint8_t *) KEYS_BASE_ADDR;
const uint8_t *app_eui      =   (uint8_t *) KEYS_BASE_ADDR + 8;
const uint8_t *app_key      =   (uint8_t *) KEYS_BASE_ADDR + 16;
```

## Preparing the unique parameters

### Creating HEX files

Personally, I have experimented with a couple of different ways of preparing the
parameters, one option is to create C files containing the parameters before
compiling them to binaries that can be flashed to the correct addresses. This
method does, however, contain quite a lot of steps, and I have found that
generating Intel HEX files directly to be easier. Hex files basically consist of
lines of hexadecimal data, called records, that specify both an address and the
data to write at this address. The
[Wikipedia entry](https://en.wikipedia.org/wiki/Intel_HEX) on the Intel HEX
format explains the file format quite clearly, based on this we can create a
python script that generates HEX files with our parameters at the addresses we
specified in the firmware.

```python
#!/usr/bin/env python3
#generateHexFile.py
#usage: ./generateHexFile.py <DevEUI> <AppEUI> <AppKey> <NodeName>"
import sys
# The hex format puts data at (baseAddr << 4) +  addr, which is why we shift
baseAddr = 0x18000
baseAddr = baseAddr >> 4

hexOut = ""

# Probably not working for all record types. I use it for data, end of file, and extended segment address
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

### Generating files and keeping them tidy

The next steps are to get the actual values of the parameters and to structure
our files in a tidy fashion. In this example, we will store the unique
parameters for IoT nodes in folders based on some name (e.g serial number) and
keep track of which are used and which are free. In addition, we will create a
human-readable file containing the same information as the HEX files to be
stored in the same folder. We will continue with a structure like this: Free
parameters are stored in `keys/free/<node_name>` and used parameters in
`keys/used/<node_name>`.

While the generation of the files and the registration of new devices could be
done when programming new devices, I would argue that it is better to generate a
bunch of files ahead of time. This makes the production setup more robust since
it would not depend on some external service.

Since we are registering our devices with TTN, we will use the TTN CLI (ttnctl)
to register a new device in our selected TTN application and return the
parameters that we want. The shell script below registers a device, generates a
human-readable file, and finally calls the python script to generate a
corresponding HEX file. There is a lot of sed commands here to get hold of the
parameters we need, but the general idea is what's important.

```shell

#!/usr/bin/env bash
# Usage: ./registerNode.sh <node-name>. Files ../keys/free/node-name/node-name.key and ../keys/free/node-name/node-name.hex will be created
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

An example of a human-readable file, `keys/free/node_id/node_id.key`, generated
by the script is shown below:

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

_The shell script in this section lets TTN come up with a DevEUI for simplicity,
check out
[this link](https://www.thethingsnetwork.org/forum/t/deveui-for-non-hardware-assigned-values/2093/22)
to see why you should not do that._

## Programming a device with our new setup

Personally, I tend to use a phony make target to flash my devices, but if you
prefer to use something else that should be easily achievable. We want our make
target to flash the firmware and the unique parameters of our device, as well as
moving a newly used set of parameters to the "used" folder.

In this example, the make target will call J-Link Commander (JLinkExe) in order
to actually flash the device. A lot of the manufacturer-specific tools such as
nrfjprog or simplicity commander are based on JlinkExe. I have personally found
it easier to generate a jlink script before flashing than to try and control a
J-link Commander session from a make target.

The make target below generates a J-Link Commander script that loads the
firmware and the keys of a specified device, then the target executes the
script, and moves the keys to the "used" folder if this is the first time they
have been used. Flashing a new device can now be done by running
`make flash NODE_ID=<nodeid> KEY_FOLDER=free`.

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

## Sharing keys across a team

I have used the word "tidy" to describe the folder structure used here several
times, but if these files are stored locally on a single developer's machine, or
checked into a git repository, tidy won't be the word you are thinking of! The
solution to this problem that I have been using, is simply to use a
cloud-storage synced folder for storing the keys (Dropbox, Drive, Onedrive etc).
This will also give you a safe backup of these important files, and an easy way
to share them across a team.

## Final thoughts

Hopefully you will find this post helpful when creating a new project, or
tidying up an old one! As always, there are many ways to achieve the same thing,
the method described here has been my preferred method after trying a lot of
different solutions with varying success. Feel free to comment or contact me if
you have any ideas related to this that you would like to share!

<!-- Interrupt Keep START -->

_Like Interrupt? [Subscribe](http://eepurl.com/gpRedv) to get our latest posts
straight to your mailbox_

See anything you'd like to change? Submit a pull request or open an issue at
[GitHub](https://github.com/memfault/interrupt)

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^reference_key]: [Post Title](https://example.com)
<!-- prettier-ignore-end -->
