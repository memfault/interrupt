---
title: "Code Size Optimization: GCC Compiler Flags"
description: "A walk through of GCC configuration options and flags to optimize firmware code size"
author: francois
tags: [fw-code-size]
---

## Introduction

This is the second post in our [Firmware Code Size Optimization]({% tag_url fw-code-size %}) series. [Last time]({% post_url
2019-06-06-best-firmware-size-tools %}), we talked about measuring code size as a
precondition to improving it. Now we’ll leap from planning into action!

<!-- excerpt start -->
In this post, we will review compiler options that we can
use to reduce firmware code size. We will focus on `arm-none-eabi-gcc`, the GCC
compiler used for ARM-based microcontrollers, though most of the compile-time
flags we will cover are available in other GCC flavors as well as in Clang.
<!-- excerpt end -->

You would think that a single flag could be used for “make this as small as
possible”, but unfortunately it isn’t so. Instead, code size optimization
involves complicated trade-offs that must be considered on a case by case basis.

{% include newsletter.html %}

{% include toc.html %}

## Setting the stage

In order to get measurable improvements in code size, we
need a reasonably large code-base to begin with. To that end, we’ll use example
code from ChibiOS[^1], a free and open
source RTOS. Specifically, we’ll use their FatFS+USB example for STM32F1 MCUs.

You can reproduce our setup simply by cloning the project and extracting
libraries:

```
$ git clone https://github.com/ChibiOS/ChibiOS.git
[...]
$ cd ChibiOS/ext
$ 7za x fatfs-0.13c_patched.7z
[...]
$ cd ../demos/STM32/RT-STM32F103-STM3210E_EVAL-FATFS-USB
```

To get our baseline, let’s disable all optimizations and build the project:

```
$ make USE_LINK_GC=no USE_LTO=no
[...]
Linking build/ch.elf
Creating build/ch.hex
Creating build/ch.bin
Creating build/ch.dmp

   text	   data	    bss	    dec	    hex	filename
  72824	    232	  98072	 171128	  29c78	build/ch.elf
Creating build/ch.list
```

Which means we are starting with a firmware size of about 71KiB.

## Changing the optimization level

Compilers have the difficult task of optimizing a program around multiple axes.
Namely:
1. Program performance - the speed at which the program executes
2. Compilation time - the time it takes to compile the program
3. Code size - the size of the compiled program
4. Debug-ability - how easily a program can be debugged

You would be hard pressed to choose trade-offs that work for every application: a
math library may care about performance, while firmware code may care about code
size. To that end, GCC bundles its optimizations into buckets which can be
selected via the Optimize
Options[^2].

The linked documentation does a great job of explaining the options, but I’ll go
over them briefly here:
* `-O0` : optimize for compile time and debug-ability. Most performance
  optimizations are disabled. This is the default for GCC.
* `-O1`: pick the low hanging fruit in terms of performance, without impacting
  compilation time too much.
* `-O2`: optimize for performance more aggressively, but not at the cost of
  larger code size.
* `-O3`: optimize for performance at all cost, no matter the code size or
  compilation time impact.
* `-Ofast` a.k.a. ludicrous mode: take off the guard rails, and disregard strict
  C standard compliance in the name of speed. This may lead to invalid programs.
* `-Os` optimize for code size. This uses `O2` as the baseline, but disables
  some optimizations. For example, it will not inline[^3] code if that leads to
a size increase.


Compiled with GCC’s default (`-O0`), our example clocks in at a whooping 108KiB
of code:

```
Linking build/ch.elf
Creating build/ch.hex
Creating build/ch.bin
Creating build/ch.dmp

   text	   data	    bss	    dec	    hex	filename
 111072	    296	  98008	 209376	  331e0	build/ch.elf
Creating build/ch.list
```

Our baseline in the previous section uses ChibiOS’s default of `-O2`. You’ll
have guessed by now that we can do better by using `-Os`:

```
Linking build/ch.elf
Creating build/ch.hex
Creating build/ch.bin
Creating build/ch.dmp

   text	   data	    bss	    dec	    hex	filename
  62956	    232	  98072	 161260	  275ec	build/ch.elf
Creating build/ch.list
```

