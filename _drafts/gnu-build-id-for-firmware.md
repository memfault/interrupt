---
title: "GNU Build IDs for Firmware"
author: francois
---

Much has been written on how to craft a firmware version. From Embedded
Artistry's excellent [blog
post](https://embeddedartistry.com/blog/2016/10/27/giving-you-build-a-version),
to Wolfram Rösler's [how to](https://gitlab.com/wolframroesler/version).

Versions are a great way to identify a *release*: a set of interfaces and
capabilities bundled together.

Versions do not - however - identify a specific binary all that well. For
example, you could have multiple binaries for a given version in order to
accommodate different variants of your hardware in the field.

Why would we want to identify a specific binaries? A few cases:

* To match a set of debug symbols to a given binary when trying to debug a
  device
* To verify that two binaries are in fact the same build

In this post, we will talk about how to use the GNU Build ID to uniquely
identify a build.

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

In GCC, you can enable build IDs with the `-Wl,--build-id` which passed the
`--build-id` flag to the linker. You can then read it back by dumping the notes
section of the resulting elf file with `readelf -n`.

```
francois-mba:minimal francois$ make clean all
...
arm-none-eabi-size build/minimal.elf
   text    data     bss     dec     hex filename
   1252       0    8192    9444    24e4 build/minimal.elf
...
francois-mba:minimal francois$ arm-none-eabi-readelf -n build/minimal.elf
[No output]
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

## Bundling the GNU build ID in firmware bin files

Getting the build ID in your executables on Linux is easy: they are ELF files!
Firmware on the other hand typically deals with binaries which are assembling by
copying relevant sections of the elf at the right offset in a file.

This is typically accomplished with objcopy:
```
$ arm-none-eabi-objcopy firmware.elf firmware.bin -O binary
```

This will take every elf section with an address and placing them at that offset
in the bin file. In the process, most debug sections are stripped out.

For this post, we'll use our "minimal" program from our [Zero to main()](XXX)
series. Dumping the elf sections of the resulting `minimal.elf` gives us:

```
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
MEMORY¬
{¬
  rom      (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00040000¬
  ram      (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00008000¬
}¬
```

You can add the build ID to your flash memory with:

```
.gnu_build_id :
{
  PROVIDE(g_gnu_build_id = .);
  *(.note.gnu.build-id)
} > rom
```

This instructs the linker to append the contents of `.note.gnu.build-id` to the
`rom` region of memory and create a symbol (`g_gnu_build_id`) to point to it.

Let's check our elf sections now:

```

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

## Reading the build ID in firmware
