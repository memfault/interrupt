---
# This is a template for Interrupt posts. Use previous, more recent posts from the _posts/
# directory for more inspiration and patterns.
# 
# When submitting a post for publishing, submit a PR with the post in the _drafts/ directory
# and it will be moved to _posts/ on the date of publish.
# 
# e.g.
# $ cp _drafts/_template.md _drafts/my_post.md 
# 
# It will now show up in the front page when running Jekyll locally.

title: Separating unique parameters from firmware
description:
  Post Description (~140 words, used for discoverability and SEO)
author: amundas
image: img/<post-slug>/cover.png # 1200x630
---

When writing firmware for embedded systems, more often than not this firmware is meant to run on a number of identical devices. These devices often have their own ID, keys or some other unique parameters. When developing firmware most of us start out by hardcoding these parameters and recompiling the code every time a new device needs to be programmed. As the project matures it becomes more important to keep track of firmware versions, the compile-time might increase, and in a production environment it might be important to know which unique IDs have been used.


<!-- excerpt start -->
In this post we take a look at a method for separating unique parameters such as keys and IDs from firmware.
This enables us to program these parameters separately from the firmware, making our setup much more scalable and easier to keep tidy. 

<!-- excerpt end -->


_Like Interrupt? [Subscribe](http://eepurl.com/gpRedv) to get our latest posts
straight to your mailbox_

{:.no_toc}

## Table of Contents

<!-- prettier-ignore -->
* auto-gen TOC:
{:toc}


### Example project
As an example, we will use SAMR35 based LoRaWAN nodes as the devices.  Simple IoT devices are good examples of devices which often have identical hardware and firmware but need to have a unique ID and their own encryption keys. For LoRaWAN devices using Over The Air Activation (OTAA) these unique parameters are DevEUI (unique device ID) AppEUI (unique application ID) and AppKey (encryption key used during activation).

### A note on security
In this example, we will store an encryption key in standard flash memory. Whether this is a good idea or not depends on your application, although most MCUs allow you to enable readback protection, these mechanisms have been defeated for a number of popular devices. Secure key storage can be done with secure elements like the ATECC608A, or special purpose registers if your device includes these. The goal with this post is to show a method for programming unique parameters separately from the firmware, security considerations is a topic for another post.

### Preparing the firmware

```c
const uint8_t demoDevEui[8] =  { 0xbe, 0xef, 0xba, 0xbe, 0xco, 0xde, 0xco, 0xde };
```

*Reference parameters on some unused address, keep this address space free of everything else. mention OTA, two banks etc?*

```c
#define KEYS_BASE_ADDR      0x18000
const uint8_t *DevEui =     (uint8_t *) KEYS_BASE_ADDR;
const uint8_t *AppEui =     (uint8_t *) KEYS_BASE_ADDR + 8;
const uint8_t *AppKey =     (uint8_t *) KEYS_BASE_ADDR + 16;
```

### Preparing the unique parameters
As stated, the idea is to program the unique parameters separately from the firmware. Personally, I have experimented with a couple of different ways of preparing the parameters, one option is to create C files containing the parameters before compiling them to binaries that can be flashed to the correct addresses. This method does, however, contain quite a lot of steps, and I have found that generating Intel HEX files directly to be easier. Since HEX files specify the addresses at which the data should be flashed, the following steps become quite simple. The [Wikipedia entry](https://en.wikipedia.org/wiki/Intel_HEX) on the Intel HEX format explains the file format quite clearly, based on this I created a python script that generates HEX files.

**TODO: Explain the Intel HEX format here instead of referring to Wikipedia?**


### Generating files and keeping them tidy
In a production environment, and during development for that matter, it becomes important to have a clear structure when storing files. In this example, we will store the unique parameters for IoT nodes in folders based on some name (e.g serial number) and keep track of which are used and which are free. **TODO: argue that these should be generated ahead of time**. In addition, we will create a human-readable file containing the same information as the HEX files to be stored in the same folder. We will continue with a structure like this: Free parameters are stored in `params/free/<node_name>` and used parameters in `params/used/<node_name>`.


Since we are using a LoRa node as an example device, we will register new devices with The Things Network (TTN) using the ttnctl CLI. This will register a new device in your selected TTN application, and return the parameters that we want. The shell script below registers a device, generates a human-readable file, and finally calls the python script to generate a corresponding HEX file. 

```shell
#!/usr/bin/env bash
# The sed command below removes ANSI color formatting
output="$(ttnctl devices register some-name | sed 's/\x1b\[[0-9;]*m//g')"

AppEUI=$(echo "$output" | sed -n -r '0,/.*AppEUI=([0-9A-F]{16})\s{1,}.*/s//\1/p')
DevEUI=$(echo "$output" | sed -n -r '/.*DevEUI=([0-9A-F]{16})\s{1,}.*/s//\1/p')
AppKey=$(echo "$output" | sed -n -r '/.*AppKey=([0-9A-F]{32})\s{1,}.*/s//\1/p')
DevID=$(echo "$output" | sed -n -r '0,/.*DevID=(.*)/s//\1/p')
 
# Store keys in a human readable file
mkdir ../keys/free/$DevID
printf "DevID: $DevID\nDevEUI: $DevEUI\nAppEUI: $AppEUI\nAppKey: $AppKey\n" > ../keys/free/$DevID/$DevID.key
# Create hex file with keys
./generateHexFile.py $DevEUI $AppEUI $AppKey $DevID

```
**TODO: Rabbit hole; TTN might generate DevEUIs marked as globally unique although they are not, it is better to generate your own (or buy a set from IEEE) not marked as globally unique and specify them in the ttnctl command**

### Programming a device with our new setup
*Show makefile example with JLinkEXE*

*Make a point out of only using a few short commands for the process at the end.* 










### Python script

```python
#!/usr/bin/env python3
#generateHexFile.py
import sys
# The hex files puts data at (baseAddr << 4) +  addr, which is why we shift
baseAddr = 0x18000
baseAddr = baseAddr >> 4
if len(sys.argv) != 5 :
   print("Wrong number of arguments, usage:\n ./generateHexFile.py <DevEUI> <AppEUI> <AppKey> <NodeName>")
   sys.exit()
 
hexFile = open("../keys/free/" + sys.argv[4] + "/" + sys.argv[4] + ".hex", 'w+')
 
# Probably not working for all record types. I use it for data, end of file, and extended segment address
def getRecordLine(address, nBytes, recordType, dataString):
   dataString = "{:02X}{:04X}{:02X}".format(nBytes, address, recordType) + dataString
   data = int(dataString, 16)
   checksum = 0
   for i in range(nBytes + 4):
       checksum += (data >> i*8) & 0xff
   checksum = (~checksum +1) & 0xff
   dataString = ":" + dataString + "{:02X}".format(checksum)
   return dataString
 
# Write a line setting a base address for subsequent lines
hexFile.write(getRecordLine(0, 2, 2, "{:04X}".format(baseAddr)) + "\n")
 
# Write lines placing keys at desired positions in flash
hexFile.write(getRecordLine(0     ,  8, 0, str(sys.argv[1])) + "\n")
hexFile.write(getRecordLine(0 + 8 ,  8, 0, str(sys.argv[2])) + "\n")
hexFile.write(getRecordLine(0 + 16, 16, 0, str(sys.argv[3])) + "\n")
 
# End of file record. Using this means that we have to flash two hex files, and not merge this file into an existing file
# This is probably the safest way to go anyway
hexFile.write(getRecordLine(0, 0, 1, ""))
 
hexFile.close()


```



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