61KiB! This is 47KiB smaller than the default GCC settings, and 10KiB smaller
than the default project settings.

### What if my program is performance sensitive?

Parts of your program may be too performance-sensitive to compile them with
`-Os`. At Pebble, this was the case for our text layout code, which was critical
to hitting our target frame rate when scrolling through a notification. This is
generally only true for a few very specific functions or files.

GCC offers two methods to selectively tweak the optimization level: at the file
level, or at the function level.

If you want to set optimization level for a single function, you can use the
`optimize` function attribute:

```c
void __attribute__((optimize("O3"))) fast_function(void) {
    // ...
}
```

For a whole file, you can use the `optimize` pragma:
```c
#pragma GCC push_options
#pragma GCC optimize ("O3")

/*
 * Code that needs optimizing
 */

#pragma GCC pop_options
```

Note that the `push_options` and `pop_options` pragmas are needed to make sure
the rest of the file is compiled with the regular options.

## Linker Garbage Collection

Now that we’ve set the appropriate optimization level, we turn our attention to
dead code elimination. Most programs contain dead code: it may come from
libraries you use partially, or test functions you left in the final program.

Since the compiler operates on one file at a time, it does not have enough
context to decide whether a function is dead code or not. Consider a library
which exposes a function to add numbers in an array: while nothing in the
library itself may call this function, the compiler has no way of knowing if
other files are using it.

The linker on the other hand has visibility into our whole program, so this is
the stage where dead code could be identified and removed. Unfortunately,
linkers do not perform optimizations by default.

To enable dead code optimization on GCC, you need two things: the compiler needs
to split each function into its own linker section so the linker knows where
each function is, and the linker needs to add an optimization pass to remove
sections that are not called by anything.

This is achieved with the `-ffunction-sections`  compile-time flag and the
`-gc-sections` link-time flag. A similar process can take place with dead data
and the `-fdata-sections` flag.

In a Makefile, it looks like this:
```makefile
CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections
```

ChibiOS sets those when the `USE_LINK_GC` variable is set, so we call:
```bash
$ make USE_LINK_GC=yes USE_LTO=no
```

And get:
```bash
Linking build/ch.elf
Creating build/ch.hex
Creating build/ch.bin
Creating build/ch.dmp

   text	   data	    bss	    dec	    hex	filename
  54828	    228	  98072	 153128	  25628	build/ch.elf
Creating build/ch.list
```

So dead code elimination yields close to another 10KiB!

### Preventing some symbols from being garbage collected

In some cases, you may want some code that is not explicitly called by anything
else to remain in your program. One such example is interrupt handlers: while it
may look to the linker like they are not called, the hardware will jump to those
addresses and expect the code to be there.

The linker provides the `KEEP` command to identify sections that should be kept.
For interrupts, the typical solution is to put your vector table in a section
called `vectors` and mark it in the linker script:

```
    .text :
    {
        KEEP(*(.vectors .vectors.*))
  	      [...]
    } > rom
```


## Link-time optimization

While linkers do not traditionally do much optimizing, this has started to
change. Nowadays, all the major compilers offer Link Time Optimization or LTO.
LTO enables optimizations that are not available to the compiler as they touch
multiple compilation units at once.

LTO comes with a few drawbacks:
1. compilation time - even for simple programs the difference in build time will
   be noticeable, some of your colleagues may complain about it!
2. debug-ability - Newer versions of debuggers like GDB and LLDB do a much
   better job of coping with LTO however, and while there is still an impact it is not dramatic.
3. higher stack usage - more aggressive inlining can use more stack memory and
   lead to overflows.

Enabling LTO is as simple as passing the `-flto` flag to both the compilation
and link steps:

```makefile
CFLAGS += -flto
LDFLAGS += -flto
```

ChibiOS once again has a build variable for it (`USE_LTO`), so we run:
```
$ make USE_LINK_GC=yes USE_LTO=yes
```

And get the following output:
```
Linking build/ch.elf
Creating build/ch.hex
Creating build/ch.bin
Creating build/ch.dmp

   text	   data	    bss	    dec	    hex	filename
  51960	    228	  98072	 150260	  24af4	build/ch.elf
Creating build/ch.list
```

