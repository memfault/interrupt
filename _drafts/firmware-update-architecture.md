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

## DFU: High Level Architecture

Over the years, I've come up with a set of basic requirements most DFU systems
should fulfill. 

1. *DFU functionality should be isolated from code churn*. While we may want to
   update our firmware application regularly, those updates should not impact
our DFU code. Even a change as innocuous as changing the address of a global
variable used by DFU could trigger som subtle bugs.
2. *DFU code should itself be update-able*. Encryption keys may need
   to be rotated, flash layouts may need to be updated, and of course a bug in
the update logic itself may need to be fixed.
3. *A failed update should not brick the device*. Whether there is a bug in our
   DFU process, or a power loss event while we are writing firmware, the device
should be able to recover. It may operate in a degraded mode for a bit, but it
should at least be able to update itself back to a good state.
4. *As much code space as possible should be reserved for the application*.
   Firmware projects regularly run out of code space, so we should shy away from
architecture decisions that dedicate too much of it to the DFU process itself.

These requirements are not exhaustive, and specific applications will come with
their own requirements. Nevertheless, they are a good place to start.

Let's take these one by one and iteratively design our system. We start with a
simple fimrware 

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

### Separatin DFU from Application

<style>
.diag2 {
    max-width: 800px;
    margin-left: auto;
    margin-right: auto;
}
</style>
{% blockdiag size:120x40 %}
blockdiag {
    span_width = 100;
    // Set labels to nodes.
    A [label = "App Loader"];
    B [label = "Application"];
    A -> B [label = "Loads, Updates", fontsize=8];
}
{% endblockdiag %}{:.diag2}

### Updating DFU Code

<style>
.diag3 {
    max-width: 800px;
    margin-left: auto;
    margin-right: auto;
}
</style>
{% blockdiag size:120x40 %}
blockdiag {
    span_width = 100;
    // Set labels to nodes.
    A [label = "App Loader"];
    B [label = "Application"];
    A -> B [label = "Loads, Updates", fontsize=8];

    E [label = "Updater"];
    A -> E [label = "Loads", fontsize=8];
    E -> A [label = "Updates", fontsize=8];
}
{% endblockdiag %}{:.diag3}

### Saving code space

{% blockdiag size:120x40 %}
blockdiag {
    span_width = 100;
    // Set labels to nodes.
    A [label = "App Loader"];
    B [label = "Application"];
    A -> B [label = "Loads, Updates", fontsize=8];

    E [label = "Updater"];
    A -> E [label = "Loads", fontsize=8];
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

### Fail-proofing DFU

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
    A -> B [label = "Loads, Updates", fontsize=8];

    E [label = "Updater"];
    A -> E [label = "Loads", fontsize=8];
    E -> A [label = "Updates", fontsize=8];

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





