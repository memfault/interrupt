---
title: "Tools we use: installing GDB for ARM"
description: Demonstrate a few methods of installing gdb for ARM
author: noah
image: img/installing-gdb/cover.png # 1200x630
tags: [arm, gdb]
---

<!-- excerpt start -->

In this mini article, I'll be going on a few strategies and nuances around
getting a copy of GDB installed for debugging ARM chips.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Introduction

GDB, the GNU Project Debugger, is by default typically compiled to target the
same architecture as the host system. When we talk about "host" and "target
architectures, what we mean is:

1. the **host** architecture: where the GDB program itself is run
2. the **target** architecture: where the program being debugged is run

Debugging a program that's running on the same machine as the host might look
like this:

[![img/installing-gdb/gdb_host_target.drawio.svg](/img/installing-gdb/gdb_host_target.drawio.svg)](/img/installing-gdb/gdb_host_target.drawio.svg)

In embedded engineering, we often want to target a foreign architecture (eg, the
embedded device, connected to some debug probe). That setup might look like
this:

[![img/installing-gdb/gdb_cross_target.drawio.svg](/img/installing-gdb/gdb_cross_target.drawio.svg)](/img/installing-gdb/gdb_cross_target.drawio.svg)

We need a version of GDB that supports the target architecture of the program
being debugged.

In this article, we're interested in running GDB on an x86-64 or arm64 host and
debugging an arm-v7m (32-bit) target. Getting the right toolchain setup can be
tricky, especially if we want advanced features to be available. I'll go over
different approaches people use to download and setup GDB and explain the pros
and cons of each of them.

## Summary of strategies

I'm going to go through the various ways to install GDB with ARM support, but
first here's a table summarizing the approaches:

