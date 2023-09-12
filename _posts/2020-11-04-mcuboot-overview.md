---
title: "MCUboot Walkthrough and Porting Guide"
description: "A guided tour of the MCUboot feature set and how to port it to a Cortex-M
application"
image: /img/mcuboot/mcuboot.png
tag: [cortex-m, bootloader]
author: chris
---

In previous articles such as [How to Write a Bootloader from Scratch]({% post_url
2019-08-13-how-to-write-a-bootloader-from-scratch %}), [device firmware update strategies]({% post_url 2020-06-23-device-firmware-update-cookbook %}), & [how to securely sign firmware updates]({% post_url 2020-09-08-secure-firmware-updates-with-code-signing %}), we have explored the various parts that make up a bootloader and DFU subsystem. We have seen there are a lot of pieces that need to come together for the system to work reliably and be secure.

Over the last few years, an interesting open-source project, [MCUboot](https://github.com/mcu-tools/mcuboot), has surfaced seeking to
simplify and standardize the approach to these problems.  MCUboot was chosen as the bootloader to
be used with the Zephyr RTOS[^2]. Implementations using MCUboot have even been incorporated in
semiconductor provided SDKs such as Nordic's nRF Connect SDK [^2] for the NRF91 & NRF53 MCUs.

If you are working on a new project, MCUboot is worth a look! At the very least, some of the design
decisions can be used to help guide your own solution.

<!-- excerpt start -->

In this article, we will explore the feature set of MCUboot and how it works. We will walk through
step-by-step how to port it to a custom platform and test our port out on a Cortex-M based development board.

<!-- excerpt end -->

> Note: While the focus of this article is about MCUboot, the general strategy outlined for porting
> MCUboot to a new platform can be used as a guide for incorporating other third-party libraries
> into an embedded project.


{% include newsletter.html %}

{% include toc.html %}

## What is MCUboot?

[MCUboot](https://github.com/mcu-tools/mcuboot) is a library which can be integrated into a bootloader to securely
perform firmware updates. The project seeks to standardize two key areas:

1. Secure Bootloader Functionality: that is, a bootloader that will only launch images that are cryptographically verified
2. Standardized Flash Layout: A common way to define the flash layout of an embedded system. That is, a way to define, where binaries live and how other regions of flash are used.

In addition to these features, MCUboot also includes options for encrypting/decrypting firmware
binaries, performing upgrades in a fault-tolerant manner (i.e resumption across unexpected
reboots), and recovering the system if a bad firmware image is installed.

For a custom project, MCUboot is a C library you would include in your bootloader and main
application. The surface area for the bootloader is one function which will be called to determine
the image to boot or install upgrades:

```c
int main(void) {
  // ...
  struct boot_rsp rsp;
  int rv = boot_go(&rsp);

  if (rv == 0) {
    // 'rsp' contains the start address of the image
    your_platform_do_boot(&rsp);
  }
  ```

Your main application can also use the MCUboot library to read information about the images which
are installed and mark images as ready to be installed after they have been downloaded.

## MCUboot Configuration Options

MCUboot has many configuration options which are controlled by`MCUBOOT_` defines added to a configuration header. The best documentation for the options available that I could find was the [Kconfig](https://github.com/mcu-tools/MCUboot/blob/master/boot/zephyr/Kconfig) wrapper in the port for the Zephyr RTOS.
While a full discussion of configuration options is outside the scope of this article, there are a
few that are particularly interesting and deserve a brief overview.

### Upgrade Strategies

For upgrades, MCUboot uses the following nomenclature:
* `images` - The individual firmware binaries that run on a system. For a typical setup, there will
  be one application image but MCUboot also has some initial support for managing multiple images
  as well. (For example, when using a Cortex-M33 with ARM Trustzone enabled, you may want to have a "Secure" Image and "Non-Secure" Image.)
* `slots` - Every image can be saved in a "primary" slot or "secondary" slot. The "primary" slot is where the binary executes from. The "secondary" slot is where an image is staged prior to being installed.

MCUboot has a number of ways "upgrades" from the "secondary" to "primary" slot can take place:

#### Overwrite Mode

* `MCUBOOT_OVERWRITE_ONLY`: When an update is requested in this mode, the contents of the primary slot are erased and the contents of the secondary slot are copied into the primary slot.

#### Swap Mode

Several of the upgrade strategies MCUboot offers swap the primary and secondary images. If the new
image installed is not working well, one can then revert to the image that was installed before the
upgrade.

* `MCUBOOT_SWAP_USING_MOVE`: This strategy relies on an extra sector of space being in the primary "slot". For this strategy, the bootloader will shift the entire firmware binary in the primary slot up by one sector and then "swap" the sectors between the secondary and primary slots.

* `MCUBOOT_SWAP_USING_SCRATCH`: This relies on allocating a "scratch" flash region. Sectors are temporarily stashed in the scratch region while the primary & secondary slots are being swapped. If the scratch area is small, there can be a lot of wear on the internal flash because every single sector gets copied in here during the swap.

The benefit of the swap approach is the ability to roll back a buggy firmware to a previous
version. The disadvantage of the strategy is it adds a lot of complexity to the bootloader system
and can wear down the flash faster due to the heavier writing. It also means any firmware installed
needs to support downgrades.

> Idea: An alternative and simpler recovery mechanism could be to use `MCUBOOT_OVERWRITE_ONLY` and store a
> "recovery" image elsewhere on flash. If an upgrade is buggy and needs to be reverted, the
> recovery image  could then be copied into the secondary slot and installed from there.

### Crypto Backends

MCUboot does not implement its own cryptographic libraries (thank goodness). Instead, it expects you
to link an existing implementation. There are default ports available for [Tinycrypt](https://github.com/intel/tinycrypt) and [Mbed TLS](https://github.com/ARMmbed/mbedtls) which can be enabled using the `MCUBOOT_USE_TINYCRYPT` and `MCUBOOT_USE_MBED_TLS` flags, respectively.

If a hardware accelerator is available, a port can be implemented to take advantage of that as well. There is already an implementation present for the NRF platform crypto engine which can be enabled with `MCUBOOT_USE_CC310`.

The `MCUBOOT_VALIDATE_PRIMARY_SLOT` flag controls whether or not images are validated on each
boot. The default (and recommended) setting is to validate on every boot. If your image is large or your MCUs clock is slow, a hardware crypto accelerator will likely have a noticeable difference on your boot time.

> For an initial port, I'd recommend starting with one of the software crypto libraries, measuring
> what the boot and upgrade times look like, and making a decision about whether or not the
> hardware accelerator is needed from those results.

## MCUboot Image Binaries

An MCUboot binary image is built by wrapping the original binary with a header and "trailer". There is a Python script, [imgtool.py](https://github.com/mcu-tools/MCUboot/blob/master/scripts/imgtool.py) included in the project which can be used to generate this image.

The header contains basic information about the image binary such as size and versioning information:

```c
// include/bootutil/image.h

#define IMAGE_MAGIC                 0x96f3b83d

#define IMAGE_HEADER_SIZE           32

struct image_version {
    uint8_t iv_major;
    uint8_t iv_minor;
    uint16_t iv_revision;
    uint32_t iv_build_num;
};

/** Image header.  All fields are in little endian byte order. */
struct image_header {
    uint32_t ih_magic;
    uint32_t ih_load_addr;
    uint16_t ih_hdr_size;           /* Size of image header (bytes). */
    uint16_t ih_protect_tlv_size;   /* Size of protected TLV area (bytes). */
    uint32_t ih_img_size;           /* Does not include header. */
    uint32_t ih_flags;              /* IMAGE_F_[...]. */
    struct image_version ih_ver;
    uint32_t _pad1;
};
```

The original binary image is suffixed with Type-Length-Value (TLV) pairs. This is where information
about how the image was signed and encrypted lives. Users can also add custom information if there is
other metadata they would like to add to an image. Using TLV pairs avoids the need to budget space
for features not being used for a particular application and gives the end-user flexibility over the
type of information included.

```c
// include/bootutil/image.h

/*
 * Image trailer TLV types.
 *
 * Signature is generated by computing signature over the image hash.
 * Currently the only image hash type is SHA256.
 *
 * Signature comes in the form of 2 TLVs.
 *   1st on identifies the public key which should be used to verify it.
 *   2nd one is the actual signature.
 */
#define IMAGE_TLV_KEYHASH           0x01   /* hash of the public key */
#define IMAGE_TLV_PUBKEY            0x02   /* public key */
#define IMAGE_TLV_SHA256            0x10   /* SHA256 of image hdr and body */
#define IMAGE_TLV_RSA2048_PSS       0x20   /* RSA2048 of hash output */
#define IMAGE_TLV_ECDSA224          0x21   /* ECDSA of hash output */
#define IMAGE_TLV_ECDSA256          0x22   /* ECDSA of hash output */
#define IMAGE_TLV_RSA3072_PSS       0x23   /* RSA3072 of hash output */
#define IMAGE_TLV_ED25519           0x24   /* ed25519 of hash output */
#define IMAGE_TLV_ENC_RSA2048       0x30   /* Key encrypted with RSA-OAEP-2048 */
#define IMAGE_TLV_ENC_KW128         0x31   /* Key encrypted with AES-KW-128 */
#define IMAGE_TLV_ENC_EC256         0x32   /* Key encrypted with ECIES-EC256 */
#define IMAGE_TLV_ENC_X25519        0x33   /* Key encrypted with ECIES-X25519 */
#define IMAGE_TLV_DEPENDENCY        0x40   /* Image depends on other image */
#define IMAGE_TLV_SEC_CNT           0x50   /* security counter */
#define IMAGE_TLV_BOOT_RECORD       0x60   /* measured boot record */
#define IMAGE_TLV_ANY               0xffff /* Used to iterate over all TLV */

#define IMAGE_TLV_INFO_MAGIC        0x6907
#define IMAGE_TLV_PROT_INFO_MAGIC   0x6908

/** Image TLV header.  All fields in little endian. */
struct image_tlv_info {
    uint16_t it_magic;
    uint16_t it_tlv_tot;  /* size of TLV area (including tlv_info header) */
};

/** Image trailer TLV format. All fields in little endian. */
struct image_tlv {
    uint8_t  it_type;   /* IMAGE_TLV_[...]. */
    uint8_t  _pad;
    uint16_t it_len;    /* Data length (not including TLV header). */
};
```

## Fault Tolerance

Metadata about the staged image and whether or not to perform an upgrade is tracked in what is referred to as "Image Trailers". This information is located at the end of each "slot" in use and looks like this:

```
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ~                                                               ~
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ~                                                               ~
    ~    Swap status (BOOT_MAX_IMG_SECTORS * min-write-size * 3)    ~
    ~                                                               ~
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                 Encryption key 0 (16 octets) [*]              |
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                 Encryption key 1 (16 octets) [*]              |
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                      Swap size (4 octets)                     |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |   Swap info   |           0xff padding (7 octets)             |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |   Copy done   |           0xff padding (7 octets)             |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |   Image OK    |           0xff padding (7 octets)             |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                       MAGIC (16 octets)                       |
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

A full discussion can be found in the design document located inside the MCUboot repository [^4]. In short, on boot, the "Swap status" is checked to resolve if an upgrade was in progress and resume it. The status of "Swap info", "Copy done", & "Image Ok" is checked to decide if an upgrade should be performed or not.

## Example Project Setup

In the sections that follow, we will explore in detail how to port MCUboot to a custom application. If you would like to skip ahead and just try out the bootloader itself, you can build and flash the example bootloader and application included in the interrupt repository.

The example bootloader and application are bare-metal targets that expose a CLI which can be used for trying out MCUboot functionality.
The setup makes use of the following:

- a nRF52840-DK[^nrf5_sdk] (ARM Cortex-M4F) as our development board
- SEGGER JLinkGDBServer[^jlink] as our gdbserver (V6.84a)
- GCC 9.3.1 / GNU Arm Embedded Toolchain as our compiler[^3]
- GNU Make as our build system
- the simple CLI shell [we built up in a previous post]({% post_url 2020-06-09-firmware-shell %}).
- PySerial's `miniterm.py`[^miniterm] to connect to the serial console on the nRF52.

### Installing

Before you get started, I recommend doing a full chip erase on the development board to put the flash in a clean state

```bash
$ nrfjprog --eraseall
```

To flash the board, you will need to start a JLinkGDBServer:

```bash
$ JLinkGDBServer  -if swd -device nRF52840_xxAA  -nogui
```

In another terminal build and flash the bootloader:

```bash
$ git clone git@github.com:memfault/interrupt.git

$ cd example/mcuboot/bootloader/
$ make flash
$ Linking library
Generated bootloader/build/nrf52-bootloader.elf
Loading section .interrupts, size 0x240 lma 0x0
Loading section .text, size 0x4d57 lma 0x240
Loading section .data, size 0x9c lma 0x4f98
Start address 0x240, load size 20531
Transfer rate: 6683 KB/sec, 5132 bytes/write.
Resetting target
(gdb) c

```

In another terminal you can start a CLI and should see:

```
$ miniterm.py - 115200 --raw

[INF] Primary image: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3
[INF] Secondary image: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3
[INF] Boot source: none
[INF] Swap type: none
[DBG] erasing trailer; fa_id=2
[DBG] flash_area_erase: Addr: 0x00047000 Length: 4096
```

You can then flash a working application by compiling and flashing the application.

```bash
$ cd example/mcuboot/application
$ make flash

$ make flash
Compiling external/MCUboot/boot/bootutil/src/bootutil_misc.c
Linking library
Restoring binary file application/build/nrf52-app.bin into memory (0x8000 to 0x9e58)
Resetting target
(gdb) c
```

From miniterm you should now see:

```
== Main Application Booted!==

shell>
```

### Testing Upgrade Functionality

#### 1. Compile a new Application

Compile a new application with some kind of change so you can easily confirm it was installed.

```diff
--- a/example/mcuboot/application/src/main.c
+++ b/example/mcuboot/application/src/main.c
@@ -39,7 +39,8 @@ int main(void) {
   // succesfully completed init, mark image as stable
   boot_set_confirmed();

   EXAMPLE_LOG("==Main Application Booted==");
+  EXAMPLE_LOG("A modified application image");
```

```
$ cd example/mcuboot/application
$ make
```

#### 2. Install the application in the upgrade slot using GDB

```
(gdb) restore ../application/build/nrf52-app.bin binary 0x28000
```

#### 3. Kickoff image upgrade operation

Installing the image that has been written to the secondary slot can be kicked off using the `swap_images` CLI command

```
swap_images: Swap images
reboot: Reboot System
help: Lists all commands
shell> swap_images
...
Starting Main Application
  Image Start Offset: 0x8000
  Vector Table Start Address: 0x8200. PC=0x8771, SP=0x20002000
[DBG] writing image_ok; fa_id=1 off=0x1ffe8 (0x27fe8)
[DBG] flash_area_write: Addr: 0x00027fe8 Length: 4
==Main Application Booted==
A modified application image

shell>
```

## Porting MCUboot

Porting a library into a new application is often an exercise of trial and error as well as patience.

The MCUboot project has a few resources that can help you along:

* [porting guide](https://github.com/mcu-tools/MCUboot/blob/master/docs/PORTING.md)
* [design guide](https://github.com/mcu-tools/MCUboot/blob/master/docs/design.md)
* [Zephyr RTOS](https://github.com/mcu-tools/MCUboot/tree/master/boot/zephyr) and [Mynewt RTOS](https://github.com/mcu-tools/MCUboot/tree/master/boot/mynewt) reference ports.

### Porting a Library Phases

I typically like to breakdown a port of a library into several incremental steps:

1. Add C sources from the library to the project and get everything to compile.
2. Add a call to the library from the application to force the dependencies to be pulled in. Upon completion of this
   step, you will see many "undefined references" at link time.
3. Fix the link-time issues by adding stub implementations for all the dependencies to a port file
   (i.e `mcuboot_port.c`). At this point, we have an application that links but doesn't yet do
   anything. We can ballpark the code size impact of the library by taking a look at the size of our
   image at this point.
4. Walk through and fill in the stub implementations we used to get a link with a real implementation.

With that in mind, let's begin!

### Adding MCUboot to Build System

The porting guide mentions we will need to call `boot_go()` from `boot/bootutil/loader.c` to determine the image to boot. Reading through the documentation [bootutil](https://github.com/mcu-tools/mcuboot/tree/master/boot/bootutil) is the "library" we will want to add to our bootloader. Using CLI tools like `tree` and `ls` we can find this all lives in `boot/bootutil`

```
# tree boot/bootutil/
boot/bootutil/
├── include
│   └── bootutil
# [...]
└── src
# [...]
```

#### Add MCUboot bootutil sources and includes

As a starting point we can add those sources and the include directory to our build system:

```makefile
MCUBOOT_DIR := $(ROOT_DIR)/external/MCUboot/boot/bootutil
MCUBOOT_SRC_DIR := $(MCUBOOT_DIR)/src
MCUBOOT_INC_DIR := $(MCUBOOT_DIR)/include

MCUBOOT_SRC_FILES += \
  $(MCUBOOT_SRC_DIR)/boot_record.c \
  $(MCUBOOT_SRC_DIR)/bootutil_misc.c \
  $(MCUBOOT_SRC_DIR)/caps.c \
  $(MCUBOOT_SRC_DIR)/encrypted.c \
  $(MCUBOOT_SRC_DIR)/fault_injection_hardening.c \
  $(MCUBOOT_SRC_DIR)/fault_injection_hardening_delay_rng_mbedtls.c \
  $(MCUBOOT_SRC_DIR)/image_ec.c \
  $(MCUBOOT_SRC_DIR)/image_ec256.c \
  $(MCUBOOT_SRC_DIR)/image_ed25519.c \
  $(MCUBOOT_SRC_DIR)/image_rsa.c \
  $(MCUBOOT_SRC_DIR)/image_validate.c \
  $(MCUBOOT_SRC_DIR)/loader.c \
  $(MCUBOOT_SRC_DIR)/swap_misc.c \
  $(MCUBOOT_SRC_DIR)/swap_move.c \
  $(MCUBOOT_SRC_DIR)/swap_scratch.c \
  $(MCUBOOT_SRC_DIR)/tlv.c

# Also need mcuboot_config/ on path
MCUBOOT_INC_PATHS := -I$(MCUBOOT_INC_DIR)

# ...
SRC_FILES += $(MCUBOOT_SRC_FILES)
INCLUDE_PATHS += $(MCUBOOT_INC_PATHS)
```

#### Adding `mcuboot_config.h`

With the sources added, let's attempt to compile the sources and fix the fallout!

```
$ cd example/mcuboot/bootloader/
$ make

fault_injection_hardening.h:57:10: fatal error: mcuboot_config/mcuboot_config.h: No such file or directory
57 | #include "mcuboot_config/mcuboot_config.h"
```

Ah missing headers! The porting guide mentions that we need to provide a `mcuboot_config/mcuboot_config.h` configuration file. We can copy `samples/mcuboot_config/mcuboot_config.template.h` into our project and use that as a starting point.

I opted to place these files in `common/include` in the example project because some of the MCUboot library utilities will be called from our main application as well so it will need access to this header.

```bash
$ mkdir -p common/include/mcuboot_config
$ cp external/MCUboot/samples/mcuboot_config/mcuboot_config.template.h common/include/mcuboot_config/mcuboot_config.h
```

In our Makefile, we can add the new include path to build as follows:

```makefile
COMMON_DIR = $(ROOT_DIR)/common
# ...
INCLUDE_PATHS += -I$(COMMON_DIR)/include
```

#### Resolving sysflash.h and flash_map_backend.h dependencies

Attempting to compile again, we see more errors:

```
$ make
fatal error: sysflash/sysflash.h: No such file or directory
   32 | #include "sysflash/sysflash.h"
```

This one isn't explicitly mentioned in the MCUboot porting guide but it looks like we need to
create this header ourselves. Looking at some of the reference ports, this file will contain
mappings between the primary, secondary, & scratch slots and the flash areas they live in. Let's create a stub for now and come back to it:

```bash
$ mkdir -p common/include/sysflash
$ touch  common/include/sysflash/sysflash.h
```

Continuing onward we bump into another missing header!

```bash
$ make
fatal error: flash_map_backend/flash_map_backend.h: No such file or directory
   33 | #include "flash_map_backend/flash_map_backend.h"
   ```

There are some notes about this in the porting document about defining the flash interface. For the port, we need to create a `flash_map_backend.h` with these definitions.

> Oddly, these functions are not defined in a header in the library itself but it
> looks like we are expected to copy/paste them into a new file in our porting layer.

Here's what the file winds up looking like:

```c
// flash_map_backend/flash_map_backend.h
#pragma once

#include <inttypes.h>

struct flash_area {
  uint8_t  fa_id;         /** The slot/scratch identification */
  uint8_t  fa_device_id;  /** The device id (usually there's only one) */
  uint16_t pad16;
  uint32_t fa_off;        /** The flash offset from the beginning */
  uint32_t fa_size;       /** The size of this sector */
};

//! Structure describing a sector within a flash area.
struct flash_sector {
  //! Offset of this sector, from the start of its flash area (not device).
  uint32_t fs_off;

  //! Size of this sector, in bytes.
  uint32_t fs_size;
};

//! Opens the area for use. id is one of the `fa_id`s */
int flash_area_open(uint8_t id, const struct flash_area **area_outp);
void flash_area_close(const struct flash_area *fa);

//! Reads `len` bytes of flash memory at `off` to the buffer at `dst`
int flash_area_read(const struct flash_area *fa, uint32_t off,
                    void *dst, uint32_t len);
//! Writes `len` bytes of flash memory at `off` from the buffer at `src`
int flash_area_write(const struct flash_area *fa, uint32_t off,
                     const void *src, uint32_t len);
//! Erases `len` bytes of flash memory at `off`
int flash_area_erase(const struct flash_area *fa,
                     uint32_t off, uint32_t len);

//! Returns this `flash_area`s alignment
size_t flash_area_align(const struct flash_area *area);
//! Returns the value read from an erased flash area byte
uint8_t flash_area_erased_val(const struct flash_area *area);

//! Given flash area ID, return info about sectors within the area
int flash_area_get_sectors(int fa_id, uint32_t *count,
                           struct flash_sector *sectors);

//! Returns the `fa_id` for slot, where slot is 0 (primary) or 1 (secondary).
//!
//! `image_index` (0 or 1) is the index of the image. Image index is
//!  relevant only when multi-image support support is enabled
int flash_area_id_from_multi_image_slot(int image_index, int slot);
int flash_area_id_from_image_slot(int slot);
```

This part of the API was something I was curious about. There are a lot of ways flash layouts can
differ on embedded systems. For example, sometimes there are multiple flash devices (internal
flash, NOR flash, eMMC). Additionally, flash chips have different erase and write sizes. Furthermore, for
some flash devices, the sector sizes may differ within the same part. Scanning the header, it looks
like all of these description challenges are covered in the API design. Neat!

The last two dependency functions in the list give us a better idea of what needs to be provided in `sysflash/sysflash.h`. We need to define macros MCUboot uses to map between `image_index`and slot id (0=primary, 1=secondary) to the `fa_id` our port uses to identify a flash area. With that in mind we can go back and fill in that header:

```c
//! sysflash/sysflash.h

//! A user-defined identifier for different storage mediums
//! (i.e internal flash, external NOR flash, eMMC, etc)
#define FLASH_DEVICE_INTERNAL_FLASH 0

//! An arbitrarily high slot ID we will use to indicate that
//! there is not slot
#define FLASH_SLOT_DOES_NOT_EXIST 255

//! NB: MCUboot expects this define to exist but it's only used
//! if MCUBOOT_SWAP_USING_SCRATCH=1 is set
#define FLASH_AREA_IMAGE_SCRATCH FLASH_SLOT_DOES_NOT_EXIST

//! The slot we will use to track the bootloader allocation
#define FLASH_AREA_BOOTLOADER 0

//! A mapping to primary and secondary/upgrade slot
//! given an image_index. We'll plan to use
#define FLASH_AREA_IMAGE_PRIMARY(i) ((i == 0) ? 1 : 255)
#define FLASH_AREA_IMAGE_SECONDARY(i) ((i == 0) ? 2 : 255)
```

#### Adding mcuboot_logging.h

With the missing flash header dependencies now resolved, let's try compiling again:

``` bash
$ make
bootutil_log.h:29:10: fatal error: mcuboot_config/mcuboot_logging.h: No such file or directory
29 | #include <mcuboot_config/mcuboot_logging.h>
```

We've bumped into another piece not explicitly mentioned in the porting guide. MCUboot has a logging subsystem. We'll definitely want to have these implemented as we work on doing a bringup of MCUboot on our platform.

I created a minimal header using the Zephyr/MyNewt ports as a guide that plumbs into the "example" logging subsystem in our minimal application:

```c
#pragma once

#include "hal/logging.h"

#define MCUBOOT_LOG_MODULE_DECLARE(...)

#define MCUBOOT_LOG_ERR(_fmt, ...) \
  EXAMPLE_LOG("[ERR] " _fmt, ##__VA_ARGS__);
#define MCUBOOT_LOG_WRN(_fmt, ...) \
  EXAMPLE_LOG("[WRN] " _fmt, ##__VA_ARGS__);
#define MCUBOOT_LOG_INF(_fmt, ...) \
  EXAMPLE_LOG("[INF] " _fmt, ##__VA_ARGS__);
#define MCUBOOT_LOG_DBG(_fmt, ...) \
  EXAMPLE_LOG("[DBG] " _fmt, ##__VA_ARGS__);
  ```

#### Adding os_malloc.h

```bash
$ make
loader.c:38:10: fatal error: os/os_malloc.h: No such file or directory
   38 | #include <os/os_malloc.h>
```

It looks like there are a few dependencies within MCUboot on `malloc`.

> This is something we'd want to audit in more detail before shipping to understand the exact
> amount of space needed. For bootloaders, it is often favorable to have everything statically allocated.

It's nice that a header is exposed that lets us override implementations. In our case since the
example app is making use of the malloc provided by Newlib, we can just `#include <stdlib.h>`:

```c
#pragma once

#include <stdlib.h>
```

### Resolving Undefined References

Finally, everything compiles! As expected, we see a number of undefined reference warnings emitted
by the linker that we now need to work through!

```bash
make 2>&1 | rg ": undefined.*" -o | sort -u
: undefined reference to `_close'
: undefined reference to `_exit'
: undefined reference to `_fstat'
: undefined reference to `_getpid'
: undefined reference to `_isatty'
: undefined reference to `_kill'
: undefined reference to `_lseek'
: undefined reference to `_read'
: undefined reference to `_write'
: undefined reference to `flash_area_align'
: undefined reference to `flash_area_close'
: undefined reference to `flash_area_erase'
: undefined reference to `flash_area_erased_val'
: undefined reference to `flash_area_get_sectors'
: undefined reference to `flash_area_id_from_image_slot'
: undefined reference to `flash_area_id_from_multi_image_slot'
: undefined reference to `flash_area_open'
: undefined reference to `flash_area_read'
: undefined reference to `flash_area_write'
: undefined reference to `tc_sha256_final'
: undefined reference to `tc_sha256_init'
: undefined reference to `tc_sha256_update'
```

Examining the missing symbols we have three classes:

1. Missing Newlib platform dependency implementations. (We've seen these before in [our post about porting newlib]({% post_url 2019-11-12-boostrapping-libc-with-newlib %})).
2. The `tc_sha256_*` are missing implementations for the Tincrypt backend we are using.
3. The `flash_*` dependency functions defined in `flash_map_backend.h` need to be filled in.

#### Remove Dependencies on Newlib stdio

This one is a little strange. Newlib calls like `_write`, `_read`, etc should only get pulled into a
build when there is a dependency on `stdio`. It would be surprising if MCUboot had a dependency like
this. One indirect source of these dependencies is Newlib's `assert()` implementation. Inspecting the MCUboot source, `<assert.h>` is included from a number of files.

However, what we can do instead is override the default assert handler by adding an override to `mcuboot_config.h` which causes `mcuboot_config/mcuboot_assert.h` to be included instead of `<assert.h>`:

```c
// mcuboot_config/mcuboot_config.h
#define MCUBOOT_HAVE_ASSERT_H 1
```

Let's override the default Newlib implementation with a lighter weight one and see if that removes
the Newlib porting dependencies:

```c
// mcuboot_config/mcuboot_assert.h
#pragma once

#include "hal/logging.h"

extern void example_assert_handler(const char *file, int line);

#define assert(exp) \
  do {                                                  \
    if (!(exp)) {                                       \
      example_assert_handler(__FILE__, __LINE__);       \
    }                                                   \
  } while (0)
```

Now let's recompile:

```bash
make  2>&1 | rg ": undefined.*" -o | sort -u
: undefined reference to `example_assert_handler'
: undefined reference to `flash_area_align'
: undefined reference to `flash_area_close'
: undefined reference to `flash_area_erase'
: undefined reference to `flash_area_erased_val'
: undefined reference to `flash_area_get_sectors'
: undefined reference to `flash_area_id_from_image_slot'
: undefined reference to `flash_area_id_from_multi_image_slot'
: undefined reference to `flash_area_open'
: undefined reference to `flash_area_read'
: undefined reference to `flash_area_write'
: undefined reference to `tc_sha256_final'
: undefined reference to `tc_sha256_init'
: undefined reference to `tc_sha256_update'
```

Sweet. All of the Newlib undefined references are gone.

#### Resolving Tinycrypt Dependencies

The `tc_*` calls are library calls into the Tinycrypt library (because we are using `MCUBOOT_USE_TINYCRYPT`). The exact dependencies will change depending on what is enabled in the bootloader (SHA256, Code Signing, Encryption). For the purposes of this initial port, we'll just be using SHA256 based validation to check for image corruption.

Tinycrypt can easily be added to a project by adding the sources to the Makefile. The library is
included as a submodule within the MCUboot repo so we can just pick it up from there:

```makefile
TINYCRYPT_DIR := $(ROOT_DIR)/external/MCUboot/ext/tinycrypt/lib
TINYCRYPT_SRC_DIR := $(TINYCRYPT_DIR)/source
TINYCRYPT_INC_DIR := $(TINYCRYPT_DIR)/include

TINYCRYPT_SRC_FILES += \
  $(TINYCRYPT_SRC_DIR)/aes_decrypt.c \
  $(TINYCRYPT_SRC_DIR)/aes_encrypt.c \
  $(TINYCRYPT_SRC_DIR)/cbc_mode.c \
  $(TINYCRYPT_SRC_DIR)/ccm_mode.c \
  $(TINYCRYPT_SRC_DIR)/cmac_mode.c \
  $(TINYCRYPT_SRC_DIR)/ctr_mode.c \
  $(TINYCRYPT_SRC_DIR)/ctr_prng.c \
  $(TINYCRYPT_SRC_DIR)/hmac.c \
  $(TINYCRYPT_SRC_DIR)/hmac_prng.c \
  $(TINYCRYPT_SRC_DIR)/sha256.c \
  $(TINYCRYPT_SRC_DIR)/utils.c

TINYCRYPT_INC_PATHS := -I$(TINYCRYPT_INC_DIR)
# [...]
SRC_FILES += $(TINYCRYPT_SRC_FILES)
INCLUDE_PATHS += $(TINYCRYPT_INC_PATHS)
```

We can rebuild and see the `tc_sha256*` dependencies have been resolved:

```bash
make  2>&1 | rg ": undefined.*" -o | sort -u
: undefined reference to `example_assert_handler'
: undefined reference to `flash_area_align'
: undefined reference to `flash_area_close'
: undefined reference to `flash_area_erase'
: undefined reference to `flash_area_erased_val'
: undefined reference to `flash_area_get_sectors'
: undefined reference to `flash_area_id_from_image_slot'
: undefined reference to `flash_area_id_from_multi_image_slot'
: undefined reference to `flash_area_open'
: undefined reference to `flash_area_read'
: undefined reference to `flash_area_write'
```

#### Resolving Flash Dependencies

Finally, we have gotten to the meat of the port, the flash dependencies required by MCUboot. Let's
add stub implementations for each of the dependency functions that just return error codes when
called.

```c
// mcu_port.c

#include <string.h>
#include <stdlib.h>

#include "flash_map_backend/flash_map_backend.h"
#include "os/os_heap.h"
#include "sysflash/sysflash.h"

#include "hal/logging.h"
#include "hal/internal_flash.h"

int flash_area_open(uint8_t id, const struct flash_area **area_outp) {
  return -1;
}

void flash_area_close(const struct flash_area *area) {
}

int flash_area_read(const struct flash_area *fa, uint32_t off, void *dst,
                    uint32_t len) {
  return -1;
}

int flash_area_write(const struct flash_area *fa, uint32_t off, const void *src,
                     uint32_t len) {
  return -1;
}

int flash_area_erase(const struct flash_area *fa, uint32_t off, uint32_t len) {
  return -1;
}

size_t flash_area_align(const struct flash_area *area) {
  return 1;
}

uint8_t flash_area_erased_val(const struct flash_area *area) {
  return 0xff;
}

int flash_area_get_sectors(int fa_id, uint32_t *count,
                           struct flash_sector *sectors) {
  return -1;
}

int flash_area_id_from_multi_image_slot(int image_index, int slot) {
  return -1;
}

int flash_area_id_from_image_slot(int slot) {
  return flash_area_id_from_multi_image_slot(0, slot);
}

void example_assert_handler(const char *file, int line) {
}
```


We can add the file to our build system

```
SRC_FILES += $(COMMON_SRC_DIR)/mcuboot_port.c
```

```
Linking library
Generated bootloader/build/nrf52-bootloader.elf
```

Success, we have a build!

At this point, we can use `arm-none-eabi-size` to get an approximation of how big our bootloader will be:

```
arm-none-eabi-size bootloader/build/nrf52-bootloader.elf
   text    data     bss     dec     hex	filename
   21295        108   14052   35455    8a7f	bootloader/build/nrf52-bootloader.elf
```

We can also flash the image on a running board and see what happens.

```
[WRN] Failed reading sectors; BOOT_MAX_IMG_SECTORS=128 - too small?
No bootable image found. Falling into Bootloader CLI:

shell>
```

As expected, `boot_go()` fails because we have not yet implemented the flash dependencies.


### Provisioning Flash

Now that we have a working build, we will need to allocate the flash available on our system to
different "flash areas".

> For a production application, it's important to think through these allocations because once we have shipped a bootloader using them, we will not be able to change them!

Notably, for the flash storage available, we will need to look up the following for each flash available on the system:

* the total size of the flash
* the erase size of sectors within each flash

This information is generally available in one of the vendor datasheets or SDKs. For example, in
the NRF52840, we have 1MB of internal flash with erase sector sizes of 4kB.

> Fun fact: For other MCUs, the layout can be a bit more complex. For example, sometimes the first few sectors
>will be smaller (so a bootloader can be put in a small slot). In particular, the layouts for STM32 parts can get quite interesting:
>
> ![]({% img_url MCUboot/stm32f4-flash-example.png %})


We saw above that the binary size of the bootloader is ~20kB  (`.text` + `.data`) so let's budget
for up to 32kB for bootloader space. We'll then use two (arbitrarily sized) 128kB slots for app images and leave the rest of internal flash for data storage.

This gives us the following target layout for our application:

|Slot | Flash Area Id |Start | End | Size
|Bootloader| 0 |0x0|0x8000| 32kB|
|App Primary Slot| 1|0x8000|0x28000| 128kB|
|App Upgrade Slot| 2|0x28000|0x48000| 128kB|
|Data Storage| |0x48000|0x100000| 736kB|


> For a real application, it's likely the configuration will be more complex than this. For example, the upgrade slot may live on external flash to support larger binaries on the device.

### Implementing Flash Backend Dependencies

With a layout decided upon, we can start to fill in our stub implementations in `mcuboot_port.c`. The first step will be defining our "flash areas":

```c
// mcuboot_port.c

#define BOOTLOADER_START_ADDRESS 0x0
#define BOOTLOADER_SIZE (32 * 1024)
#define APPLICATION_PRIMARY_START_ADDRESS (32 * 1024)
#define APPLICATION_SECONDARY_START_ADDRESS (APPLICATION_PRIMARY_START_ADDRESS + APPLICATION_SIZE)
#define APPLICATION_SIZE (128 * 1024)

static const struct flash_area bootloader = {
  .fa_id = FLASH_AREA_BOOTLOADER,
  .fa_device_id = FLASH_DEVICE_INTERNAL_FLASH,
  .fa_off = BOOTLOADER_START_ADDRESS,
  .fa_size = BOOTLOADER_SIZE,
};

static const struct flash_area primary_img0 = {
  .fa_id = FLASH_AREA_IMAGE_PRIMARY(0),
  .fa_device_id = FLASH_DEVICE_INTERNAL_FLASH,
  .fa_off = APPLICATION_PRIMARY_START_ADDRESS,
  .fa_size = APPLICATION_SIZE,
};

static const struct flash_area secondary_img0 = {
  .fa_id = FLASH_AREA_IMAGE_SECONDARY(0),
  .fa_device_id = FLASH_DEVICE_INTERNAL_FLASH,
  .fa_off = APPLICATION_SECONDARY_START_ADDRESS,
  .fa_size = APPLICATION_SIZE,
};

static const struct flash_area *s_flash_areas[] = {
  &bootloader,
  &primary_img0,
  &secondary_img0,
};
```

where,

- `fa_id` is a **unique** id for the flash area being referenced. MCUboot will be accessing the primary & secondary slots for our images but should never touch the bootloader region. The actual "id" used is entirely up to us. MCUboot looks up the id using the `FLASH_AREA_IMAGE_PRIMARY/SECONDARY` macros.
- `fa_device_id` is a unique identifier for the flash storage. If there are multiple backing storages in use, this gives us a way to implement different behavior based on the storage being accessed. For our example setup, we will only have one device id to represent internal flash.
- `fa_off` - is the offset within the flash area device where the slot starts. We will use this
  value in dependency function implementations to determine what physical address to write to within the flash.
- `fa_size` is the total space available in the slot.

> Note these regions must all be defined along erase sector boundaries for upgrades to work correctly and to avoid corrupting adjacent slots!

#### Implementing `flash_area_open` / `flash_area_close`

This is how MCUboot determines information about flash areas being written to / read from. The "id" passed into the function is the "slot" it would like to try and read. In our setup, since we have defined all the flash regions statically at compile time, we can walk through the structure and return the appropriate information to the library:

```c
#define ARRAY_SIZE(arr) sizeof(arr)/sizeof(arr[0])

static const struct flash_area *prv_lookup_flash_area(uint8_t id) {
  for (size_t i = 0; i < ARRAY_SIZE(s_flash_areas); i++) {
    const struct flash_area *area = s_flash_areas[i];
    if (id == area->fa_id) {
      return area;
    }
  }
  return NULL;
}

int flash_area_open(uint8_t id, const struct flash_area **area_outp) {
  MCUBOOT_LOG_DBG("%s: ID=%d", __func__, (int)id);

  const struct flash_area *area = prv_lookup_flash_area(id);
  *area_outp = area;
  return area != NULL ? 0 : -1;
}

void flash_area_close(const struct flash_area *fa) {
  // no cleanup to do for this flash part
}
```

#### Implementing `flash_area_id_from_multi_image_slot`

MCUboot needs to be able to resolve the "flash area id" to read from a given image index and slot id.
The "slot" argument is set to 0 when looking up the primary slot and 1 to lookup the secondary slot.

```c
int flash_area_id_from_multi_image_slot(int image_index, int slot) {
    switch (slot) {
      case 0:
        return FLASH_AREA_IMAGE_PRIMARY(image_index);
      case 1:
        return FLASH_AREA_IMAGE_SECONDARY(image_index);
    }

    MCUBOOT_LOG_ERR("Unexpected Request: image_index=%d, slot=%d", image_index, slot);
    return -1; /* flash_area_open will fail on that */
}
```

> If you are using the `MCUBOOT_SWAP_USING_SCRATCH=1` configuration, the `FLASH_AREA_IMAGE_SCRATCH`
> slot identifier may also be passed in the "slot" argument and will need to be handled.

```c
int flash_area_id_from_image_slot(int slot) {
  return flash_area_id_from_multi_image_slot(0, slot);
}
```

#### Implementing `flash_area_align`, `flash_area_erased_val`, `flash_area_get_sectors`

There are several dependencies used to describe various flash device properties. These are used by
MCUboot so it can issue erase and write requests that comply with the flash device in use. For the NRF52 internal flash, we have:

```c
#define FLASH_SECTOR_SIZE 4096

size_t flash_area_align(const struct flash_area *area) {
  // the smallest unit a flash write can occur along.
  // Note: Image trailers will be scaled by this size
  return 4;
}

uint8_t flash_area_erased_val(const struct flash_area *area) {
  // the value a byte reads when erased on storage.
  return 0xff;
}

int flash_area_get_sectors(int fa_id, uint32_t *count,
                           struct flash_sector *sectors) {
  const struct flash_area *fa = prv_lookup_flash_area(fa_id);
  if (fa->fa_device_id != FLASH_DEVICE_INTERNAL_FLASH) {
    return -1;
  }

  // All sectors for the NRF52 are the same size
  const size_t sector_size = FLASH_SECTOR_SIZE;
  uint32_t total_count = 0;
  for (size_t off = 0; off < fa->fa_size; off += sector_size) {
    // Note: Offset here is relative to flash area, not device
    sectors[total_count].fs_off = off;
    sectors[total_count].fs_size = sector_size;
    total_count++;
  }

  *count = total_count;
  return 0;
}
```

#### Implementing `flash_area_read`, `flash_area_write`, `flash_area_erase`

Finally, we need to implement the APIs that MCUboot uses to program flash. The example application
used in this article has a minimal flash driver implementation for the NRF52 in
`minimal_nrf52_flash.c`. The functions are all named `example_internal_flash_*`. Most vendor SDKs
provide something similar that you could use for these implementations.

For initial bringup, we will also add:
* Diagnostic logging so we can see what addresses writes and erases are being issued to
* A validation pass that makes sure writes and erases are working as expected. A bug in how we are performing program operations is generally the most common reason an image will fail to install so best to catch it at the source.

Taking all that into account, we have the following:

```c
//! Useful for bringup to make sure the write
//! and erase operations are behaving as expected
#define VALIDATE_PROGRAM_OP 1

int flash_area_read(const struct flash_area *fa, uint32_t off, void *dst,
                    uint32_t len) {
  if (fa->fa_device_id != FLASH_DEVICE_INTERNAL_FLASH) {
    return -1;
  }

  const uint32_t end_offset = off + len;
  if (end_offset > fa->fa_size) {
    MCUBOOT_LOG_ERR("%s: Out of Bounds (0x%x vs 0x%x)", __func__, end_offset, fa->fa_size);
    return -1;
  }

  // internal flash is memory mapped so just dereference the address
  void *addr = (void *)(fa->fa_off + off);
  memcpy(dst, addr, len);

  return 0;
}

int flash_area_write(const struct flash_area *fa, uint32_t off, const void *src,
                     uint32_t len) {
  if (fa->fa_device_id != FLASH_DEVICE_INTERNAL_FLASH) {
    return -1;
  }

  const uint32_t end_offset = off + len;
  if (end_offset > fa->fa_size) {
    MCUBOOT_LOG_ERR("%s: Out of Bounds (0x%x vs 0x%x)", __func__, end_offset, fa->fa_size);
    return -1;
  }

  const uint32_t addr = fa->fa_off + off;
  MCUBOOT_LOG_DBG("%s: Addr: 0x%08x Length: %d", __func__, (int)addr, (int)len);
  example_internal_flash_write(addr, src, len);

#if VALIDATE_PROGRAM_OP
  if (memcmp((void *)addr, src, len) != 0) {
    MCUBOOT_LOG_ERR("%s: Program Failed", __func__);
    assert(0);
  }
#endif

  return 0;
}

int flash_area_erase(const struct flash_area *fa, uint32_t off, uint32_t len) {
  if (fa->fa_device_id != FLASH_DEVICE_INTERNAL_FLASH) {
    return -1;
  }

  if ((len % FLASH_SECTOR_SIZE) != 0 || (off % FLASH_SECTOR_SIZE) != 0) {
    MCUBOOT_LOG_ERR("%s: Not aligned on sector Offset: 0x%x Length: 0x%x", __func__,
                    (int)off, (int)len);
    return -1;
  }

  const uint32_t start_addr = fa->fa_off + off;
  MCUBOOT_LOG_DBG("%s: Addr: 0x%08x Length: %d", __func__, (int)start_addr, (int)len);

  for (size_t i = 0; i < len; i+= FLASH_SECTOR_SIZE) {
    const uint32_t addr = start_addr + i;
    example_internal_flash_erase_sector(addr);
  }

#if VALIDATE_PROGRAM_OP
  for (size_t i = 0; i < len; i++) {
    uint8_t *val = (void *)(start_addr + i);
    if (*val != 0xff) {
      MCUBOOT_LOG_ERR("%s: Erase at 0x%x Failed", __func__, (int)val);
      assert(0);
    }
  }
#endif

  return 0;
}
```

#### Implementing `example_assert_handler`

Lastly, we need to provide an implementation for the custom assert handler we added. For example:

```c
void example_assert_handler(const char *file, int line) {
  EXAMPLE_LOG("ASSERT: File: %s Line: %d", file, line);
  __builtin_trap();
}
```

## Creating an MCUboot Image

At this point, we have implemented all the dependencies required to perform an upgrade within MCU boot.

Now that we have ported the bootloader itself, we need to generate an MCUboot compatible binary
image that can be launched. This can easily be done using the `imgtool` utility included in the project. The tool can operate on a binary file.

As we have discussed in [this post]({% post_url 2019-08-13-how-to-write-a-bootloader-from-scratch %}#putting-it-all-together), an ELF can be converted to a binary using `objcopy`:

```bash
$ arm-none-eabi-objcopy build/nrf52-app.elf nrf52-app-no-header.bin -O binary
```

The `imgtool sign` command can be used to add the MCUboot Image header and trailer to the binary image:

```bash
$ python external/MCUboot/scripts/imgtool.py sign \
    --header-size 0x200 \
    --align 4 \
    --slot-size 0x20000 \
    --version 1.0.0 \
    --pad-header \
    build/nrf52-app-no-header.bin build/nrf52-app.bin
```

where

* `--header-size` Controls the size of the binary header. Usually, the ARM vector table comes immediately after the image header so we need to make sure things are sized appropriately such that the vector table is [aligned appropriately]({% post_url 2019-08-13-how-to-write-a-bootloader-from-scratch %}#making-our-app-bootloadable)
* `--pad-header` causes the binary passed to be prefixed with the header information. Another
  option is to pre-add a zero-filled region to the binary and then it will be filled with the
  header.
* `--slot-size` matches the size we have provisioned on device for the slot

We can automate the generation of this binary from the ELF by adding it to our Makefile:

```makefile
$(MCUBOOT_IMG0_BIN): $(TARGET_BIN)
    @python $(IMGTOOL_PY) sign --header-size 0x200 --align 8 -S 131072 -v 1.0.0 --pad-header $(TARGET_BIN) $(MCUBOOT_IMG0_BIN)
    @echo "Generated $(patsubst $(ROOT_DIR)/%,%,$@)"

$(TARGET_BIN): $(TARGET_ELF)
    @arm-none-eabi-objcopy $< $@ -O binary
    @echo "Generated $(patsubst $(ROOT_DIR)/%,%,$@)"

$(TARGET_ELF): $(OBJ_FILES) $(LDSCRIPT)
    @echo "Linking library"
    @arm-none-eabi-gcc $(CFLAGS) $(LDFLAGS) $(OBJ_FILES) -o $@ -Wl,-lc -Wl,-lgcc
    @echo "Generated $(patsubst $(ROOT_DIR)/%,%,$@)"
```

The image can also be signed by providing the `--key` argument and encrypted with the `--encrypt`
argument.

## Launching an Application

The `boot_go()` routine returns a struct populated with information about the image to boot. It's
the bootloader applications responsibility to actually launch the application from the
information:

```c
static void start_app(uint32_t pc, uint32_t sp) {
  __asm volatile ("MSR msp, %0" : : "r" (sp) : );
  void (*application_reset_handler)(void) = (void *)pc;
  application_reset_handler();
}

static void do_boot(struct boot_rsp *rsp) {
  EXAMPLE_LOG("Starting Main Application");
  EXAMPLE_LOG("  Image Start Offset: 0x%x", (int)rsp->br_image_off);

  // We run from internal flash. Base address of this medium is 0x0
  uint32_t vector_table = 0x0 + rsp->br_image_off + rsp->br_hdr->ih_hdr_size;

  uint32_t *app_code = (uint32_t *)vector_table;
  uint32_t app_sp = app_code[0];
  uint32_t app_start = app_code[1];

  EXAMPLE_LOG("  Vector Table Start Address: 0x%x. PC=0x%x, SP=0x%x",
    (int)vector_table, app_start, app_sp);

  // We need to move the vector table to reflect the location of the main application
  volatile uint32_t *vtor = (uint32_t *)0xE000ED08;
  *vtor = (vector_table & 0xFFFFFFF8);

  start_app(app_start, app_sp);
}
```

## Triggering Firmware Upgrades from Main Application

MCUboot has a few functions that we will typically want to call from the main application to put the
firmware update process in motion:

1. `boot_set_pending(int permanent)` - This triggers a write to the "image trailer" in flash that
   marks an upgrade of the primary slot to the data in the secondary slot has been requested. The
   swap can either be permanent or temporary. If it is temporary, `boot_set_confirmed()` must be
   called after the upgrade to prevent a revert to the previous image. Next time the bootloader
   starts, it will start to perform the operation requested.
2. `boot_set_confirmed()` - If `boot_set_pending` was called with `permanent=0`, the bootloader
   will swap back to the older image if a restart takes place and `boot_set_confirmed()` has not
   been called.

Both of these utilities can be picked up by compiling `bootutil_misc.c` and the `mcuboot_port.c` file into the main application which is what we have done for the example application.

I've added a `swap_images` CLI command which calls `boot_set_pending()` and triggers a
reboot. `boot_set_confirmed()` is called by the main application once initialization routines have
run successfully.

```c
static int prv_swap_images(int argc, char *argv[]) {
  EXAMPLE_LOG("Triggering Image Swap");

  const int permanent = 0;
  boot_set_pending(permanent);
  prv_reboot();
  return 0;
}
```

We can simulate a DFU by copying the application binary into our secondary slot for image 0 with GDB. Recall this slot starts at address `0x28000`:

```
(gdb) restore application/build/nrf52-app.bin binary 0x28000
```

After running the GDB command, we can call `swap_images` from the shell and we should see an upgrade kickoff in the bootloader:

```
shell> swap_images
Triggering Image Swap
[INF] Primary image: magic=good, swap_type=0x2, copy_done=0x1, image_ok=0x1
[INF] Secondary image: magic=good, swap_type=0x2, copy_done=0x3, image_ok=0x3
[INF] Boot source: none
[INF] Swap type: test
[DBG] erasing trailer; fa_id=1
[DBG] flash_area_erase: Addr: 0x00027000 Length: 4096
[DBG] initializing status; fa_id=1
# ...
[DBG] writing copy_done; fa_id=1 off=0x1ffe0 (0x27fe0)
[DBG] flash_area_write: Addr: 0x00027fe0 Length: 4
Starting Main Application
  Image Start Offset: 0x8000
  Vector Table Start Address: 0x8200. PC=0x8771, SP=0x20002000
```

Sweet, it works!

## Conclusion

I hope this article taught you a little bit about what MCUboot is, the features it offers, and how
to go about porting it to your own application. I definitely think MCUboot is worth a look if you find yourself tasked with implementing a secure bootloader for a Cortex-M MCU. A lot of the tricky signing & binary packaging problems have already been thought through and tested in the field with this implementation.

With major semiconductor companies shipping MCUboot ports as part of their SDKs, I see the bootloader only becoming more popular in the years to come.

If you have used MCUboot before, I'd be especially curious to hear what your experiences have been, what configuration flags you have found most useful, and what else you would have mentioned in a post about it!

Either way, looking forward to hearing your thoughts in the discussion area below or [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/).

{:.no_toc}

## Further Reading

A few interesting articles I've enjoyed about MCUboot:

- [MCUboot Security](https://www.zephyrproject.org/MCUboot-security-part-1/)
- [Zephyr & MCUboot security assessment](https://research.nccgroup.com/2020/05/26/research-report-zephyr-and-MCUboot-security-assessment/)

## References

[^1]: [MCUboot Github Project](https://github.com/mcu-tools/MCUboot)
[^2]: [nRF Connect SDK using MCUboot](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/mcuboot/readme-ncs.html)
[^3]: [imgtool script](https://github.com/mcu-tools/MCUboot/blob/master/scripts/imgtool.py)
[^4]: [Details about image trailer](https://github.com/mcu-tools/MCUboot/blob/master/docs/design.md#image-trailer)
[^miniterm]: [PySerial Miniterm](https://pyserial.readthedocs.io/en/latest/tools.html#module-serial.tools.miniterm)
[^nrf5_sdk]: [nRF52840 Development Kit](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK)
[^jlink]: [JLinkGDBServer](https://www.segger.com/products/debug-probes/j-link/tools/j-link-gdb-server/about-j-link-gdb-server/)
