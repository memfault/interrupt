---
title: "GNU Build IDs for Firmware"
description: "An overview of firmware versions and how to enable and use GNU Build IDs to uniquely identify a firmware build or binary"
author: francois
tags: [best-practices]
---

<!-- excerpt start -->
In this post, we demonstrate how to use the GNU Build ID to uniquely
identify a build. We explain what the GNU build ID is, how it is enabled,
and how it is used in a firmware context.
<!-- excerpt end -->

Much has been written on how to craft a firmware version. From Embedded
Artistry's excellent [blog
post](https://embeddedartistry.com/blog/2016/10/27/giving-you-build-a-version),
to Wolfram Rösler's [how to](https://gitlab.com/wolframroesler/version).

Versions are a great way to identify a *release*: a set of interfaces and
capabilities bundled together.

Versions do not - however - identify a specific binary all that well. For
example, you could have multiple binaries for a given version in order to
accommodate different variants of your hardware in the field.

For this, we need something else. This is where the GNU build ID comes in.

Why would we want to identify a specific binary? A few cases:

* To match a set of debug symbols to a given binary when trying to debug a
  device
* To verify that two binaries are in fact the same build

## What is the GNU Build ID

Firmware engineers are not alone in wanting to uniquely identify a build for
debugging purposes. In fact, Linux developers have long wanted to match a
coredump to a specific build.

Roland McGrath of glibc fame came up with the GNU build ID 15 years ago, and
contributed the implementation to various build tools.

The build ID is a 160-bit SHA1 string computed over the elf header bits and
section contents in the file. It is bundled in the elf file as an entry in the
notes section.

Each note section entry has the following layout:

```
+----------------+
|     namesz     |   32-bit, size of "name" field
+----------------+
|     descsz     |   32-bit, size of "desc" field
+----------------+
|      type      |   32-bit, vendor specific "type"
+----------------+
|      name      |   "namesz" bytes, null-terminated string
+----------------+
|      desc      |   "descsz" bytes, binary data
+----------------+

```

In the case of the GNU build ID:
* `name` is `"GNU\0"`, which gives us `namesz` = 4
* `desc` is our 160-bit SHA1 value, which gives us `descsz` = 20
* `type` is 3

## Adding the GNU build ID to your builds

**Note**: all of our example are based on the
[minimal](https://github.com/memfault/zero-to-main/tree/master/minimal) program from our Zero to main()
series.

In GCC, you can enable build IDs with the `-Wl,--build-id` which passes the
`--build-id` flag to the linker. You can then read it back by dumping the notes
section of the resulting elf file with `readelf -n`.

By default, this is not enabled:

```terminal
francois-mba:minimal francois$ make clean all
...
arm-none-eabi-size build/minimal.elf
   text    data     bss     dec     hex filename
   1252       0    8192    9444    24e4 build/minimal.elf
...
francois-mba:minimal francois$ arm-none-eabi-readelf -n build/minimal.elf
[No output]
```

But a small change to the CFLAGS is all it takes:
```terminal
francois-mba:minimal francois$ CFLAGS="-Wl,--build-id" make clean all
build/minimal.elf
...
arm-none-eabi-size build/minimal.elf
   text    data     bss     dec     hex filename
   1288       0    8192    9480    2508 build/minimal.elf
francois-mba:minimal francois$ arm-none-eabi-readelf -n build/minimal.elf

Displaying notes found in: .note.gnu.build-id
  Owner                 Data size       Description
  GNU                  0x00000014       NT_GNU_BUILD_ID (unique build ID
bitstring)
    Build ID: bab6b09f86b3c3017499d8e386447a610c559bd5
```

As you can see, our binary now contains the build id
`bab6b09f86b3c3017499d8e386447a610c559bd5`.

## Bundling the GNU build ID in firmware bin files

Getting the build ID in your executables on Linux is easy: they are ELF files!
Firmware on the other hand typically deals with binaries which are assembled by
copying relevant sections of the elf at the right offset in a file.

This is typically accomplished with objcopy:
```terminal
$ arm-none-eabi-objcopy firmware.elf firmware.bin -O binary
```

This takes every elf section earmarked to be loaded and places them at
the correct offset in the bin file. In the process, most debug sections are stripped out.

Dumping the elf sections of the resulting `minimal.elf` gives us:

```
$ arm-none-eabi-objdump -h build/minimal.elf

build/minimal.elf:     file format elf32-littlearm

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .note.gnu.build-id 00000024  00000000  00000000  00010000  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  1 .text         000004e4  00000024  00000024  00010024  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  2 .bss          00000000  20000000  20000000  00000000  2**0
                  ALLOC
  3 .data         00000000  20000000  20000000  00010508  2**0
                  CONTENTS, ALLOC, LOAD, DATA
  4 .stack        00002000  20000000  20000000  00020000  2**0
                  ALLOC
  5 .debug_info   00004076  00000000  00000000  00010508  2**0
                  CONTENTS, READONLY, DEBUGGING
  6 .debug_abbrev 00000ac3  00000000  00000000  0001457e  2**0
                  CONTENTS, READONLY, DEBUGGING
  7 .debug_aranges 000000f8  00000000  00000000  00015041  2**0
                  CONTENTS, READONLY, DEBUGGING
  8 .debug_ranges 000000b8  00000000  00000000  00015139  2**0
                  CONTENTS, READONLY, DEBUGGING
  9 .debug_line   00000cfa  00000000  00000000  000151f1  2**0
                  CONTENTS, READONLY, DEBUGGING
 10 .debug_str    0000111a  00000000  00000000  00015eeb  2**0
                  CONTENTS, READONLY, DEBUGGING
 11 .comment      00000075  00000000  00000000  00017005  2**0
                  CONTENTS, READONLY
 12 .ARM.attributes 0000002c  00000000  00000000  0001707a  2**0
                  CONTENTS, READONLY
 13 .debug_frame  00000298  00000000  00000000  000170a8  2**2
                  CONTENTS, READONLY, DEBUGGING
```

As you can see, the `.text`, `.bss`, `.data`, `.stack` sections each have the
`LOAD` attribute, all others (including our `.note.gnu.build-id`) do not and will be
discarded.

To add a section to our binary, we must specify an address for it in our linker
script. Assuming your linker script declares the following memory layout:

```
MEMORY
{
  rom      (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00040000
  ram      (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00008000
}
```

You can add the build ID to your flash memory with:

```
.gnu_build_id :
{
  PROVIDE(g_note_build_id = .);
  *(.note.gnu.build-id)
} > rom
```

This instructs the linker to append the contents of `.note.gnu.build-id` to the
`rom` region of memory and create a symbol (`g_note_build_id`) to point to it.

Let's check our elf sections now:

```
$ arm-none-eabi-objdump -h build/minimal.elf

build/minimal.elf:     file format elf32-littlearm

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text         000004e4  00000000  00000000  00010000  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .gnu_build_id 00000024  000004e4  000004e4  000104e4  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  2 .bss          00000000  20000000  20000000  00000000  2**0
                  ALLOC
  3 .data         00000000  20000000  20000000  00010508  2**0
                  CONTENTS, ALLOC, LOAD, DATA
  4 .stack        00002000  20000000  20000000  00020000  2**0
                  ALLOC
  5 .debug_info   00004076  00000000  00000000  00010508  2**0
                  CONTENTS, READONLY, DEBUGGING
  6 .debug_abbrev 00000ac3  00000000  00000000  0001457e  2**0
                  CONTENTS, READONLY, DEBUGGING
  7 .debug_aranges 000000f8  00000000  00000000  00015041  2**0
                  CONTENTS, READONLY, DEBUGGING
  8 .debug_ranges 000000b8  00000000  00000000  00015139  2**0
                  CONTENTS, READONLY, DEBUGGING
  9 .debug_line   00000cfa  00000000  00000000  000151f1  2**0
                  CONTENTS, READONLY, DEBUGGING
 10 .debug_str    0000111a  00000000  00000000  00015eeb  2**0
                  CONTENTS, READONLY, DEBUGGING
 11 .comment      00000075  00000000  00000000  00017005  2**0
                  CONTENTS, READONLY
 12 .ARM.attributes 0000002c  00000000  00000000  0001707a  2**0
                  CONTENTS, READONLY
 13 .debug_frame  00000298  00000000  00000000  000170a8  2**2
                  CONTENTS, READONLY, DEBUGGING
```

As you can see, our build ID now has an address assigned to it and is marked
with the `LOAD` attribute.

**Note**: Make sure to declare the `.gnu_build_id` section after the `.text`
section, otherwise the build ID will be set to address `0x0` and the firmware
will not boot.

## Reading the build ID in firmware

Once the build ID is in our binary, we need to modify our firmware to read it
and, at the very least, print it over serial at boot.

From the linker script, we know that we will find the build ID section at
`&g_note_build_id`. From the spec, we know the structure of the section and can
write down a typedef:

```c
typedef struct {
    uint32_t namesz;
    uint32_t descsz;
    uint32_t type;
    uint8_t data[];
} ElfNoteSection_t;

extern const ElfNoteSection_t g_note_build_id;
```

We can now simply index into the data field and print the build ID data.

```c
void print_build_id(void) {
    const uint8_t *build_id_data = &g_note_build_id.data[g_note_build_id.namesz];

    printf("Build ID: ");
    for (int i = 0; i < g_note_build_id.descsz; ++i) {
        printf("%02x", build_id_data[i]);
    }
    printf("\n");
}
```

Calling this code on boot, we get:
```
...
Build ID: 8d7aec8b900dce6c14afe557dc8889230518be3e
...
```

> Update: A prior version of the above code was incorrect: `g_note_build_id` was
> declared as a pointer which would lead to random data being read in the best case,
> and a crash in the worst case. Thanks to [Simon
> Doppler](https://github.com/dopsi) for reporting the
> problem.

## References

- [Original build ID proposal](https://fedoraproject.org/wiki/RolandMcGrath/BuildID)
- [Elf Note section description](https://docs.oracle.com/cd/E23824_01/html/819-0690/chapter6-18048.html)
- [GNU LD source code for Build
  ID](https://github.com/bminor/binutils-gdb/blob/4de283e4b5f21207fe12f99913d1f28d4f07843c/ld/emultempl/elf32.em#L1165)