| Strategy                                                  | Binary or source install? | Pinned to version | Python support        | 3rd party package manager |
| --------------------------------------------------------- | ------------------------- | ----------------- | --------------------- | ------------------------- |
| [Binaries from ARM](#binaries-from-arm)                   | 📦binary                  | ✅ yes            | ⚠️ varies per version |                           |
| [xPack GNU Arm Embedded GCC](#xpack-gnu-arm-embedded-gcc) | 📦binary                  | ✅ yes            | ✅ yes                |                           |
| [System package manager](#system-package-manager)         | 📦binary                  | ⚠️ varies         | ⚠️ varies             |                           |
| [Conda](#conda)                                           | 📦binary                  | ✅ yes            | ✅ yes                | ⚠️ yes                    |
| [Docker](#docker)                                         | 📦binary                  | ✅ yes            | ✅ yes                | ⚠️ yes                    |
| [SDK-specific tools](#sdk-specific-tools)                 | 📦binary                  | ⚠️ varies         | ✅ (usually yes)      |                           |
| [Build from source](#build-from-source)                   | 📁source                  | ✅ yes            | ✅ (configurable)     |                           |
| [crosstool-NG](#crosstool-ng)                             | 📁source                  | ✅ yes            | ✅ (configurable)     |                           |

## Details on each strategy

### Binaries from ARM

ARM ships full prebuilt GCC + Binutils toolchains for Linux, Windows, and Mac.
These were originally hosted on launchpad.net, but were moved here around 2016:

<https://developer.arm.com/downloads/-/gnu-rm>

In 2022, the downloads relocated again to this page:

<https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/downloads>

These packages are very handy- they include the following components:

- GCC compiler
- all binutils
  - GDB
  - LD
  - objdump, etc
- prebuilt Newlib C library
- prebuilt "Newlib-Nano" reduced size C library
- various example linker scripts and startup files

Another benefit- you can have developers on Windows/Linux/Mac all using the same
version of the compiler (doesn't necessarily guarantee bit-for-bit reproducible
builds, but chances are pretty good).

The same tools can also easily be used in whatever CI system you are using.

Note that these packages are the officially supported toolchain from ARM itself.

There is one gotcha with these packages, though- the Python support is whatever
was configured at build time by ARM. This means if you're using [GDB's Python
API]({% post_url 2019-07-02-automate-debugging-with-gdb-python-api %}), you'll
need to make sure your Python scripts/dependencies/interpreter matches the
interpreter embedded into GDB.

And in the latest release, Python support was dropped in the Mac release:

```bash
# check which dynamic libraries the mac arm-none-eabi-gdb binary depends on

# v11.2-2022.02 (no Python support 😔. note the *gdb-py binary is absent from
# the archive)
❯ llvm-otool-14 -L ~/Downloads/gcc-arm-11.2-2022.02-darwin-x86_64-arm-none-eabi/bin/arm-none-eabi-gdb
/home/noah/Downloads/gcc-arm-11.2-2022.02-darwin-x86_64-arm-none-eabi/bin/arm-none-eabi-gdb:
        /usr/lib/libncurses.5.4.dylib (compatibility version 5.4.0, current version 5.4.0)
        /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1281.100.1)
        /usr/lib/libiconv.2.dylib (compatibility version 7.0.0, current version 7.0.0)
        /usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 902.1.0)

# v10.3-2021.10 (python2.7! in 2021!)
❯ llvm-otool-14 -L ~/Downloads/gcc-arm-none-eabi-10.3-2021.10-mac/bin/arm-none-eabi-gdb-py
/home/noah/Downloads/gcc-arm-none-eabi-10.3-2021.10-mac/bin/arm-none-eabi-gdb-py:
        /usr/lib/libncurses.5.4.dylib (compatibility version 5.4.0, current version 5.4.0)
        /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.250.1)
        /System/Library/Frameworks/Python.framework/Versions/2.7/Python (compatibility version 2.7.0, current version 2.7.10)
        /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation (compatibility version 150.0.0, current version 1575.17.0)
        /usr/lib/libiconv.2.dylib (compatibility version 7.0.0, current version 7.0.0)
        /usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.4)

# still present in the linux version of v11.2-2022.02 though. But it's a
# somewhat venerable python3.6, which I don't have installed right now 😅
❯ ldd ~/Downloads/gcc-arm-11.2-2022.02-x86_64-arm-none-eabi/bin/arm-none-eabi-gdb
        linux-vdso.so.1 (0x00007ffd3fd4d000)
        libncursesw.so.5 => /lib/x86_64-linux-gnu/libncursesw.so.5 (0x00007f9f6c717000)
        libtinfo.so.5 => /lib/x86_64-linux-gnu/libtinfo.so.5 (0x00007f9f6c6e8000)
        libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007f9f6c6e3000)
        libpython3.6m.so.1.0 => not found
        libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f9f6c6de000)
        libutil.so.1 => /lib/x86_64-linux-gnu/libutil.so.1 (0x00007f9f6c6d7000)
        libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f9f6c5f0000)
        libexpat.so.1 => /lib/x86_64-linux-gnu/libexpat.so.1 (0x00007f9f6c5bf000)
        libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007f9f6c393000)
        libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007f9f6c373000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f9f6c14b000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f9f6c764000)
```

This could potentially be a pretty serious problem if there is some tooling or
scripting that uses the Python API and the release for your host system doesn't
have Python enabled.

### xPack GNU Arm Embedded GCC

The [xPack Project](https://xpack.github.io/) provides a prebuilt gcc-arm
toolchain:

- <https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases/>

They tend to be pretty up-to-date, and more reliably include Python support.

There's also build documentation, if you'd like to tweak the package:

- <https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/blob/xpack/README-BUILD.md>

Overall pretty simple and easy to use, and make a very good alternative to the
official ARM GCC toolchains! I've used it as a drop-in replacement for the ARM
GCC toolchains, and it's working very well.

Advantages:

- wider host support (`darwin-arm64`) than the official releases
- more consistent Python support across host platform versions
- documented + reproducible build instructions

Disadvantages:

- only a single Python version supported per release
- similarly non-customizable as the official ARM GCC packages. _you get what you
  get and you don't get upset!_

### System package manager

Depending on your host platform (Linux, Mac, Windows, \*BSD, etc), you may or
may not have a built-in or defacto package manager. For example, on Linux, you
may have one of:

- `apt-get` / `apt` (Debian, Ubuntu)
- `pacman` (Arch)
- `yum` (Red Hat)

Mac users will often install [homebrew](https://brew.sh/), since there is no
built-in package manager.

Windows users may use something like [chocolately](https://chocolatey.org/) or
the recent
[winget](https://docs.microsoft.com/en-us/windows/package-manager/winget/) tool.

It's certainly convenient to be able to do something as simple as this to
acquire a version of `gdb` compatible with arm targets:

```bash
❯ sudo apt install gdb-multiarch
```

On Linux, those `gdb-multiarch` packages usually have a _lot_ of supported
targets. On my Ubuntu 22.04 machine, it's **214** supported targets!

(the `sed` one-liner is from [below](#build-from-source))

```bash
# using the one liner from below to list supported targets
❯ gdb-multiarch --batch -nx --ex 'set architecture' 2>&1 | tail -n 1 | sed 's/auto./\n/g' | sed 's/,/\n/g' | sed 's/Requires an argument. Valid arguments are/\n/g' | sed '/^[ ]*$/d' | sort | uniq | wc -l

214
```

Of course, it's only important that the targets we're interested in are
supported.

There are a few downsides to using a package manager to install the tools:

- you are at the mercy of the available packages (or you need to create and
  maintain a package, which is probably more difficult than a normal from-source
  build!)
- available versions could be pulled from registries for no reason, leaving your
  environment stranded
- bugs in package versions might be difficult to deal with, since packages may
  not be updated frequently
- package managers are usually host-specific (homebrew does support some Linux
  packages though!), which means if your development team is using different
  host machines, it may be difficult to standardize on one set of tools

### Conda

[Conda](https://conda.io/) is a cross-platform package manager, which we've
written about before on Interrupt:

- [Managing Developer Environments with
  Conda]({% post_url 2020-01-07-conda-developer-environments %})

It behaves quite similarly to system package managers, but can be used on
different hosts (Linux/Mac/Windows), so a development team can standardize
tooling without requiring a single host platform!

This is great! But there are a few downsides:

- same as with system package managers, we are relying on externally maintained
  tooling packages
- complex conda environments can get into dependency issues when updating
  packages
- not all packages will provide versions for all of Linux + Mac + Windows
- creating + maintaining conda packages can be pretty difficult, especially
  since cross-building conda packages is not yet fully supported

Plug (note: the author is an employee at Memfault), Memfault maintains conda
packages for multi-arch GDB:

- <https://anaconda.org/Memfault/multi-arch-gdb>
- <https://docs.memfault.com/docs/mcu/coredump-elf-with-gdb/#installing-multi-architecture-gdb-with-conda>

### Docker

[Docker](https://www.docker.com/) is a well-known technology for freezing
dependencies, by using virtualization.

For development tooling, you can set up a Docker image that contains all the
tooling you need to build + debug an embedded project:

```dockerfile
# select a specific SHA, to strictly pin the base image
FROM ubuntu:22.04@sha256:bace9fb0d5923a675c894d5c815da75ffe35e24970166a48a4460a48ae6e0d19

ARG DEBIAN_FRONTEND=noninteractive

# install GDB + GCC for ARM
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-arm-none-eabi \
    gdb-multiarch
```

This approach has a few advantages:

- anyone using the same Docker image should be using _exactly_ the same version
  of the tools
- the same Docker image can be used in your test / CI infrastructure, which
  simplifies things greatly
- Docker images can be used on Linux/Mac/Windows without too much host setup

And a few downsides:

- running Docker images on Mac or Windows is likely going to be lower
  performance than on Linux, especially for file IO, since on those platforms
  Docker containers are running in a virtualization environment
- accessing host data (files) from within the Docker container can be tricky
- accessing host hardware (USB or Serial ports) can be easy or impossible,
  depending on host platform
- maintaining the Docker image itself requires some knowledge of Docker (though
  there are a lot of good tutorials online)

Plug- Memfault also provides a Docker image for running gdb-multiarch:

- <https://docs.memfault.com/docs/mcu/coredump-elf-with-gdb/#multi-architecture-gdb-using-docker>

### SDK-specific tools

SDK Vendors (and IDE vendors too!) will often package copies of GDB, along with
other toolchain components, to make it easy to get started with their platform.

For example, Zephyr RTOS provides pre-built toolchains, that include the
appropriate GDB versions:

- <https://github.com/zephyrproject-rtos/sdk-ng/releases/tag/v0.14.2>

ESP-IDF from Espressif provides similar tool packages:

- <https://github.com/espressif/binutils-gdb/releases/esp-gdb-v11.2_20220715>

Advantages:

- these packages are often well-tested by the vendors, and you may even be able
  to get support if you encounter issues!
- generally support all Linux/Mac/Windows these days

Disadvantages:

- often these packages are very tailored to the particular environment (which
  usually is just fine), including custom patches for example
- Python support can be pretty variable- some packages won't have Python support
  at all, or it will only be for a specific version.
- external dependencies may not be provided (eg libpython), and you'll have to
  bring your own
- can be based on pretty old GDB/binutils releases, so there might be
  long-resolved bugs present, or missing newer optimizations + features

### Build from source

[Building a custom copy of GDB]({% post_url 2020-04-08-gnu-binutils %}) is one
way to enable support for the necessary architecture.

You might for example build GDB enabling all targets:

```bash
❯ ./configure
  --target="arm-elf-linux"
  --enable-targets=all
  --with-python
❯ make -j $(nproc)
```

This results in a nicely tuned copy of GDB- you can select only the features
needed, enable different optimizations, select the correct Python version to
integrate, etc., and are in full control of your software supply chain!

The downsides of from-source builds (in general):

- can be challenging to build for Windows or other non-Unix platforms
- need to maintain build tooling + binary repositories
- using custom builds, you might encounter edge cases or warts that have been
  solved in more widely used copies of the same software

> Quick note- if you want to see which architectures a given `gdb` binary
> supports, here's a shell "one-liner" that can be useful (taken from
> [here](https://sourceware.org/git/?p=binutils-gdb.git;a=blob;f=gdb/gdb_buildall.sh;hb=4a94e36819485cdbd50438f800d1e478156a4889#l206)):
>
> ```bash
> gdb --batch -nx --ex 'set architecture' 2>&1 | \
>   tail -n 1 | \
>   sed 's/auto./\n/g' | \
>   sed 's/,/\n/g' | \
>   sed 's/Requires an argument. Valid arguments are/\n/g' | \
>   sed '/^[ ]*$/d' | sort | uniq
> ```
>
> Warning, this won't work with the built-in `sed` on macos, it's too old and
> doesn't have the necessary GNU extensions.

### crosstool-NG

From the [website](https://crosstool-ng.github.io/):

> Crosstool-NG is a versatile (cross) toolchain generator. It supports many
> architectures and components and has a simple yet powerful menuconfig-style
> interface.

It's quite easy to get started with crosstool-ng, and there's a good number of
example configurations provided. A quickstart might run like this:

```bash
# 1. build crosstool-ng itself

# fetch the crosstool-ng repo
❯ git clone https://github.com/crosstool-ng/crosstool-ng
❯ cd crosstool-ng
❯ git checkout crosstool-ng-1.25.0

# build crosstool-ng
❯ ./bootstrap && ./configure --enable-local && make -j$(nproc)

# 2. now to build a toolchain

# use one of the example configs
❯ ./ct-ng arm-nano-eabi
# you can use './ct-ng menuconfig' to adjust the config, eg, add CT_DEBUG_GDB=y
# to enable building gdb

# build the toolchain. this can take a while- on my middle-range laptop, it
# took about 12 minutes
❯ ./ct-ng build

# now to test it (the toolchain output needs to be inserted to path)
❯ export PATH=$PWD/.build/arm-nano-eabi/buildtools/bin:$PATH
❯ export PATH=$PWD/.build/arm-nano-eabi/build/build-gdb-cross/gdb:$PATH
# confirm gdb is the freshly built one
❯ which gdb
/home/noah/dev/github/crosstool-ng/.build/arm-nano-eabi/build/build-gdb-cross/gdb/gdb
# run the check-architecture script from above
❯ gdb --batch -nx --ex 'set architecture' 2>&1 | tail -n 1 | sed 's/auto./\n/g' | sed 's/,/\n/g' | sed 's/Requires an argument. Valid arguments are/\n/g' | sed '/^[ ]*$/d' | sort | uniq

 arm
 arm_any
 armv2
 armv2a
 armv3
 armv3m
 armv4
 armv4t
 armv5
 armv5t
 armv5te
 armv5tej
 armv6
 armv6k
 armv6kz
 armv6-m
 armv6s-m
 armv6t2
 armv7
 armv7e-m
 armv8.1-m.main
 armv8-a
 armv8-m.base
 armv8-m.main
 armv8-r
 ep9312
 iwmmxt
 iwmmxt2
 xscale

# build an uninteresting program
❯ echo "int main(){return 0;}" > main.c
❯ arm-nano-eabi-gcc -mcpu=cortex-m0plus -ggdb3 -nostdlib -Wl,--entry=main -o arm.elf main.c
# check the elf it built with binutils
❯ gdb arm.elf -batch --ex 'disassemble /s main'
threads module disabled
Dump of assembler code for function main:
main.c:
1       int main(){return 0;}
   0x00008000 <+0>:     push    {r7, lr}
   0x00008002 <+2>:     add     r7, sp, #0
   0x00008004 <+4>:     movs    r3, #0
   0x00008006 <+6>:     movs    r0, r3
   0x00008008 <+8>:     mov     sp, r7
   0x0000800a <+10>:    pop     {r7, pc}
End of assembler dump.

❯ arm-nano-eabi-objdump --disassemble=main --source arm.elf

arm.elf:     file format elf32-littlearm


Disassembly of section .text:

00008000 <main>:
int main(){return 0;}
    8000:       b580            push    {r7, lr}
    8002:       af00            add     r7, sp, #0
    8004:       2300            movs    r3, #0
    8006:       0018            movs    r0, r3
    8008:       46bd            mov     sp, r7
    800a:       bd80            pop     {r7, pc}

❯ arm-nano-eabi-size -Ax arm.elf
arm.elf  :
section             size      addr
.text                0xc    0x8000
.persistent          0x0   0x1800c
.noinit              0x0   0x1800c
.comment            0x22       0x0
.debug_aranges      0x20       0x0
.debug_info         0x48       0x0
.debug_abbrev       0x3a       0x0
.debug_line         0x3c       0x0
.debug_frame        0x2c       0x0
.debug_str        0x2c12       0x0
.debug_macro       0xa7d       0x0
.ARM.attributes     0x2c       0x0
Total             0x37f3
```

You end up with a fully-baked toolchain set up to your specifications, without a
ton of work manually configuring and building various packages.

The Zephyr project currently bases its toolchain builds on crosstool-NG, with
some custom sources:

<https://github.com/zephyrproject-rtos/sdk-ng/tree/61bd806b61e9fa92a90c8e1d81237353556a0560>

And the famous Compiler Explorer project also uses crosstool-NG for cross
toolchain building:

<https://github.com/compiler-explorer/gcc-cross-builder/blob/190e5ba158c42529090fa8ea5f33a4f79b6e179b/Dockerfile#L77>

## Outro

I learned a lot wading through the different varieties of GDB floating around! I
was particularly surprised at how much Python support varied across all these
options. That's certainly a spot that's caused me headaches in the past.

Overall I think the choice of toolchain/GDB depends on these criteria:

- does your SDK/platform already provide a nicely working copy?
- do you have specific needs that are not met by existing options?

The biggest difficulty overall seems to be sourcing a copy with

1. the version of Python your tools are relying on
2. supports Linux/Mac/Windows (depending on what developers on your team are
   using)

Though it might seem like an option of last resort, I was pleasantly surprised
at how straightforward it was to build GDB from source, and `crosstool-NG` gets
a special shoutout for really simplifying building a complete cross toolchain
🙌!

The great thing about these tools being open source: we have lots of options!

_(also could be considered a downfall 😅)_

Thanks so much for reading!

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->
