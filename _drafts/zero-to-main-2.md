---
title: "From zero to main(): Memory Layout"
author: francois
---

<!-- excerpt start -->
[Last time](https://interrupt.memfault.com/blog/zero-to-main-1) we talked about
bootstrapping a C environment on an MCU before invoking our `main` function. One
thing we took for granted was the fact that functions and data end up in the
right place in our binary. Today, we're going to dig into how that happens by
learning about memory regions and linker scripts.
<!-- excerpt end -->

You may remember the following things happening auto-magically:
1. We used variables like `&_ebss`, `&_sdata`, ...etc. to know where each of our
   section was placed in flash, and where some needed to go in RAM.
2. A pointer to our `ResetHandler` was found at address 0x00000004 for the MCU
   to find.

While these things are true for many projects, they are held together at best by
convention, at worst by generations of copy/paste engineering. You'll find some
MCUs have different memory maps, some startup scripts name those variables
differently, and some programs have more or less segments.

Since they are not standardized, those things need to be specified somewhere in
our project. In the case of projects linked with a Unix-`ld` like, that
somewhere is the linker script.

### A brief primer on linking

Linking is the last stage in compiling a program. It takes a number of compiled
object files and merges them into a single program, filling in addresses so
that everything is in the right place.

Prior to linking, the compiler will have taken your source files one by one and compiled
them into machine code. In the process, it leaves placeholders for
addresses as (1) it does not know where the code will end up within the broader
structure of the program and (2) it knows nothing about symbols outside of the
current file or compilation unit.

The linker takes all of those object files, and merges them together along with
external dependencies like the C Standard Library into your program. To figure
out what bits go where, the linker relies on a linker script - a blueprint for
your program. Lastly, all placeholders are replaced by addresses.

The linker often does a bit more than that. For example, it can generate debug
information, garbage collect unused sections of code, or run whole-program
optimization (also known as Link-Time Optimization, or LTO). For the sake of
this conversation, those activities are less important.

For more information on the linker, there's a great thread on [Stack
Overflow](https://stackoverflow.com/questions/3322911/what-do-linkers-do).

### Anatomy of a linker script

The linker script contains four things:
* Memory layout: what memory is available where
* Section defitions: what part of a program should go where
* Options: commands to specify architecture, entry point, ...etc. if needed
* Symbols: variables to inject into the program at link time

### Memory Layout

In order to allocate program space, the linker needs to know how much memory is
available, and at what addresses that memory exists. This is what the `MEMORY`
section of the linker script is for.

The syntax for the memory section is defined in the [binutils
docs](https://sourceware.org/binutils/docs/ld/index.html#SEC_Contents) and is as follow:

```
MEMORY
  {
    name [(attr)] : ORIGIN = origin, LENGTH = len
    …
  }
```

Where
* `name` is a name you want to use for this section. Names do not carry meaning,
  so you're free to use anything you want. You'll often find "flash", and "ram"
as section names.
* `(attr)` are optional attributes for the region, like whether it's writable
   (`w`), readable (`r`), or executable (`x`). Flash memory is usually `(rx)`,
while ram is `rwx`. Marking a region as non-writable does not magically make it
write protected: these attributes are meant to describe the properties of the
memory, not set it.
* `origin` is the start address of the memory region.
* `len` is the size of the memory region, in bytes.

The memory map for the SAMD21G18 chip we've got on our board can be found in its
[datasheet](http://ww1.microchip.com/downloads/en/DeviceDoc/SAMD21-Family-DataSheet-DS40001882D.pdf)
in table 10-1, reproduced below.

<table align="center" border="0" summary="SAMD21G18 Memory map">
<caption>SAMD21G18 Memory Map</caption>
    <thead xmlns="http://www.w3.org/1999/xhtml">
        <tr><th>Memory</th><th>Start Address</th><th>Size</th></tr>
    </thead>
    <tbody xmlns="http://www.w3.org/1999/xhtml">
        <tr>
            <td>Internal Flash</td>
            <td>0x00000000</td>
            <td>256 Kbytes</td>
        </tr>
        <tr>
            <td>Internal SRAM</td>
            <td>0x20000000</td>
            <td>32 Kbytes</td>
        </tr>
    </tbody>
</table>
<br/>
Transcribed into a `MEMORY` section, this gives us:

```
MEMORY
{
  rom      (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00040000
  ram      (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00008000
}
```

### Section Definitions

Code and data are bucketed into sections, which are contiguous areas of memory.
There are no hard rules about how many sections you should have, or what they
should be, but you typically want to put symbols in the same section if:
1. They should be in the same region of memory, or
2. They need to be initialized together.

In our previous post, we learned about two types of symbols that are initialized
in bulk:
1. Initialized static variables which must be copied from flash
2. Uninitialized static variables which must be zeroed.

Our linker script concerns itself with two more things:
1. Code and constant data, which can live in read only memory (e.g. flash)
2. Reserved sections of ram, like a stack or a heap

By convention, we name those sections as follow:
1. `.text` for code & constants
2. `.bss` for unintialized data
3. `.data` for initialized data
4. `.stack` for our stack

The [elf spec](http://refspecs.linuxbase.org/elf/elf.pdf) holds a full list.
Your firmware will work just fine if you call them anything else, but your
colleagues may be confused and some tools may fail in odd ways. The only
constraint is that you may not call your section `/DISCARD/`, which is a
reserved keyword.

The linker script syntax to define those sections is as follow:

```
section [address] [NOLOAD] :
  [AT(lma)]
  [ALIGN(section_align)]
  {
    output-section-command
    output-section-command
    …
  } [>region] [AT>lma_region]
```

Note that I've removed rarely-used options, you can find the full details in the
[ld
manual](https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_Linux/4/html/Using_ld_the_GNU_Linker/sections.html#OUTPUT-SECTION-DESCRIPTION).

Let's go over the syntax above briefly:
* `section` is the name you want to give to your section. lists a set of common section
names which may hold special meaning for your operating system. On embedded, you
may use whichever names you would like.
* `address` (optional) the virtual address for this section.
* `NOLOAD` (optional) marks a section as not loaded, as in no data needs to be
  copied to that memory region to load the program. This is used for memory
regions you want to reserve such as a stack.
* `AT(...)` what address the section needs to be loaded to. We discuss this in
  details below.
* `ALIGN` the memory alignment, in bytes, of the section. ARM requires stack
  memory to be 8-byte aligned for example, this is where you specify this.
* `>region` specifies which of the regions from the `MEMORY` definition this
  section should be added to. This sets the VMA.
* `AT>lma_region` similarly, region from the `MEMORY` definition this section
  should be *loaded* to.

**A note on LMA and VMA**

Every section has two addresses, its *load* address (LMA) and its *virtual*
address (VMA). In a firmware context, the LMA is where your JTAG loader needs to
place the section and the VMA is where the section is found during execution.

You can think of the LMA as the address "at rest" and the VMA the address during
execution, when the device is on the and the program is running.

How can the section be at a different address at execution than at load? There
are a few options, but the most common by far is code or data in RAM. Since RAM
isn't persisted while power is off, those sections need to be loaded in flash.
At boot, the `Reset_Handler` copies the data from its LMA in flash to its VMA
in RAM before your `main` function is called (we covered this in our previous article).

So with that in mind, let's write our sections!

First, the text section. This section contains the code





### Relocation