To get an idea of the difference in compile time, we can use the `time` command
on OSX:

Before:
```
$ time make USE_LINK_GC=yes USE_LTO=no
Linking build/ch.elf
Creating build/ch.hex
Creating build/ch.bin
Creating build/ch.dmp

   text	   data	    bss	    dec	    hex	filename
  54828	    228	  98072	 153128	  25628	build/ch.elf
Creating build/ch.list

Done

real	0m27.462s
user	0m23.545s
sys	0m2.752s
```

After:
```
$ time make USE_LINK_GC=yes USE_LTO=yes
[...]
Linking build/ch.elf
Creating build/ch.hex
Creating build/ch.bin
Creating build/ch.dmp

   text	   data	    bss	    dec	    hex	filename
  51960	    228	  98072	 150260	  24af4	build/ch.elf
Creating build/ch.list

Done

real	0m32.865s
user	0m26.861s
sys	0m3.386s
```

So we traded 5 seconds of build time for ~3KiB of code space.

> Note: /u/xenoamor on Reddit[^7] pointed out that in some cases, LTO is dropping
> vector tables even when the KEEP command is used. You can find more details,
> as well as a workaround on Launchpad[^8].

## C Library

The C library often times takes a non trivial amount of code space. Looking at
our example program, we can see multiple libc symbols in list of the 30 largest
symbols:

```
$ arm-none-eabi-nm --print-size --size-sort --radix=d build/ch.elf | tail -30
[...]
134218584 00000442 T strcmp
[...]
134235240 00000664 T chvprintf.
[...]
```

Those two symbols alone represent 1KiB of code space!

By default, `arm-none-eabi-gcc` ships with newlib[^4], a libc implementation
optimized for embedded systems. While newlib is quite small, ARM added a slimmed
down version of newlib dubbed “newlib-nano”, which does away with some C99
features and rewrites part of the library directly in thumb code to shrink its
size. You can read more about newlib nano on ARM’s website[^6].

Libc selection in GCC is implemented with specs files. I won’t go into too many
details, but the easiest way to think about specs files is that they are CFLAGS
and LDFLAGS which are conveniently bundled into a single configuration. You can
find the spec file for newlib-nano on
GitHub[^5].

In order to use newlib nano, we add the spec file to our flags:
```
CFLAGS += --specs=nano.specs
LDFLAGS += --specs=nano.specs
```

Recompiling our example with newlib-nano gives us:
```
Linking build/ch.elf
Creating build/ch.hex
Creating build/ch.bin
Creating build/ch.dmp

   text	   data	    bss	    dec	    hex	filename
  50450	    228	  98072	 148750	  2450e	build/ch.elf
Creating build/ch.list
```

Almost 2KiB saved!

## Closing

We hope reading this post has given you new ways to optimize the size of your
firmware.

One overarching lesson is that default compiler settings are inadequate in most
cases. The default settings had our applications clocking in at over 108KiB. A
few compiler flag changes cut it down by more than half!

Future posts in this series will consider coding style, and even some desperate
hacks one can use to slim down code size further.

{% include newsletter.html %}

## Reference & Links

[^1]: [ChibiOS Website](http://chibios.org/dokuwiki/doku.php)
[^2]: [GCC Optimize Options](https://gcc.gnu.org/onlinedocs/gcc-12.2.0/gcc/Optimize-Options.html)
[^3]: [Inline Expansion](https://en.wikipedia.org/wiki/Inline_expansion)
[^4]: [Newlib Home](https://sourceware.org/newlib/)
[^5]: [Newlib on Github](https://github.com/littlekernel/newlib/blob/master/libgloss/arm/elf-nano.specs)
[^6]: [ARM GCC 4.7 Announcement](https://community.arm.com/developer/ip-products/system/b/embedded-blog/posts/shrink-your-mcu-code-size-with-gcc-arm-embedded-4-7)
[^7]: [Reddit comment about LTO bug](https://www.reddit.com/r/embedded/comments/ctjjg3/code_size_optimization_gcc_compiler_flags/exlal31/)
[^8]: [Launchpad: LTO Bug](https://bugs.launchpad.net/gcc-arm-embedded/+bug/1747966)
