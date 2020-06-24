---
title: "Firmware Update Architecture"
description: "WIP"
author: francois
---

<!-- excerpt start -->
<!-- excerpt end -->

## Table of Contents

<!-- prettier-ignore -->
* auto-gen TOC:
{:toc}

## Setup

All the code in this post was written for the STM32F429 MCU by ST Micro. While
the examples run fine on the STM32F429i discovery board they were developed in
Renode, a popular MCU emulation platform.

You can find the complete code example for this blog post in the [Interrupt
Github repository](https://github.com/memfault/interrupt/tree/master/example/fwup-architecture)

### Renode

Since writing about Renode for Interrupt, I’ve been looking for an opportunity
to use it for another project. This blog post was the perfect pretext. If you
are not familiar with Renode, I recommend reading [my previous blog post]() on
the topic.

Because I use `bin` rather than `elf` files as firmware images, I had to make two
change to the renode configuration:
1. I used `sysbus LoadBinary $bin 0x8000000` rather than `LoadELF` to load the
   firmware.
2. I manually set the Vector Table Offset with `sysbus.cpu VectorTableOffset
   0x8000000`. By default, Renode looks for the vector table at `0x0` which
different from the default behavior of the STM32.

Additionally, I had to modify Renode slightly to enable software-controlled
resets. Cortex-M microcontrollers can be reset by writing the AICR register,
which was not fully implemented in the emulator. As of this writing, this change
is still in review and not yet merged into the emulator. You can find the pull
request [on Github](https://github.com/renode/renode-infrastructure/pull/15/files).

Thankfully, building our own version of renode is relatively
straightforward using [their
instructions](https://renode.readthedocs.io/en/latest/advanced/building_from_sources.html).

I updated my start.sh script to run my home-built Renode instance rather than
the installed binary:

```
#!/bin/sh

RENODE_EXE_PATH=~/code/renode/output/bin/Release/Renode.exe

mono64 $RENODE_EXE_PATH renode-config.resc
```

You will have to update this script to point at your own `Renode.exe`.

### Toolchain

I used the following tools to build my firmware:
* GNU Make 4.2.1 as build system
* `arm-none-eabi-gcc` version 9.2.1 20191025 (release) as compiler

Rather than the STM32Cube HAL, I used an open source MCU HAL called `libopencm3`
with excellent support for the STM32. I find it easier to use, and like that it
is open source and on Github. The included `Makefile` will clone `libopencm3`
during your first build.

### Building & running the example

The example can be built with Make. From the `examples/fwup-architecture`
directory:

```terminal
$ make
  LD            build/fwup-example-boot.elf
  OBJCOPY       build/fwup-example-boot.bin
  LD            build/fwup-example-app.elf
  OBJCOPY       build/fwup-example-app.bin
  XXD           build/app_bin.c
  LD            build/fwup-example-loader.elf
  OBJCOPY       build/fwup-example-loader.bin
  CAT           build/fwup-example.bin
```

After which you can call `./start.sh` to start Renode. You will need to type the
`start` command in the Renode window to get the emulation going.

![](/img/fwup-architecture/renode-running.png)

## High Level Architecture

Over the years, I've come up with a set of basic requirements most DFU systems
should fulfill. Let's walk through  these one by one and iteratively design our
system.

We start with the simplest possible description of what we want to achieve with
DFU: an application that updates itself.

<style>
.diag1 {
    max-width: 400px;
    margin-left: auto;
    margin-right: auto;
}
</style>
{% blockdiag size:120x40 %}
blockdiag {
   // Set labels to nodes.
   A [label = "Application"];
   A -> A [label = "Updates", fontsize=8];
}
{% endblockdiag %}{:.diag1}

This is not a practical design. For one, self modifying code is easy
to mess up. Let's see how we might modify this architecture to get to something
we're happy with.

### DFU should be separate from the application

The only time I ever broke DFU on a device, I did it without changing a line of
code relating to DFU. Unbeknownst to me, our DFU processes depended on an
uninitialized variable which up until then had always ended up being `0`.
Inevitably a new version reshuffled the content of the stack, and all of a
sudden our uninitialized variable held a "1". This prevented DFU from taking
place.[^chris-dfu-debug]

The moral to this story: keep your DFU process and your application code
separate. Firmware update code is critical and should not be changed unless
absolutely necessary. Separating application code from firmware update code
allows us to update our application code without risking problems with DFU.

How do we modify our architecture to meet this requirement? We simply split the
firmware into a "Loader" and an "Application". The loader verifies the
application, runs it, and can update it.

<style>
.diag2 {
    max-width: 800px;
    margin-left: auto;
    margin-right: auto;
}
</style>
{% blockdiag %}
blockdiag {
    span_width = 100;
    // Set labels to nodes.
    A [label = "App Loader"];
    B [label = "Application"];
    A -> B [label = "Loads, Updates", fontsize=8];
}
{% endblockdiag %}{:.diag2}

### DFU code should be updatable

While we want to update our DFU code as little as possible, updating it should
still be *possible*. Invitably we will find bug in our firmware update code
which we must fix. We may want to change our memory map to allocate more code
space to our app, or to rotate a security key baked into our Loader.

But where should the code lives that updates our Loader? It cannot be in the
application, or else we would violate the previous principle. It cannot be in
the Loader itself either. That leaves one option: a third module tasks with
updating the loader. We'll call it the "Updater".

The Updater is loaded by the Loader, perhaps when a specific input is received.
All it knows how to do is update the Loader.

<style>
.diag3 {
    max-width: 800px;
    margin-left: auto;
    margin-right: auto;
}
</style>
{% blockdiag %}
blockdiag {
    span_width = 100;
    // Set labels to nodes.
    A [label = "App Loader"];
    B [label = "Application"];
    A -> B [label = "Loads, Updates", fontsize=8];

    E [label = "Updater"];
    A -> E [label = "Loads, Updates", fontsize=8];
    E -> A [label = "Updates", fontsize=8];
}
{% endblockdiag %}{:.diag3}

### DFU should use minimal code space

Every firmware project I've ever worked on has run out of code space. At Pebble,
we spent months porting our 3.0 firmware to the original watch; most of that
time was spent slimming down the code so it could fit within the 512KB of Flash
available on that device.

With that in mind, we should make sure our Loader and Updater do not take more
code space than absolutely necessary. Are there ways we can update our design to
use less code space? Absolutely!

The key insight here is that the Updater needs to run very rarely, and that it
never needs to coexist with the application. We can therefore use the same
"slot" in flash for both the Updater and the Application. The main tradeoff here
is that our Loader update flow becomes more complicated:

Go to Loader -> Update the Application with the Updater -> Load Updater ->
Update Loader -> Reboot into Loader -> Replace the Updater with the Application

We can tolerate this complexity because it should not be used often.

{% blockdiag %}
blockdiag {
    span_width = 100;
    // Set labels to nodes.
    A [label = "App Loader"];
    B [label = "Application"];
    A -> B [label = "Loads, Updates", fontsize=8];

    E [label = "Updater"];
    A -> E [label = "Loads, Updates", fontsize=8];
    E -> A [label = "Updates", fontsize=8];

    group {
        label = "Slot 1";
        color = "LightPink";
        A;
    }
    group {
        label = "Slot 2";
        color = "LemonChiffon";
        B; E;
    }
}
{% endblockdiag %}{:.diag3}

### DFU failures should not brick the device

This one should be obvious. Whether there is a bug in our DFU process, or a power loss event while we are
writing firmware, the device should be able to recover. It may operate in a
degraded mode for a bit, but it should at least be able to update itself back to
a good state.

Our design already does a reasonable job of this: if we lose power in the middle
of an Application update, we can reboot into our Loader and start our update
again. There are however two failure modes we must deal with.

First, in the event we lose power while updating the Loader, we could find
ourselves with no valid image at the address the chip boots from (`0x0` by
default for Cortex-M, but aliased to `0x80000000` on STM32). The solution is to
add a small, immutable bootloader whose sole job is to sit at the start address
and load our Loader.

Second, what happens if we find ourselves without a functional Loader? We would
want to fall back to the Updater. Here again, the small bootloader is the
solution. In the event no valid Loader is found, it should try to load whatever
is found in the "Application" slot.

<style>
.diag4 {
    max-width: 1200px;
    margin-left: auto;
    margin-right: auto;
}
</style>
{% blockdiag size:120x40 %}
blockdiag {
    span_width = 100;
    // Set labels to nodes.
    C [label = "Bootloader"];
    A [label = "App Loader"];
    B [label = "Application"];
    C -> A [label = "Loads", fontsize=8];
    A -> B [label = "Ld, Updt", fontsize=8];

    E [label = "Updater"];
    A -> E [label = "Ld, Updt", fontsize=8];
    E -> A [label = "Updates", fontsize=8];
    C -> E [label = "Loads", style=dashed, fontsize=8];

    group {
        label = "Slot 0";
        color = "PaleGreen";
        C;
    }
    group {
        label = "Slot 1";
        color = "LightPink";
        A;
    }
    group {
        label = "Slot 2";
        color = "LemonChiffon";
        B; E;
    }
}
{% endblockdiag %}{:.diag4}

## Implementation Cookbook

### Image Metadata

Firmware images typically need to be bundled alongside some metadata. It
identifies what type of image it is, how large it is, what version it is, ...
etc.

That metadata must be:
1. Easy to find and parse for an external program
2. Accessible from within the firmware itself

My favorite approach is to add a struct at the beginning of the image to serve
as a header. Here's what it looks like:

```c
// image.h
typedef struct __attribute__((packed)) {
    uint16_t image_magic;
    uint16_t image_hdr_version;
    uint32_t crc;
    uint32_t data_size;
    uint8_t image_type;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_patch;
    uint32_t vector_addr;
    uint32_t reserved;
    char git_sha[8];
} image_hdr_t;
```

The contents of that header will vary per application, but you must include (1)
a way to indicate that the header is valid (here we used a constant in
`image_magic`), (2) a version for the header, and (3) the address of the start
of your vector table.

Each of our images must fill in its own header. For example, here it is for my
Application:

```c
// app.c
image_hdr_t image_hdr __attribute__((section(".image_hdr"))) = {
    .image_magic = IMAGE_MAGIC,
    .image_hdr_version = IMAGE_VERSION_CURRENT,
    .image_type = IMAGE_TYPE_APP,
    .version_major = 1,
    .version_minor = 0,
    .version_patch = 1,
    .vector_addr = (uint32_t)&vector_table,
    // GIT_SHA is generated by Makefile
    .git_sha = GIT_SHA,
};
```

You will notice the `section` attribute, which is used to place the variable in
a specific section of our firmware. This is paired with some code in our linker
script which declares that sections and puts it at the very start of flash:

```
// app.ld
SECTIONS
{
    .image_hdr : {
        KEEP (*(.image_hdr))
    } > slot2rom
    ...
};
```

Some data can simply be hardcoded into the header. In my case, this includes the
`image_magic`, the semantic version, the image type, the header version, and the
vector table address. Other bits must be read from the environment and injected
as `DEFINES` by our Makefile. For example, this is how we get the `GIT_SHA`:

```makefile
# Makefile
GIT_SHA := \"$(shell $(GIT) rev-parse --short HEAD)\"
DEFINES += GIT_SHA=$(GIT_SHA)
CFLAGS += $(foreach d,$(DEFINES),-D$(d))
```

The `GIT_SHA` macro can then be called in our code to get the short (7-char)
hash as a string.

Some header information can only be calculated *after* the firmware has been
compiled and linked. For example, the length of the binary, its CRC, or its
cryptographic signature. To add it to the header, we must edit the binary
directly.

Typically, I would write a Python script to do this but in a pinch you can use
some shell commands in a Makefile. For example, given `app.elf` I can do the
following to get a `bin` file with the CRC added:

```shell
# Create "app.data", a binary object without our image header
$ arm-none-eabi-objcopy app.elf app.data -O binary --remove-section=.image_hdr
# Compute the CRC and write it to a binary file
$ crc32 app.data | xxd -r -p | xxd -p -c1 | tac | xxd -r -p > app.crc
# Create "app.bin", the full firmware binary
$ arm-none-eabi-objcopy app.elf app.bin -O binary
# Inject the CRC into "app.bin"
$ dd if=app.crc of=app.bin obs=1 seek=4 conv=notrunc
```

That's it! We've added the CRC to our `bin` file, the main complexity really is
the Endian swap. We can now display the information in our firmware:

```c
// app.c
printf("App STARTED - version %d.%d.%d (%s) - CRC 0x%lx\n",
       image_hdr.version_major,
       image_hdr.version_minor,
       image_hdr.version_patch,
       image_hdr.git_sha,
       image_hdr.crc);
```

And as expected we get:

```
App STARTED - version 1.0.1 (e8e8d53) - CRC 0xeb4f7f52
```

### Loading Images

Both the Bootloader and the Loader need to load firmware images and start them.
This includes (1) locating them, (2) verifying them, and (3) starting them.

The code may look something like this:

```c
// boot.c
for (image_slot_t slot = IMAGE_SLOT_1; slot < IMAGE_NUM_SLOTS; ++slot) {
    const image_hdr_t *hdr = image_get_header(slot);
    if (hdr == NULL) {
        continue;
    }
    if (image_validate(slot, hdr) != 0) {
        continue;
    }

    printf("Booting slot %d\n", slot);
    image_start(hdr);
}
```

Here our approach to locating images is simple: we use hardcoded "slots" where
we expect images. Slot 1 is at 0x8004000, and slot 2 is at 0x8020000. We simply
iterate through the slots looking for valid headers.

Remember that the first field of our image header is the "magic" value. In
`image_get_header` we read the header and verify the magic value.

```c
// image.c
const image_hdr_t *image_get_header(image_slot_t slot) {
    const image_hdr_t *hdr = NULL;

    switch (slot) {
        case IMAGE_SLOT_1:
            hdr = (const image_hdr_t *)&__slot1rom_start__;
            break;
        case IMAGE_SLOT_2:
            hdr = (const image_hdr_t *)&__slot2rom_start__;
            break;
        default:
            break;
    }

    if (hdr && hdr->image_magic == IMAGE_MAGIC) {
        return hdr;
    } else {
        return NULL;
    }
}
```

To validate the image, we compute its CRC and compare it with the value written
in the header. This will allow you to catch data corruption or bit errors, and
save you some trouble down the line. Note that verifying CRC on boot is not
appropriate for every applicaton, as it can slow down boot significantly.

```c
int image_validate(image_slot_t slot, const image_hdr_t *hdr) {
    void *addr = (slot == IMAGE_SLOT_1 ? &__slot1rom_start__ : &__slot2rom_start__);
    addr += sizeof(image_hdr_t);
    uint32_t len = hdr->data_size;
    uint32_t a = crc32(addr, len);
    uint32_t b = hdr->crc;
    if (a == b) {
        return 0;
    } else {
        printf("CRC Mismatch: %lx vs %lx\n", a, b);
        return -1;
    }
}
```

Last but not least we start the image by extracting the vector table address
from the header, writing it to the `VTOR` register (which tells the chip that
the vector table address has moved), and branching to it with a bit of assembly.

```c
static void prv_start_image(void *pc, void *sp) {
    __asm("           \n\
          msr msp, r1 \n\
          bx r0       \n\
    ");
}

void image_start(const image_hdr_t *hdr) {
    const vector_table_t *vectors = (const vector_table_t *)hdr->vector_addr;
    SCB_VTOR = (uint32_t)vectors;
    prv_start_image(vectors->reset, vectors->initial_sp_value);
     __builtin_unreachable();
}
```

> Note: The `VTOR` register is available on the Cortex-M0+,M3,M4,M7, but not on
> the older M0. Unfortunately this makes building a bootloader much more
> complicated on that platform.

### Writing & Committing Images


```c
int dfu_commit_image(image_slot_t slot, const image_hdr_t *hdr) {
    uint32_t addr = (uint32_t)(slot == IMAGE_SLOT_1 ?
                               &__slot1rom_start__ : &__slot2rom_start__);
    uint8_t *data_ptr = (uint8_t *)hdr;
    for (int i = 0; i < sizeof(image_hdr_t); ++i) {
        flash_program_byte(addr + i, data_ptr[i]);
    }

    // new app -- reset the boot counter
    shared_memory_clear_boot_counter();

    return 0;
}
```

```c
int dfu_invalidate_image(image_slot_t slot) {
    // We just write 0s over the image header
    uint32_t addr = (uint32_t)(slot == IMAGE_SLOT_1 ?
                               &__slot1rom_start__ : &__slot2rom_start__);
    for (int i = 0; i < sizeof(image_hdr_t); ++i) {
        flash_program_byte(addr + i, 0);
    }

    return 0;
}
```

```c
uint8_t *data_ptr = FIRMWARE_DATA;
image_hdr_t *hdr = (image_hdr_t *)data_ptr;
data_ptr += sizeof(image_hdr_t);

if (dfu_write_data(IMAGE_SLOT_2,
                   data_ptr,
                   build_fwup_example_app_bin_len)) {
    return -1;
}

if (image_validate(IMAGE_SLOT_2, hdr)) {
    return -1;
};

if (dfu_commit_image(IMAGE_SLOT_2, hdr)) {
    return -1;
};

scb_reset_system();
```


### Shared Memory

```c
// shared_memory.c
typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t flags;
    uint8_t boot_counter;
} shared_memory_t;

shared_memory_t shared_memory __attribute__((section(".shared_memory")));
```

```
// memory_map.ld
MEMORY
{
  sharedram (rwx): ORIGIN = 0x20000000, LENGTH = 256
  ram (rwx)      : ORIGIN = 0x20000100, LENGTH = 128K - 256
}

SECTIONS
{
    .shared_memory (NOLOAD) : {
        KEEP(*(.shared_memory))
    } >sharedram
}
```

```c
void shared_memory_init(void) {
    if (shared_memory.magic != MAGIC) {
        printf("Shared memory uinitialized, setting magic\n");
        memset(&shared_memory, 0, sizeof(shared_memory_t));
        shared_memory.magic = MAGIC;
    }
}
```

```c
bool shared_memory_is_dfu_requested(void) {
    return prv_get_flag(DFU_REQUESTED);
}

void shared_memory_set_dfu_requested(bool yes) {
    prv_set_flag(DFU_REQUESTED, yes);
}
```

```c
int cli_command_dfu_mode(int argc, char *argv[]) {
    shell_put_line("Rebooting into DFU mode");
    shared_memory_set_dfu_requested(true);
    scb_reset_system();
    while (1) {}
    return 0;
}
```

```c
// loader.c
if (!shared_memory_is_dfu_requested()) {
    // start app
}

printf("Entering DFU Mode\n");
shared_memory_set_dfu_requested(false);
```

### Boot Stability

## References

- [^chris-dfu-debug]: This is not the end to this story. My cofounder Chris
    eventually found a set of inputs to provide to the device which would set
the stack just so, and allow us to update out of that state. Phew!



