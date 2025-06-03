---
title: Practical Zephyr - Devicetree basics (Part 3)
description: Article series "Practical Zephyr", the third part covering Devicetree basics.
author: lampacher
tags: [zephyr, devicetree, practical-zephyr-series]
---

In the [previous article]({% post_url 2024-01-24-practical_zephyr_kconfig %}), we configured _software_ using the [kernel configuration tool Kconfig](https://docs.zephyrproject.org/latest/build/kconfig/index.html#configuration-system-kconfig), and we've silently assumed that there's a UART interface on our board that is configurable and used for logging.

<!-- excerpt start -->

In this third article of the "Practical Zephyr" series, we'll see how we configure and use _hardware_. For this, Zephyr borrows another tool from the Linux kernel: [**Devicetree**](https://docs.zephyrproject.org/latest/build/dts/index.html).

In contrast to _Kconfig_, the *Devicetree* syntax and its use are more intricate. Therefore, we'll cover *Devicetree* in two articles. In this article, we'll see what Devicetree is and how we can write our own Devicetree source files. In the next article, we'll look at so-called Devicetree _bindings_, which add semantics to our Devicetree. Be prepared for a fair bit of theory, but as usual, we'll use an example project to follow along.


<!-- excerpt end -->

👉 Find the other parts of the *Practical Zephyr* series [here](/tags#practical-zephyr-series).

> **Disclaimer:** This article covers Devicetree in _detail_ and from first principles. If you're already familiar with Devicetree or don't want to know how it works in detail, this article may not be for you. Instead, consider tuning in for the next article!

> 🎬
> [Listen to Martin on Interrupt Live](https://www.youtube.com/watch?v=ls_Y45WsTiA?feature=share)
> talk about the content and motivations behind writing this series.

{% include newsletter.html %}

{% include toc.html %}

## Prerequisites

This article is part of an article _series_. If you haven't read the previous articles, please have a look. The [first article]({% post_url 2024-01-10-practical_zephyr_basics %}) explains the required installation steps, and we'll follow along with our running UART example from the [previous article]({% post_url 2024-01-24-practical_zephyr_kconfig %}). If you're already familiar with Zephyr and have a working installation at hand, it should be easy enough for you to follow along with your own setup without reading the previous articles.

> **Note:** A full example application including all the Devicetree source files that we'll see throughout this article is available in the [`02_devicetree_basics` folder of the accompanying GitHub repository](https://github.com/lmapii/practical-zephyr/tree/main/02_devicetree_basics).

We'll again be using the [development kit for the nRF52840](https://www.nordicsemi.com/Products/Development-hardware/nrf52840-dk) as a reference and explore its files in Zephyr, but you can follow along with any target - even virtual ones.

A short heads-up: We will _not_ define our own custom _board_, and we will _not_ describe memory layouts. Those are more advanced topics and go beyond this series. With this article, we want to have a detailed look and familiarize ourselves with Devicetree and the syntax of its input files.


## What's a _Devicetree_?

Let's first deal with the terminology: In simple words, the _Devicetree_ is a tree data structure you provide to describe your hardware. Each _node_ describes one _device_, e.g., the UART peripheral that we used for logging via `printk` in the [previous article]({% post_url 2024-01-24-practical_zephyr_kconfig %}). Except for the root note, each node has exactly one parent, thus the term device*tree*.

Devicetree files use their own _Devicetree Source (DTS) format_, defined in the [official Devicetree specification](https://www.devicetree.org/specifications/). For certain file types (bindings) used by _Devicetree_, Zephyr uses yet another file format - but fear not, it simply replaces the _DTS format_ with simple `.yaml` files. We'll cover _bindings_ in the next article, so we'll focus on Devicetree source files for now. There are also some subtle differences between the official Devicetree specification and the way it is used in Zephyr, but we'll touch up on that throughout this article.

The build system takes the _Devicetree_ specification and feeds it to its own compiler, which - in the case of Zephyr - generates `C` macros that are, in turn, used by Zephyr's device drivers. For all the readers coming straight from Linux - yes, this approach is a little different than what you're used to, but we'll get to that.

The following is a snippet of [Nordic's](https://www.nordicsemi.com/) _Devicetree Source Include_ file of their nRF52840 SoC:

`zephyr/dts/arm/nordic/nrf52840.dtsi`

```dts
#include <arm/armv7-m.dtsi>
#include "nrf_common.dtsi"

/ {
  soc {
    uart0: uart@40002000 {
      compatible = "nordic,nrf-uarte";
      reg = <0x40002000 0x1000>;
      interrupts = <2 NRF_DEFAULT_IRQ_PRIORITY>;
      status = "disabled";
    };
  };
};
```

One could compare the Devicetree to something like a `struct` in `C` or a `JSON` object. Each node (or object) lists its properties and their values and thus describes the associated device and its configuration. From the above snippet, we can see that there's a _system on chip (soc)_ node that contains a UART instance with some configuration data.

Personally, I felt the details of the [Devicetree specification](https://www.devicetree.org/specifications/) or [Zephyr's official documentation on _Devicetree_](https://docs.zephyrproject.org/latest/build/dts/index.html) a bit overwhelming, and I could hardly keep all the information in my head when reading straight through it (getting lost following links again), so in this article, I'm choosing a slightly different approach:

Instead of going into great detail about the _DTS (Devicetree Source) format_, we'll start with a simple project, build it, and dive straight into the input and output files used or generated by the build process. Based on those files, one by one, we'll try and figure out how this whole thing works. Finally, we'll also create our own nodes, but we won't fully specify the file format.

In case you're looking for a more detailed description of the _DTS (Devicetree Source) format_, other authors provided much more detailed descriptions - much better than anything I could possibly come up with. Here are some of my favorites:

- My obvious first choice is the [official Devicetree specification](https://www.devicetree.org/specifications/). Just keep in mind that Zephyr made some slight adjustments.
- Second, when it comes to details [Zephyr's official documentation on Devicetree](https://docs.zephyrproject.org/latest/build/dts/index.html) is very hard to beat.
- Zephyr's official documentation also includes a more practical information article in the form of a [Devicetree how-tos](https://docs.zephyrproject.org/latest/build/dts/index.html).
- Finally, the official Raspberry PI documentation also has a [great section about Devicetree and the DTS syntax](https://www.raspberrypi.com/documentation/computers/configuration.html#device-trees-overlays-and-parameters).



## Devicetree and Zephyr

We create a new freestanding application with the files listed below to get started. If you're not familiar with the required files, the installation, or the build process, have a look at the previous articles or the official documentation. As mentioned in the [prerequisites](#prerequisites), you should be familiar with creating, building, and running a Zephr application.

```bash
$ tree --charset=utf-8 --dirsfirst
.
├── src
│   └── main.c
├── CMakeLists.txt
└── prj.conf
```

The `prj.conf` can remain empty for now, and the `CMakeLists.txt` only includes the necessary boilerplate to create a Zephyr application with a single `main.c` source file. As an application, we'll use the same old `main` function that outputs the string _"Message in a bottle."_ each time it is called, and thus each time the device starts.

```c
#include <zephyr/kernel.h>
#define SLEEP_TIME_MS 100U

void main(void)
{
    printk("Message in a bottle.\n");
    while (1)
    {
        k_msleep(SLEEP_TIME_MS);
    }
}
```

As usual, we can now build this application for the development kit of choice, in my case, the [nRF52840 Development Kit from Nordic](https://www.nordicsemi.com/).

```bash
$ west build --board nrf52840dk_nrf52840 --build-dir ../build
```

But wait, we didn't "do anything Devicetree" yet, did we? That's right, someone else already did it for us! After our first look into Zephyr and exploring Kconfig, we're familiar with the build output of our Zephyr application, so let's have a look! You should find a similar output right at the start of your build:

```
-- Found Dtc: /opt/nordic/ncs/toolchains/4ef6631da0/bin/dtc (found suitable version "1.6.1", minimum required is "1.4.6")
-- Found BOARD.dts: /opt/nordic/ncs/v2.4.0/zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts
-- Generated zephyr.dts: /path/to/build/zephyr/zephyr.dts
-- Generated devicetree_generated.h: /path/to/build/zephyr/include/generated/devicetree_generated.h
-- Including generated dts.cmake file: /path/to/build/zephyr/dts.cmake
```

### CMake integration

The build output shows us that Devicetree - just like Kconfig - is a central part of any application build. It is added to your build via the CMake module `zephyr/cmake/modules/dts.cmake`. The first thing that Zephyr is looking for, is the _Devicetree compiler_ `dtc` (see, `zephyr/cmake/modules/FindDtc.cmake`). Typically, this tool is in your `$PATH` after installing Zephyr or loading your Zephyr environment:

```bash
$ where dtc
/opt/nordic/ncs/toolchains/4ef6631da0/bin/dtc
```

Since we didn't specify any so-called _Devicetree overlay file_ (bear with me for now, we'll see how to modify our Devicetree using _overlay_ files in a later section), Zephyr then looks for a Devicetree source file that matches the specified _board_. In my case, that's the nRF52840 development kit, which is supported in the current Zephyr version: The board and thus its Devicetree is fully described by the file `zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts`.

> **Note:** If you're using a custom board not supported by Zephyr, you'd have to provide your own DTS file for it. We won't go into details about adding support for a custom board in this article (and maybe the next one), but at the end of it, you should have all the knowledge to do that - or to understand any other article showing you how to do it.

Let's not dive into the file's contents for now; instead, look at the build. Somehow, the build ends up with a generated `zephyr.dts` file. A couple of interesting things are happening in this process that are worth mentioning, so we'll have a closer look.

> **Note:** In this section, we're really just brushing over the files to get a bit of a feeling for the syntax and how nodes are used. We'll cover all of this in great detail still!


### Devicetree includes and sources in Zephyr

If you're familiar with the [Devicetree specification](https://www.devicetree.org/specifications/), you might have wondered why the Devicetree snippet that we've seen before uses `C/C++` style `#include` directives:

`zephyr/dts/arm/nordic/nrf52840.dtsi`

```c
#include <arm/armv7-m.dtsi>
#include "nrf_common.dtsi"
```

The [Devicetree specification](https://www.devicetree.org/specifications/) introduces a dedicated `/include/` compiler directive to include other sources in a DTS file. E.g., our include for `nrf_common.dtsi` should have the following format:

```dts
/include/ "nrf_common.dtsi"
```

The reason for this discrepancy is that Zephyr uses the `C/C++` preprocessor to resolve includes - and for resolving actual `C` macros that are used within DTS files. This happens in the call to the CMake function `zephyr_dt_preprocess` in the mentioned Devicetree CMake module.

Let's have a look at the "include" tree of the Devicetree source file of the nRF52840 development kit used in my build:

```
nrf52840dk_nrf52840.dts
├── nordic/nrf52840_qiaa.dtsi
│   ├── mem.h
│   └── nordic/nrf52840.dtsi
│       ├── arm/armv7-m.dtsi
│       │   └── skeleton.dtsi
│       └── nrf_common.dtsi
│           ├── zephyr/dt-bindings/adc/adc.h
│           ├── zephyr/dt-bindings/adc/nrf-adc.h
│           │   └── zephyr/dt-bindings/dt-util.h
│           │       └── zephyr/sys/util_macro.h
│           │           └── zephyr/sys/util_internal.h
│           │               └── util_loops.h
│           ├── zephyr/dt-bindings/gpio/gpio.h
│           ├── zephyr/dt-bindings/i2c/i2c.h
│           ├── zephyr/dt-bindings/pinctrl/nrf-pinctrl.h
│           ├── zephyr/dt-bindings/pwm/pwm.h
│           ├── freq.h
│           └── arm/nordic/override.dtsi
└── nrf52840dk_nrf52840-pinctrl.dtsi
```

> **Note:** You might have noticed the file extensions `.dts` and `.dtsi`. Both file extensions are Devicetree source files, but by convention the `.dtsi` extension is used for DTS files that are intended to be _included_ by other files.

That "include" tree makes a lot of sense, doesn't it?
- Since we didn't specify anything external in an overlay file, our outermost Devicetree source file is our _board_ `nrf52840dk_nrf52840.dts`.
- Our board uses the nRF52840 QIAA microcontroller from Nordic, which is described in its own Devicetree include source file `nrf52840_qiaa.dtsi`.
- That MCU is, in turn, a variant of the nRF52840, and therefore includes `nrf52840.dtsi`.
- The nRF52840 is an ARMv7 core, which is described in `armv7-m.dtsi`.
- In addition, the nRF52840 uses Nordic's peripherals, specified in `nrf_common.dtsi`.

The included `C/C++` header files `.h` are just a "bonus" since Zephyr uses the preprocessor and we're therefore allowed to use `C/C++` macros within Devicetree source files: All the macros will be replaced by their values before the actual compilation process. E.g., the macro `NRF_DEFAULT_IRQ_PRIORITY` in `interrupts = <2 NRF_DEFAULT_IRQ_PRIORITY>;` that we've seen before is expanded and thus replaced by its actual value before the Devicetree is compiled.

This type of include graph is very common for Devicetrees in Zephyr: You start with your _board_, which uses a specific _MCU_, which has a certain _architecture_ and vendor-specific peripherals.

Each Devicetree source file can _reference_ nodes to add, remove, or modify properties for each included file. E.g., for our console output we can find the following parts (the below snippets are incomplete!) in the Devicetree source file of the development kit:

`zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts`

```dts
/ {
  chosen {
    zephyr,console = &uart0;
  };
};

&uart0 {
  compatible = "nordic,nrf-uarte";
  status = "okay";
  current-speed = <115200>;
  /* other properties */
};
```

With `zephyr,console` we seem to be able to tell Zephyr that we want it to use the node `uart0` for the console output and therefore `printk` statements. We're also modifying the `&uart0` reference's properties, e.g., we set the baud rate to `115200` and enable it by setting its status to "okay".

The `uart0` node is originally defined in the included Devicetree source file of the SoC `nrf52840.dtsi`, and seems to be disabled by default:

`zephyr/dts/arm/nordic/nrf52840.dtsi`

```dts
/ {
  soc {
    uart0: uart@40002000 {
      compatible = "nordic,nrf-uarte";
      reg = <0x40002000 0x1000>;
      interrupts = <2 NRF_DEFAULT_IRQ_PRIORITY>;
      status = "disabled";
    };
  };
};
```

But that's enough Devicetree syntax for now. After this step, we end up with a single `zephyr.dts.pre` Devicetree (DTS) source file in the `build/zephyr` output directory. In there, we can also find our `uart@40002000` node (with the value of the expanded `NRF_DEFAULT_IRQ_PRIORITY` macro):

`build/zephyr/zephyr.dts.pre`

```dts
/ {
  soc {
    uart0: uart@40002000 {
      compatible = "nordic,nrf-uarte";
      reg = < 0x40002000 0x1000 >;
      interrupts = < 0x2 0x1 >;
      status = "okay";
      current-speed = < 0x1c200 >;
      pinctrl-0 = < &uart0_default >;
      pinctrl-1 = < &uart0_sleep >;
      pinctrl-names = "default", "sleep";
    };
  };
};
```


### Compiling the Devicetree

At the beginning of this article, we mentioned that Zephyr uses the `dtc` Devicetree compiler to generate the corresponding source code. That is, however, not entirely true: While the official Devicetree compiler `dtc` is definitely invoked during the build process, it is not used to generate any source code. Instead, Zephyr feeds the generated `build/zephyr/zephyr.dts.pre` into its own `GEN_DEFINES_SCRIPT` Python script, located at `zephyr/scripts/dts/gen_defines.py`.

Now, why would you want to do that? The Devicetree compiler `dtc` is typically used to compile Devicetree sources into a _binary_ format called the Devicetree blob `dtb`. The Linux kernel parses the DTB and uses the information to configure and initialize the hardware components described in the DTB. This allows the kernel to know how to communicate with the hardware without hardcoding this information in the kernel code. Thus, in Linux, the Devicetree is parsed and loaded during _runtime_ and thus can be _changed_ without modifying the application.

Zephyr, however, is designed to run on resource-constrained, embedded systems. It is simply not feasible to load a Devicetree blob during runtime: Any such structure would take up too many resources in both the Zephyr drivers and storing the Devicetree blob. Instead, the Devicetree is resolved during _compile time_.

> **Note:** In case this article is too slow for you but you still want to know more about Devicetree, there is a [brilliant video of the Zephyr Development Summit 2022 by Martí Bolívar on Devicetree](https://www.youtube.com/watch?v=w8GgP3h0M8M&list=PLzRQULb6-ipFDwFONbHu-Qb305hJR7ICe).

So, if not a binary, what's the output of this `gen_defines.py` generator? Let's have another peek at the output of our build process:

```
-- Generated zephyr.dts: /path/to/build/zephyr/zephyr.dts
-- Generated devicetree_generated.h: /path/to/build/zephyr/include/generated/devicetree_generated.h
-- Including generated dts.cmake file: /path/to/build/zephyr/dts.cmake
```

The build process creates three important output files: The `zephyr.dts` that has been generated out of the preprocessed `zephyr.dts.pre`, a `devicetree_generated.h` header file, and a CMake file `dts.cmake`.

As promised, the original Devicetree `dtc` compiler _is_ invoked during the build, and that's where it comes into play: The `zephyr.dts` Devicetree source file is fed into `dtc`, but not to generate any binaries or source code: Instead, it is only used to generate warnings and errors; the output itself is discarded. This helps to reduce the complexity of the Python Devicetree script `gen_defines.py` and ensures that the generated Devicetree source file used in Zephyr is at least still compatible with the original specification.

The `devicetree_generated.h` header file replaces the Devicetree blob `dtb`. It contains _macros_ for "all things Devicetree" and is included by the drivers and our application thereby strips all unnecessary or unused parts. **"Macrobatics"** is the term that Martí Bolívar used in his [talk about the Zephyr Devicetree at the June 2022 developer summit](https://www.youtube.com/watch?v=w8GgP3h0M8M&list=PLzRQULb6-ipFDwFONbHu-Qb305hJR7ICe), and it fits. Even for our tiny application, the generated header is over 15000 lines of code! In the next article, we'll see what these macros look like and how they are used with the Zephyr Devicetree API. If you're curious, have a look at `zephyr/include/zephyr/devicetree.h` already. In fact, let's have a glimpse:

`build/zephyr/include/generated/devicetree_generated.h`

```c
#define DT_CHOSEN_zephyr_console DT_N_S_soc_S_uart_40002000
// --snip---
#define DT_N_S_soc_S_uart_40002000_P_current_speed 115200
#define DT_N_S_soc_S_uart_40002000_P_status "okay"
```

Looks cryptic? With just a few hints, this becomes readable. Know that:
- `DT_` is just the common prefix for Devicetree macros,
- `_S_` is a forward slash `/`,
- `_N_` refers to a _node_,
- `_P_` is a _property_.

Thus, e.g., `DT_N_S_soc_S_uart_40002000_P_current_speed` simply refers to the _property_ `current_speed` of the _node_ `/soc/uart_40002000`. In Zephyr, this configuration value is set during _compile time_. You'll need to recompile your application in case you want to change this property. The approach in Linux would be different: There, the (UART speed) property is read from the Devicetree blob `dtb` during runtime. You could change the property, recompile the Devicetree, exchange the Devicetree blob, and wouldn't need to touch your application or the Kernel at all.

But let's leave it at that for now, we'll have a proper look at this later. For now, it is just important to know that we'll resolve our Devicetree at _compile time_ using lots of generated macros.

Finally, the generated `dts.cmake` is a file that basically allows to access the entire Devicetree from within CMake, using CMake target properties, e.g., we'll find the _current speed_ of our UART peripheral also within CMake:

`dts.cmake`

```cmake
set_target_properties(
    devicetree_target
    PROPERTIES
    "DT_PROP|/soc/uart@40002000|current-speed" "115200"
)
```

That's it for a peek into the Zephyr build. Let's wrap it up:
- Zephyr uses a so-called Devicetree to describe hardware.
- Most of the Devicetree sources are already available within Zephyr, e.g., MCUs and boards.
- We'll use Devicetree source files mostly to override or extend existing DTS files.
- In Zephyr, the Devicetree is resolved at _compile time_, using [macrobatics](https://www.youtube.com/watch?v=w8GgP3h0M8M&list=PLzRQULb6-ipFDwFONbHu-Qb305hJR7ICe).



## Basic source file syntax

Now we finally take a closer look at the Devicetree syntax and its files. We'll walk through it by creating our own Devicetree nodes. This section heavily borrows from existing documentation such as the [Devicetree specification](https://www.devicetree.org/specifications/), [Zephyr's Devicetree docs](https://docs.zephyrproject.org/latest/build/dts/index.html) and the [nRF Connect SDK Fundamentals lesson on Devicetree](https://academy.nordicsemi.com/topic/devicetree/).

> **Note:** We won't explain _all_ syntactical aspects, but will focus on the most important ones. Have a look at the [Devicetree specification](https://www.devicetree.org/specifications/) for a full syntax description. Also, misplaced semicolons, etc., will also be caught by the Devicetree compiler in the future, so we won't talk about trivial details.


### Node names, unit-addresses, `reg`, and labels

Let's start from scratch. We create an empty Devicetree source file `.dts` with the following empty tree:

```dts
/dts-v1/;
/ { /* Empty. */ };
```

The first line contains the _tag_ `/dts-v1/;` which identifies the file as a version _1_ Devicetree source file. Without this tag, the Devicetree compiler would treat the file as being of the obsolete version _0_ - which is incompatible with the current major Devicetree version _1_ used by Zephyr. The tag `/dts-v1/;` is therefore required when working with Zephyr. Following the version tag is an empty Devicetree: Its only _node_ is the _root node_, identified by convention by a forward slash `/`.

> **Note:** Devicetree source files use `C/C++` style comments. You can use both, multi-line `/* ... */` and single-line `//` comments.

Within this root node, we can now define our own nodes in the form of a tree, kind of like a `JSON` object or nested `C` structure:

```dts
/dts-v1/;

/ {
  node {
    subnode {
      /* name/value properties */
    };
  };
};
```

Nodes are identified via their _node name_. Each node can have _subnodes_ and _properties_. In the above example, we have a node with the name _node_, containing a subnode named, well, _subnode_. For now, all you need to know about _properties_ is that they are name/value pairs.

A node in the Devicetree can be uniquely identified by specifying the full _path_ from the root node, through all subnodes, to the desired node, separated by forward slashes. E.g., our full path to our _subnode_ is `/node/subnode`.

Node names can also have an optional, hexadecimal _unit-address_, specified using an `@` and thus resulting in the full node name format `node-name@unit-address`. E.g., we could give our `subnode` the _unit-address_ `0123ABC` as follows:

```dts
/dts-v1/;

/ {
  node {
    subnode@0123ABC {
      reg = <0x0123ABC>;
      /* properties */
    };
  };
};
```

The _unit-address_ can be used to distinguish between several subnodes of the same type. It can be a real register address, typically a base address, e.g., the base address of the register space of a specific UART interface, but also a plain instance number, e.g., when describing a multi-core MCU by using a `/cpus` node, with two instances `cpu@0` and `cpu@1` for each CPU core. Each node with a _unit-address_ **must** also have the property `reg` - and any node _without_ a _unit-address_ must _not_ have the property `reg`. It may seem redundant now, but we'll learn more about the `reg` property later in this article.

Let's finish up on the node name with a convention that ensures that each node in the Devicetree can be uniquely identified by specifying its full _path_. For any node name and property at the same level in the tree:
- in the case of _node-name_ without an _unit-address_ the `node-name` must be unique,
- or if a node has a _unit-address_, then the full `node-name@unit-address` must be unique.

Now we can address all of our nodes using their full path. As you might imagine, within a more complex Devicetree the paths become quite long, and if you'd ever need to reference a node (we'll see how that works in practice later) this is quite tedious, which is why any node can be assigned a unique _node label_:

```dts
/dts-v1/;

/ {
  node {
    subnode_label: subnode@0123ABC {
      reg = <0x0123ABC>;
      /* properties */
    };
  };
};
```

Now, instead of using `/node/subnode@0123ABC` to identify the node, we can simply use its label `subnode_label` - which must be **unique** throughout the entire Devicetree. This form of the label syntax is known as a _node label_. This term is not explicitly defined in the Devicetree specification but is used extensively in Zephyr.

That's it for the first theory lesson - let's see how nodes look like in a real Devicetree source (_include_) file in Zephyr. The following is a snippet of the Devicetree source file for the nRF52840 microcontroller:

`zephyr/dts/arm/nordic/nrf52840.dtsi`

```dts
/ {
  soc {
    uart0: uart@40002000 {
      reg = <0x40002000 0x1000>;
    };
    uart1: uart@40028000 {
      reg = <0x40028000 0x1000>;
    };
  };
};
```

The System-On-Chip is described using the `soc` node. The `soc` node contains two _uart_ instances, which match the UARTE instances that we can find in the register map of the nRF52840 datasheet (the `E` in UARTE refers to _EasyDMA_ support):

|  ID   | Base address | Peripheral | Instance | Description               |
| :---: | :----------: | :--------: | :------: | :------------------------ |
|   0   |  0x40000000  |   CLOCK    |  CLOCK   | Clock control             |
|       |     ...      |            |          |                           |
|   2   |  0x40002000  |   UARTE    |  UARTE0  | UART with EasyDMA, unit 0 |
|   3   |  0x40003000  |    SPIM    |  SPIM0   | SPI master 0              |
|       |     ...      |            |          |                           |
|  40   |  0x40028000  |   UARTE    |  UARTE1  | UART with EasyDMA, unit 1 |
|  41   |  0x40029000  |    QSPI    |   QSPI   | External memory interface |
|       |     ...      |            |          |                           |

As we can see, the _unit-address_ of each `uart` node matches its base address. The `reg` property seems to be some kind of value list enclosed in angle brackets `<...>`. The first value of the property matches the node's _unit-address_ and thus the base address of the UARTE instance, but the property also has a second value and thus provides more information than the _unit-address_ itself: Looking at the register map we can see that the second value `0x1000` matches the length of the address space that is reserved for each UARTE instance:

- The base address of the `UARTE0` instance is `0x40002000`, followed by `SPIM0` at `0x40003000`.
- The base address of the `UARTE1` instance is `0x40028000`, followed by `QSPI` at `0x40029000`.

Finally, each UART instance also has a unique label:
- `uart0` is the label of the node `/soc/uart@40002000`,
- `uart1` is the label of the node `/soc/uart@40028000`.

> **Note:** Throughout this article, we sometimes refer to _node_ labels as just "labels". The [Devicetree specification](https://www.devicetree.org/specifications/) also allows creating labels to reference properties and their values but Zephyr's DTS generator ignores this feature. We therefore use the terms _label_ and _node label_ interchangeably.


### Property names and basic value types

Let's now have a look at properties. As we've already seen for the property `reg`, properties consist of a _name_ and a _value_ and are used to describe the characteristics of the node. Property names can contain:

- digits `0-9`,
- lower and uppercase letters `a-z` and `A-Z`,
- and any of the special characters `.,_+?#-`.

Most of the properties you'll encounter in Zephyr simply use `kebab-case` names, though you'll also encounter properties starting with a `#`, e.g., the standard properties `#address-cells` and `#size-cells` (also described in the [Devicetree specification](https://www.devicetree.org/specifications/)). The special character `#` is really just part of the property name, there's no magic behind it.

<!--
For describing property values, we'll follow the approach of the [official Devicetree specification (DTSpec)](https://www.devicetree.org/specifications/). If you're not interested in the details, skip ahead to the table containing value types and matching examples. The goal of the following paragraphs is to help you understand the connection between the DTSpec and [Zephyr's documentation](https://docs.zephyrproject.org/latest/build/dts/index.html) - which can sometimes be confusing. Let's start with the definition of a property value from the section _"Property values"_ in the [DTSpec](https://www.devicetree.org/specifications/):

> "A property value is an **array** of zero or more bytes that contain information associated with the property. Properties might have an empty value if conveying true-false information. In this case, the presence or absence of the property is sufficiently descriptive." [[DTSpec]](https://www.devicetree.org/specifications/)

This is basically just a fancy way of saying that Devicetree supports properties without a value assignment and properties with one or multiple values. The [[DTSpec]](https://www.devicetree.org/specifications/) then goes ahead and provides the following possible _"property values"_ and their representation in memory. In our table, we can skip the memory representation since we've learned that [Zephyr doesn't compile the DTS files into a binary](#compiling-the-devicetree):

| Type                   | Description                                                            |
| :--------------------- | :--------------------------------------------------------------------- |
| `<empty>`              | Empty value. `true` if the property is present, otherwise `false`.     |
| `<u32>`                | A 32-bit integer in big endian format (a "cell").                      |
| `<u64>`                | A 64-bit integer in big endian format. Consists of two `<u32>` values. |
| `<string>`             | A printable, null-terminated string.                                   |
| `<prop-encoded-array>` | An array of values, where the type is defined by the property.         |
| `<phandle>`            | A `<u32>` value used to reference another node in the Devicetree.      |
| `<stringlist>`         | A list of concatenated `<string>` values.                              |

> **Note:** The term **"cell"** is typically used to refer to the individual data elements within properties and can thus be thought of as a single unit of data in a property. The [Devicetree specification](https://www.devicetree.org/specifications/) v0.4 defines a **"cell"** as _"a unit of information consisting of 32 bits"_ and thus explicitly defines its size as 32 bits. Meaning it is just another confusing name for a 32-bit integer.

If you find this information underwhelming - you're not alone. It also won't be of much help when trying to understand property values from Zephyr's Devicetree source files. The missing link to understanding the nature of the above table is the following: Values in Zephyr's DTS files are **represented** using the _"Devicetree Source (DTS) Format"_ version _1_, meaning they use a specific format or **syntax** to represent the above property value types. The above table, on the other hand, is a format-agnostic list of types that any specific DTS value format must translate to. E.g., a new _"Devicetree Source (DTS) Format"_ version _2_ might use a different syntax to represent values of the listed types.

The information that you'll most likely be interested in is part of the later section _"Devicetree Source (DTS) Format"_ in the DTSpec, specifically the subsection _"Node and property definitions"_. There, you'll find the matching syntax description for the above types - or at least the very basic information. Thankfully, Zephyr (and most likely any other ecosystem using Devicetree) doesn't just verbally describe its supported types but uses specific type names. We'll summarize the basics below. For details, refer to the [_"type"_ section in Zephyr's documentation on Devicetree bindings](https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#type) or [Zephyr's Devicetree introduction on property values](https://docs.zephyrproject.org/latest/build/dts/intro-syntax-structure.html#writing-property-values).
-->

Zephyr uses the type names summarized below. For details, refer to the [_"type"_ section in Zephyr's documentation on Devicetree bindings](https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#type) or [Zephyr's Devicetree introduction on property values](https://docs.zephyrproject.org/latest/build/dts/intro-syntax-structure.html#writing-property-values).

> **Note:** If you're planning on reading the Devicetree specification, skip ahead to the section _"Devicetree Source (DTS) Format"_, specifically the subsection _"Node and property definitions"_. In case something is unclear, try to find the information in the initial chapters. Reading the specification top to bottom can be quite confusing.

The syntax used for property _values_ is a bit peculiar. Except for `phandles`, which we'll cover separately, the following table contains all property types supported by Zephyr and their DTSpec equivalent.

| Zephyr type      | DTSpec equivalent              | Example                                            |
| :--------------- | :----------------------------- | :------------------------------------------------- |
| `boolean`        | `property with no value`       | `interrupt-controller;`                            |
| `string`         | `string`                       | `status = "disabled";`                             |
| `array`          | `32-bit integer cells`         | `reg = <0x40002000 0x1000>;`                       |
| `int`            | `A single 32-bit integer cell` | `current-speed = <115200>;`                        |
| `64-bit integer` | `32-bit integer cells`         | `value = <0xBAADF00D 0xDEADBEEF>;`                 |
| `uint8-array`    | `bytestring`                   | `mac-address = [ DE AD BE EF 12 34 ];`             |
| `string-array`   | `stringlist`                   | `compatible = "nordic,nrf-egu", "nordic,nrf-swi";` |
| `compound`       | "comma-separated components"   | `foo = <1 2>, [3, 4], "five"`                      |

The table deserves some observations and explanations, but we'll repeat all the types just for the sake of completeness:

**`boolean`**

A `boolean` property is `true` if the property exists, otherwise `false`.

**`string`**

A `string` property is just like a `C` string literal: double-quoted, null-terminated text.

**`array`**

An `array` is a lists of values enclosed in `<` and `>`, separated by spaces. `arrays` could in theory contain mixed elements of any supported type but most commonly uses 32-bit integers. Just like in `C/C++`, the prefix `0x` is used for hexadecimal numbers, and numbers _without_ a prefix are decimals.

**`int`**

An `int`eger is represented as an `array` containing a single 32-bit value ("cell"): It's a single 32-bit value enclosed in `<` and `>`.

> **Note:** The term **"cell"** is typically used to refer to the individual data elements within properties and can thus be thought of as a single unit of data in a property. The [Devicetree specification](https://www.devicetree.org/specifications/) v0.4 defines a **"cell"** as _"a unit of information consisting of 32 bits"_ and thus explicitly defines its size as 32 bits. Meaning it is just another confusing name for a 32-bit integer.

64-bit integers do not have their own type but are instead represented by an array of two `int`egers.

**`uint8-array`**

A `uint-array` consists of 8-bit hexadecimal values enclosed in `[` and `]`, _optionally_ separated by spaces.

In contrast to an `array`, a `uint8-array` _always_ uses **hexadecimal** literals _without_ the prefix `0x` - which can be confusing at first. E.g., `<11 12>` represents the two _32-bit_ integers with the decimal values `11` and `12`, whereas `[11, 12]` represents the two _8-bit_ integers with the decimal values `17` and `18`.

The spaces between each byte in a `uint8-array` are optional. E.g., `mac-address = [ DEADBEEF1234 ];` is equivalent to `mac-address = [ DE AD BE EF 12 34 ];`. It practice, you'll rarely see `uint8-array`s without spaces between each byte.

**`string-array`**

A `string-array` is just a comma-separated list of strings. The only thing worth mentioning is, that the value of a `string-array` property may also be a single `string`. E.g., `compatible = "something"` is still a valid value assignment for a `string-array` property.

**`compound`**

The `compound` type is essentially a "catch-all" for custom types. Zephyr does **not** generate any code for `compound` properties. Also, notice how compound value definitions are syntactically ambiguous, e.g., for `foo = <1 2>, "three", "four"`: Is `"three", "four"` a single value of type `string-array`, or two separate `string` values?

When reading Zephyr DTS files, keep in mind that [in Zephyr all DTS files are fed into the preprocessor](#devicetree-includes-and-sources-in-zephyr) and therefore Zephyr allows the use of macros in DTS files. E.g., you might encounter properties like `max-frequency = <DT_FREQ_M(8)>;`, which do not match the Devicetree syntax at all. There, the preprocessor replaces the macro `DT_FREQ_M` with the corresponding literal before the source file is parsed.

The following is a snippet of a test file of Zephyr's Python Devicetree generator. It contains the node `props` that nicely demonstrate the different property types supported by Zephyr.

`zephyr/scripts/dts/python-devicetree/tests/test.dts`

```dts
/dts-v1/;

/ {
  props {
    existent-boolean;
    int = <1>;
    array = <1 2 3>;
    uint8-array = [ 12 34 ];
    string = "foo";
    string-array = "foo", "bar", "baz";
  };
};
```

There's one more thing that is worth mentioning: Parentheses, arithmetic operators, and bitwise operators are allowed in property values, though the entire expression must be parenthesized. [Zephyr's introduction on property values](https://docs.zephyrproject.org/latest/build/dts/intro-syntax-structure.html#writing-property-values) provides the following example for a property `bar`:

```dts
/ {
  foo {
    bar = <(2 * (1 << 5))>;
  };
};
```

The property `bar` contains a single 32-bit value ("cell") with the value _64_. Notice that operators are not just allowed in Zephr, but also according to the [Devicetree specification](https://www.devicetree.org/specifications/): You therefore don't _need_ to use macros for simple arithmetic or bit operations.


## References and `phandle` types

We've already seen how we can create _node labels_ as shorthand forms of a node's full path, but haven't really seen how such labels are used within the Devicetree. Tired of all the theory? I thought so. Now that we're familiar with a good part of the Devicetree source file syntax, it's time for _a little_ hands-on, so let's dive back into the command line.

We'll practice using a Devicetree _overlay_ file. In the next article, we'll go more into detail about what an overlay is. For now, it is enough to know that an overlay file is simply an additional DTS file on top of the hierarchy of files that is included starting with the board's Devicetree source file. We can [specify an extra Devicetree overlay file using the CMake variable `EXTRA_DTC_OVERLAY_FILE`](https://docs.zephyrproject.org/latest/build/dts/howtos.html#set-devicetree-overlays), and we'll use a newly created `props-phandles.overlay` file for that:

```bash
$ mkdir -p dts/playground
$ touch dts/playground/props-phandles.overlay
$ rm -rf ../build
$ west build --board nrf52840dk_nrf52840 --build-dir ../build -- \
  -DEXTRA_DTC_OVERLAY_FILE=dts/playground/props-phandles.overlay
```

The build system's output now announces that it encountered the newly created overlay file with the message `Found devicetree overlay`:

```
-- Found Dtc: /opt/nordic/ncs/toolchains/4ef6631da0/bin/dtc (found suitable version "1.6.1", minimum required is "1.4.6")
-- Found BOARD.dts: /opt/nordic/ncs/v2.4.0/zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts
-- Found devicetree overlay: playground/test.overlay
-- Generated zephyr.dts: /path/to/build/zephyr/zephyr.dts
```


### `phandle`

So what is a `phandle`? The easy answer is: Devicetree source files need some way to refer to nodes, something like pointers in `C`, and that's what `phandle`s are. If we're being picky about the terminology - it's complicated. Why?

- In the [DTSpec](https://www.devicetree.org/specifications/), we've seen that a `phandle` is a base **type**.
- In addition, the [DTSpec](https://www.devicetree.org/specifications/) also defines a standard **property** named `phandle` - ironically of type _32-bit integer_, and not _phandle_. We'll see this property in just a second.
- In Zephyr, the term `phandle` is used pretty much only for node **references** in any format and never even mentions the same named _property_.

Why this ambiguity? Because in the end, any reference to a node is replaced by a unique, 32-bit value that identifies the node - the value stored in the node's `phandle` property. The fact that `phandle` _property_ is not intended to be set manually, but is instead created by the Devicetree compiler for each referenced node, makes mentioning the `phandle` property as such unnecessary. Thus, the approach chosen in Zephyr's documentation - referring to any reference as `phandle` - makes a lot of sense.

But enough nit-picking. Let's see how this looks in a real Devicetree source file. Let's create two nodes `node_a` and `node_refs` in our overlay file, and have `node_refs` reference the `node_a` once by its path and once by a label `label_a` that we create for `node_a`. How do we do this? The syntax is specified in the [DTSpec](https://www.devicetree.org/specifications/) as follows:

> "Labels are created by appending a colon ('`:`') to the label name. References are created by prefixing the label name with an ampersand ('`&`'), or they may be followed by a node's full path in braces." [[DTSpec]](https://www.devicetree.org/specifications/)

Thus, in our Devicetree overlay file, we can create the properties as shown below. Notice how we're missing the DTS version `/dts-v1/;`: The version is only defined Devicetree *source* files, but **not** in *overlay* files.

```dts
/ {
  label_a: node_a { /* Empty. */ };
  node_refs {
    phandle-by-path = <&{/node_a}>;
    phandle-by-label = <&label_a>;
  };
};
```

Let's run a build to ensure that our overlay is sent through the preprocessor so that we can have a look at the generated `zephyr.dts`:

```bash
$ west build --board nrf52840dk_nrf52840 --build-dir ../build -- \
  -DEXTRA_DTC_OVERLAY_FILE=dts/playground/props-phandles.overlay
```

`build/zephyr/zephyr.dts`

```dts
/dts-v1/;

/ {
  /* Possibly lots of other nodes ... */
  label_a: node_a {
    phandle = < 0x1c >;
  };
  node_refs {
    phandle-by-path = < &{/node_a} >;
    phandle-by-label = < &label_a >;
  };
};
```

We can now see that the generator has created a `phandle` property for our referenced `node_a`. Your mileage may vary on the exact value for the property since it depends on the number of referenced nodes in the set of DTS files that are merged. What is this `phandle` property? The [DTSpec](https://www.devicetree.org/specifications/) defines `phandle` as follows:

> Property name `phandle`, value type _32-bit integer_.
>
> The `phandle` property specifies a numerical identifier for a node that is unique within the Devicetree. The `phandle` property value is used by other nodes that need to refer to the node associated with the property.
>
> **Note:** Most devicetrees [...] will not contain explicit phandle properties. The DTC tool automatically inserts the phandle properties when the DTS is compiled [...].

This is also what we see in the generated `zephyr.dts`: Since `node_a` is referenced by `node_ref`, Zephyr's DTS generator has inserted the property `phandle` for `node_a` in our Devicetree. To see what happens to the reference within this `phandle` property, we need to jump back to the syntax section in the [DTSpec](https://www.devicetree.org/specifications/), where we find the following:

> "In a cell array, a reference to another node will be expanded to that node's phandle."

This means that the references `&{/node_a}` and `&label_a` in our properties `phandle-by-path` and `phandle-by-label` are essentially expanded to `node_a`'s `phandle` _0x1c_. Thus, **the reference is equivalent to its phandle**. Zephyr's documentation is right to refer to `&{/node_a}` and `&label_a` as "`phandle`s". Could we also define `node_a`'s `phandle` property by ourselves? Let's find out:

```dts
/ {
  label_a: node_a {
    phandle = <0xC0FFEE>;
  };
  node_refs {
    phandle-by-path = <&{/node_a}>;
    phandle-by-label = <&label_a>;
  };
};
```

After executing `west build` with the previous parameters again, we indeed end up with the value `0xc0ffee` for `node_a`'s `phandle` property:

`build/zephyr/zephyr.dts`

```dts
/dts-v1/;

/ {
  /* Possibly lots of other nodes ... */
  label_a: node_a {
    phandle = < 0xc0ffee >;
  };
  node_refs {
    phandle-by-path = < &{/node_a} >;
    phandle-by-label = < &label_a >;
  };
};
```

What's the use of a `phandle`? A single `phandle` can be useful where a 1:1 relation between nodes in the Devicetree is required. E.g., the following is a snippet from the nRF52840 DTS include file, which contains a software PWM node with a configurable generator, which by default refers to `timer2`:

`zephyr/dts/arm/nordic/nrf52840.dtsi`

```dts
/ {
  /* ... */
  sw_pwm: sw-pwm {
    compatible = "nordic,nrf-sw-pwm";
    status = "disabled";
    generator = <&timer2>;
    clock-prescaler = <0>;
    #pwm-cells = <3>;
  };
};
```

We've seen the type of the _phandle_ property, but what about the types of the properties `phandle-by-path` and `phandle-by-label`? Knowing that a `phandle` is expanded to a *cell* we might guess that `phandle-by-path` and `phandle-by-label` are of type `int` or `array`: Assuming that `node_a`'s _phandle_ has the value 0xc0ffee, both `<&{/node_a}>` and `<&label_a>` essentially expand to `<0xc0ffee>`, which could either be a single `int` or an `array` of size `1` containing a 32-bit value.

In Zephyr, however, an array containing a **single** node reference has its own type **`phandle`**.


### `path`, `phandles` and `phandle-array`

In addition to the `phandle` type, three more types are available in Zephyr when dealing with `phandle`s and references:
<!--
| Zephyr type     | DTSpec equivalent      | Syntax                                   | Example                                |
| :-------------- | :--------------------- | :--------------------------------------- | :------------------------------------- |
| `path`          | `<prop-encoded-array>` | A plain node path string or reference    | `zephyr,console = &uart0;`             |
| `phandle`       | `<phandle>`            | An `array` containing a single reference | `pinctrl-0 = <&uart0_default>;`        |
| `phandles`      | `<prop-encoded-array>` | An array of references                   | `cpu-power-states = <&idle &suspend>;` |
| `phandle-array` | `<prop-encoded-array>` | An array containing references and cells | `gpios = <&gpio0 13 GPIO_ACTIVE_LOW>;` |
-->

| Zephyr type     | DTSpec equivalent      | Example                                |
| :-------------- | :--------------------- | :------------------------------------- |
| `path`          | `32-bit integer cells` | `zephyr,console = &uart0;`             |
| `phandle`       | `phandle`              | `pinctrl-0 = <&uart0_default>;`        |
| `phandles`      | `32-bit integer cells` | `cpu-power-states = <&idle &suspend>;` |
| `phandle-array` | `32-bit integer cells` | `gpios = <&gpio0 13 GPIO_ACTIVE_LOW>;` |

Let's go through the remaining types one by one, starting with `path`s: In our previous example, we've enclosed the references in `<` and `>` and ended up with our `phandle` type. If we don't place the reference within `<` and `>`, the [DTSpec](https://www.devicetree.org/specifications/) defines the following behavior:

> "Outside a cell array, a reference to another node will be expanded to that node's full path."

So let's try this out: Let's first get rid of the properties `phandle-by-path` and `phandle-by-label`, and create two new properties `path-by-path` and `path-by-label` as follows:

```dts
/ {
  label_a: node_a { /* Empty. */ };
  node_refs {
    path-by-path = &{/node_a};
    path-by-label = &label_a;
  };
};
```

If we now execute `west build`, we can see something interesting in our generated `zephyr.dts`:

`build/zephyr/zephyr.dts`

```dts
/dts-v1/;

/ {
  /* Lots of other nodes ... */
  label_a: node_a {
  };
  node_refs {
    path-by-path = &{/node_a};
    path-by-label = &label_a;
  };
};
```

The `phandle` property for `node_a` is gone! The reason for this is simple - and also matches exactly what is stated in the [DTSpec](https://www.devicetree.org/specifications/): The references used for the values of the properties `path-by-path` and `path-by-label` are really just a different notation for the path `/node_a`. They are **not** `phandle`s, and therefore do not require a property _phandle_ in the referenced node.

In Zephyr, you'll encounter `path`s exclusively for properties of the standard nodes `/aliases` and `/chosen`, both of which we'll see in a [later section in this article](#about-aliases-and-chosen). Meaning: You can't actually use `path`s for your own nodes.

Next, `phandles`. This is not a typo: The plural form `phandles` of `phandle` is really a separate type in Zephyr, and it's as simple as it sounds: Instead of supporting only a single _reference_ - or _phandle_ - in its value, it is an **array** of _phandles_. Let's create another `node_b` and change `node_refs` to contain only a new property `phandles` and recompile:


```dts
/ {
  label_a: node_a { };
  label_b: node_b { };
  node_refs {
    phandles = <&{/node_a} &label_b>;
  };
};
```
`build/zephyr/zephyr.dts`

```dts
/dts-v1/;

/ {
  /* Lots of other nodes ... */
  label_a: node_a {
    phandle = < 0x1c >;
  };
  label_b: node_b {
    phandle = < 0x1d >;
  };
  node_refs {
    phandles = < &{/node_a} &label_b >;
  };
};
```

The output is not surprising: Both nodes are now referenced by `node_ref`'s `phandles` and therefore get their own `phandle` property. The `phandles` type is especially useful when we need to reference nodes of the same type, like the `cpu-power-states` example in the previous table.

This leaves us with the last type `phandle-array`, a type that is used quite a lot in Devicetree. Let's go by example:

```dts
/ {
  label_a: node_a {
    #phandle-array-of-ref-cells = <2>;
  };
  label_b: node_b {
    #phandle-array-of-ref-cells = <1>;
  };
  node_refs {
    phandle-array-of-refs = <&{/node_a} 1 2 &label_b 3>;
  };
};
```
`build/zephyr/zephyr.dts`

```dts
/dts-v1/;

/ {
  /* Lots of other nodes ... */
  label_a: node_a {
    #phandle-array-of-ref-cells = < 0x2 >;
    phandle = < 0x1c >;
  };
  label_b: node_b {
    #phandle-array-of-ref-cells = < 0x1 >;
    phandle = < 0x1d >;
  };
  node_refs {
    phandle-array-of-refs = < &{/node_a} 0x1 0x2 &label_b 0x3 >;
  };
};
```

So what's this `phandle-array` type? It is a list of phandles _with metadata_ for each phandle. This is how it works:

- By convention, a `phandle-array` property is plural and its name should thus end with "_s_".
- The value of a `phandle-array` property is an array of phandles, but each phandle can be followed by cells (32-bit values), sometimes also called a phandle's "metadata". In the example above, the two values `1 2` are `&{/node_a}`'s _metadata_, whereas `3` is `&label_b`'s metadata.
- The new properties `#phandle-array-of-ref-cells` tell the compiler how many metadata _cells_ are supported by the corresponding node. Such properties are called [specifier cells](https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#specifier-cell-names-cells): In our example, `node_a` specifies that the node supports _two_ cells, `node_b`'s specifier cell only allows _one_ cell after its phandle.

_Specifier cells_ like `#phandle-array-of-ref-cells` have a defined naming convention: The name is formed by removing the plural '_s_' and attaching '_-cells_' to the name of the `phandle-array` property. For our property _phandle-array-of-refs_, we thus end up with _phandle-array-of-ref~~s~~**-cells**_.

If you've been browsing Zephyr's Devicetree files, you may have noticed that the property `#gpio-cells` doesn't follow the specifier cell naming convention: Every rule has its exceptions, and in case you're interested in details, I'll leave you with a reference to [Zephyr's documentation on specifier cells](https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#specifier-cell-names-cells).

Before we look at a real-world example of a `phandle-array`, let's sum up `phandle` types. You'll find this exact list also in [Zephyr's documentation on `phandle` types](https://docs.zephyrproject.org/latest/build/dts/phandles.html), but since it is a short one, I hope it's OK to borrow it:

- To reference exactly one node, use the `phandle` type.
- To reference _zero_ or more nodes, we use the `phandles` type.
- To reference _zero_ or more nodes **with** metadata, we use a `phandle-array`.

In case you're still confused, we'll look at a real-world example for a `phandle-array` just now.


#### Syntax and semantics for `phandle-array`s

In case you've been experimenting with the above overlay, you may have noticed that I've been cheating a little: The project still _compiles_ just fine even if you delete the `#phandle-array-of-ref-cells` properties, or give `phandle-array-of-refs` a different name that does _not_ end in an _s_.

Why? Our Devicetree is still _syntactically_ correct even if we do not follow the given convention. In the end, `phandle-array-of-refs` is simply an array of cells since every reference is expanded to the node's `phandle` property's value - even without the expansion, its syntax would still fit an array of 32-bit integer cells. A _syntactically_ sound Devicetree, however, is only half the job: Eventually, we'll have to define some _schema_ and add _meaning_ to all the properties and their values; we'll have to define the Devicetree's **semantics**.

Without semantics, the DTS generator can't make sense of the provided Devicetree and therefore also won't generate anything that you'd be able to use in your application. Once you add semantics to your nodes, you'll have to strictly follow the previous convention, which is also why I've already included the notation in this article. The details, however, we'll explore in the next article about Devicetree _bindings_.


#### `phandle-array` in practice

How are `phandle-array`s used in practice? Let's look at the nRF52840's _General Purpose input and outputs (GPIOs)_. In the datasheet, we can find the following table on the GPIO instances:

| Base address | Peripheral | Instance | Description  | Configuration  |
| :----------: | :--------: | :------: | :----------- | :------------- |
|  0x50000000  |    GPIO    |    P0    | GPIO, port 0 | P0.00 to P0.31 |
|  0x50000300  |    GPIO    |    P1    | GPIO, port 1 | P1.00 to P1.15 |

Thus, the nRF52840 uses two instances `P0` and `P1` of the GPIO peripheral to expose control over its GPIOs. In the nRF52840 DTS include file, we find the matching nodes `gpio0` for port _0_ and `gpio1` for port _1_:

`zephyr/dts/arm/nordic/nrf52840.dtsi`

```dts
/ {
  soc {
    gpio0: gpio@50000000 {
      compatible = "nordic,nrf-gpio";
      gpio-controller;
      reg = <0x50000000 0x200 0x50000500 0x300>;
      #gpio-cells = <2>;
      status = "disabled";
      port = <0>;
    };

    gpio1: gpio@50000300 {
      compatible = "nordic,nrf-gpio";
      gpio-controller;
      reg = <0x50000300 0x200 0x50000800 0x300>;
      #gpio-cells = <2>;
      ngpios = <16>;
      status = "disabled";
      port = <1>;
    };
  };
};
```

With this structure it is clear that we cannot just reference the entire `gpio` instance in case we want to control only one pin: We always have to choose either `gpio0` or `gpio1` _and_ specify at least which exact _pin_ of the _port_ we need. Let's see how this is solved in the nRF52840 development kit's Devicetree source file.

The nRF52840 development kit connects LEDs to the nRF52840 MCU, which are described using the node `leds` in the board's DTS file. Within this Devicetree, we now see how the `led` instances reference to the nRF52840's GPIOs: E.g., `gpio0` is used in `led0`'s property `gpios` using a `phandle-array`:

`zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts`

```dts
/ {
  leds {
    compatible = "gpio-leds";
    led0: led_0 {
      gpios = <&gpio0 13 GPIO_ACTIVE_LOW>;
      label = "Green LED 0";
    };
  };
};
```

There are no individual nodes for each pin in the nRF52840's Devicetree. Therefore, when referencing the `gpio0` node, we need to be able to tell exactly which pin we're using for our LED. In addition, we also typically need to provide some configuration for our pin, e.g., set the pin to _active low_.

The nodes `gpio0` and `gpio1` both contain the _specifier cells_ `#gpio-cells` which indicate that we need to pass exactly two _cells_ to use the node in a `phandle-array`. In the `led0`'s property `gpios` of type `phandle-array` we can see that we do exactly that: We use the two cells to specify the _pin_ and _flags_ that we're using for the LED. Now, how would we know that `13` is the pin number and `GPIO_ACTIVE_LOW` is a flag?

As we've seen before, without additional information all the DTS compiler can do is make sure the _syntax_ of your file is correct. It doesn't know anything about the **semantics** and therefore can't really associate the values in the `phandle-array` to `gpio0`. It therefore also doesn't care about any semantic requirements.

In the next article, we'll see how to use the standard property `compatible` and so-called **bindings** to provide the semantics - and thus the information that we need to associate the first cell with the _pin_ number and the second cell with the _flags_.



## A complete list of Zephyr's property types

Having explored `phandle` types and `paths`, we can complete the list of types that are used in Zephyr devicetrees. You can find the same information in [Zephyr's documentation on bindings](https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#type) and [Zephyr's how-to on property values](https://docs.zephyrproject.org/latest/build/dts/intro-syntax-structure.html#writing-property-values).

| Zephyr type      | DTSpec equivalent              | Example                                            |
| :--------------- | :----------------------------- | :------------------------------------------------- |
| `boolean`        | `property with no value`       | `interrupt-controller;`                            |
| `string`         | `string`                       | `status = "disabled";`                             |
| `array`          | `32-bit integer cells`         | `reg = <0x40002000 0x1000>;`                       |
| `int`            | `A single 32-bit integer cell` | `current-speed = <115200>;`                        |
| `64-bit integer` | `32-bit integer cells`         | `value = <0xBAADF00D 0xDEADBEEF>;`                 |
| `uint8-array`    | `bytestring`                   | `mac-address = [ DE AD BE EF 12 34 ];`             |
| `string-array`   | `stringlist`                   | `compatible = "nordic,nrf-egu", "nordic,nrf-swi";` |
| `compound`       | "comma-separated components"   | `foo = <1 2>, [3, 4], "five"`                      |
| `path`           | `32-bit integer cells`         | `zephyr,console = &uart0;`                         |
| `phandle`        | `phandle`                      | `pinctrl-0 = <&uart0_default>;`                    |
| `phandles`       | `32-bit integer cells`         | `cpu-power-states = <&idle &suspend>;`             |
| `phandle-array`  | `32-bit integer cells`         | `gpios = <&gpio0 13 GPIO_ACTIVE_LOW>;`             |

<!--
| Zephyr type     | DTSpec equivalent                      | Syntax                                                                                                  | Example                                            |
| :-------------- | :------------------------------------- | :------------------------------------------------------------------------------------------------------ | :------------------------------------------------- |
| `boolean`       | `<empty>`                              | no value; a property is `true` if the property exists                                                   | `interrupt-controller;`                            |
| `string`        | `<string>`                             | double-quoted text (null terminated string)                                                             | `status = "disabled";`                             |
| `array`         | `<prop-encoded-array>`                 | 32-bit values enclosed in `<` and `>`, separated by spaces                                              | `reg = <0x40002000 0x1000>;`                       |
| `int`           | `<u32>`                                | a single 32-bit value ("cell"), enclosed in `<` and `>`                                                 | `current-speed = <115200>;`                        |
| `array`         | `<u64>`                                | 64-bit values are represented by an array of two *cells*                                                | `value = <0xBAADF00D 0xDEADBEEF>;`                 |
| `uint8-array`   | `<prop-encoded-array>` or "bytestring" | 8-bit hexadecimal values _without_ `0x` prefix, enclosed in `[` and `]`, separated by spaces (optional) | `mac-address = [ DE AD BE EF 12 34 ];`             |
| `string-array`  | `<stringlist>`                         | `string`s, separated by commas                                                                          | `compatible = "nordic,nrf-egu", "nordic,nrf-swi";` |
| `path`          | `<prop-encoded-array>`                 | A plain node path string or reference                                                                   | `zephyr,console = &uart0;`                         |
| `phandle`       | `<phandle>`                            | An `array` containing a single reference                                                                | `pinctrl-0 = <&uart0_default>;`                    |
| `phandles`      | `<prop-encoded-array>`                 | An array of references                                                                                  | `cpu-power-states = <&idle &suspend>;`             |
| `phandle-array` | `<prop-encoded-array>`                 | An array containing references and cells                                                                | `gpios = <&gpio0 13 GPIO_ACTIVE_LOW>;`             |
| `compound`      | "comma-separated components"           | comma-separated values                                                                                  | `foo = <1 2>, [3, 4], "five"`                      |
-->

Again, it is worth mentioning that the `compound` type is only a "catch-all" for custom types. Zephyr does **not** generate any macros for `compound` properties.



## About `/aliases` and `/chosen`

There are two standard _nodes_ that are important to know when talking about *Devicetree*: `/aliases` and `/chosen`. We'll see them again in the next article about Devicetree bindings, but since they are quite prominent in Zephyr's DTS files, we can't just leave them aside for now.


### `/aliases`

Let's have a look at `/aliases` first. The [DTSpec](https://www.devicetree.org/specifications/) specifies that the `/aliases` node is a child node of the root node. The following is specified for its properties:

> Each property of the `/aliases` node defines an alias. The property _"name"_ specifies the _alias name_. The property _"value"_ specifies the _full **path** to a node in the Devicetree_. [[DTSpec]](https://www.devicetree.org/specifications/)

Simply put, `/aliases` are just yet another way to get the full path to nodes _in your application_. In the [previous section](#path-phandles-and-phandle-array), we've learned that outside of `<` and `>` a reference to another node is expanded to that node's full path. Thus, for any alias, we can specify its **value of type `path`** using references or a plain string, as shown in the example below:

```dts
/ {
  aliases {
    alias-by-label = &label_a;
    alias-by-path = &{/node_a};
    alias-as-string = "/node_a";
  };
  label_a: node_a {
    /* Empty. */
  };
};
```

So what's the point of having an alias? Can't we use labels instead? Well, yes - but also no. As mentioned, _for the application_ there is no real difference between referring to a node via its label, its full path - or using its alias. We'll learn about the Devicetree API in Zephyr in the next article. For now, know that there are three macros `DT_ALIAS`, `DT_LABEL`, and `DT_PATH` in `zephyr/include/zephyr/devicetree.h` that you can use to get a node's identifier.

Why the emphasis on "_the application_" and what exactly doesn't work? It's important to know that `/aliases` is just another node with properties that are compiled accordingly. You can**not** use aliases in the Devicetree itself as you can do with _labels_. Thus, you can't replace an occurrence of a label with its alias. E.g., the following does not compile:

```dts
/ {
  aliases {
    alias-by-label = &label_a;
  };
  label_a: node_a {
    /* Empty. */
  };
  node_ref {
    // This doesn't work. An alias cannot be used like labels.
    phandle-by-alias = <&alias-by-label>;
  };
};
```

Still wondering why you'd need an _alias_ if you already have labels? Let's have a look at how aliases are used in Zephyr's DTS files. Say, we want to build an application that reads the state of a button. If we look at the nRF52840 development kit, we find the following nodes for the board's buttons:

`zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts`

```dts
/ {
  buttons {
    compatible = "gpio-keys";
    button0: button_0 { /* ... */ };
    button1: button_1 { /* ... */ };
    button2: button_2 { /* ... */ };
    button3: button_3 { /* ... */ };
  };
};
```

In our application, we could refer to `button_0` via its full path `/buttons/button_0`, or using its label `button0`. Let's say we want to run the same application on a different board, e.g., STM's Nucleo-C031C6. After all, Zephyr's promise is that switching boards should be easily doable, right? Let's have a look at its board's DTS file:

`zephyr/boards/arm/nucleo_c031c6/nucleo_c031c6.dts`

```dts
/ {
  gpio_keys {
    compatible = "gpio-keys";
    user_button: button { /* ... */ };
  };
};
```

So, our STM board has only one button, which has the path `/gpio_keys/button` and the label `user_button` - both incompatible with the references we'd use for our nRF development kit. Notice that `user_button` is a perfectly fine label for the only available button on the board.

However, if we'd like to use this button in our application, we'd now have to change our sources - or even worse - adapt the DTS files. Instead of doing this, we can use _aliases_ - and since buttons are commonly used throughout Zephyr's example applications, the corresponding alias, in our case `sw0`, already exists:

`zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts`

```dts
/ {
  aliases {
    led0 = &led0;
    /* ... */
    pwm-led0 = &pwm_led0;
    sw0 = &button0;
    /* ... */
  };
};
```

`zephyr/boards/arm/nucleo_c031c6/nucleo_c031c6.dts`

```dts
/ {
  aliases {
    led0 = &green_led_4;
    pwm-led0 = &green_pwm_led;
    sw0 = &user_button;
    /* ... */
  };
};
```

> **Note:** In case the aliases don't exist in the board DTS files, you could use overlay files - just like we already did in our examples. We'll see this again in the next article.

As you can see, there are also other examples where labels are not consistent. Instead, _aliases_ are used. We could change our application to get the node using the commonly available alias `sw0`, and it'll work with both boards.


### `/chosen`

Now what about the `/chosen` node? If you've been following along, or if you've just had another look at the DTS file of the nRF52840 development kit, then you might find that some of the `/chosen` nodes look an awful lot like what you find in `/aliases`, just with a different property name format:

`zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts`

```dts
/ {
  chosen {
    zephyr,console = &uart0;
    zephyr,shell-uart = &uart0;
    zephyr,uart-mcumgr = &uart0;
    zephyr,bt-mon-uart = &uart0;
    zephyr,bt-c2h-uart = &uart0;
    zephyr,sram = &sram0;
    zephyr,flash = &flash0;
    zephyr,code-partition = &slot0_partition;
    zephyr,ieee802154 = &ieee802154;
  };
};
```

So what's the difference between a property in `/chosen` and in `/aliases`? Let's first look at the definition of the `/chosen` node in the [DTSpec](https://www.devicetree.org/specifications/):

> The `/chosen` node does not represent a real device in the system but describes parameters chosen or specified by the system firmware at run time. It shall be a child of the root node. [[DTSpec]](https://www.devicetree.org/specifications/)

The first sentence can be a bit misleading: It doesn't mean that we cannot refer to "real devices" using `/chosen` properties, it simply means that a device defined as a `/chosen` property is always a _reference_. Thus, in short, `/chosen` contains a list of _system parameters_.

> **Note:** In Zephyr, `/chosen` is only used at build-time. There is no run-time feature.

According to the [DTSpec](https://www.devicetree.org/specifications/), technically, `/chosen` properties are not restricted to the `path` type. The following are acceptable `/chosen` properties according to the specification, and Zephyr's DTS generator does indeed accept them as input:

```dts
/ {
  chosen {
    chosen-by-label = &label_a;
    chosen-by-path = &{/node_a};
    chosen-as-string = "/node_a";
    chosen-foo = "bar";
    chosen-bar = <0xF00>;
    chosen-invalid = "/invalid/path/is/a/string";
  };
  label_a: node_a {
    /* Empty. */
  };
};
```

At the time of writing, Zephyr uses `/chosen` properties exclusively for references to other _nodes_ and therefore the `/chosen` node only contains properties of the type `path`. Also, the `DT_CHOSEN` macro in the Devicetree API is only used to retrieve node identifiers. You can find a list of all Zephyr-specific `/chosen` nodes [in the official documentation](https://docs.zephyrproject.org/latest/build/dts/api/api.html#devicetree-chosen-nodes).

So when would you use `/aliases` and when `/chosen` properties? If you want to specify a _node_ that is independent of the nodes in a Devicetree, you should use an _alias_ rather than a chosen property. `/chosen` is used to specify global configuration options and properties that affect the system as a whole. In Zephyr, `/chosen` is typically only used for [Zephyr-specific parameters](https://docs.zephyrproject.org/latest/build/dts/api/api.html#devicetree-chosen-nodes) and not for application-specific configuration options.

Also, think twice when considering adding `/chosen` properties - for some _configuration options_, [Kconfig](https://docs.zephyrproject.org/latest/build/kconfig/index.html#configuration-system-kconfig) is probably better suited. Zephyr's official documentation has a dedicated page on [Devicetree vs. Kconfig](https://docs.zephyrproject.org/latest/build/dts/dt-vs-kconfig.html), check it out!



## Complete examples and alternative array syntax

Along with [the complete list of Zephyr's property types](#a-complete-list-of-zephyrs-property-types), we can now create two overlay files as complete examples for basic types, `phandle`s, `/aliases`, and `/chosen` nodes.

> **Note:** The full example application including all the Devicetree files that we've seen throughout this article is available in the [`02_devicetree_basics` folder of the accompanying GitHub repository](https://github.com/lmapii/practical-zephyr/tree/main/02_devicetree_basics).

The first example covers all the basic types that we've seen:

`dts/playground/props-basics.overlay`

```dts
/ {
  aliases {
    // Aliases cannot be used as a references in the Devicetree itself, but are
    // used within the application as an alternative name for a node.
    alias-by-label = &label_equivalent;
    alias-by-path = &{/node_with_equivalent_arrays};
    alias-as-string = "/node_with_equivalent_arrays";
  };

  chosen {
    // `chosen` describes parameters "chosen" or specified by the application. In Zephyr,
    // all `chosen` parameters are paths to nodes.
    chosen-by-label = &label_equivalent;
    chosen-by-path = &{/node_with_equivalent_arrays};
    chosen-as-string = "/node_with_equivalent_arrays";
    // Technically, `chosen` properties can have any valid type.
    chosen-foo = "bar";
    chosen-bar = <0xF00>;
  };

  node_with_props {
    existent-boolean;
    int = <1>;
    array = <1 2 3>;
    uint8-array = [ 12 34 ];
    string = "foo";
    string-array = "foo", "bar", "baz";
  };
  label_equivalent: node_with_equivalent_arrays {
    // No spaces needed for uint8-array values.
    uint8-array = [ 1234 ];
    // Alternative syntax for arrays.
    array = <1>, <2>, <3>;
    int = <1>;
  };
};

// It is not possible to refer to a node via its alias - aliases are just properties!
// &alias-by-label {... };

// It is possible to "extend" and overwrite (non-const) properties of a node using
// its full path or its label.
&{/node_with_equivalent_arrays} {
  int = <2>;
};
&label_equivalent {
  string = "bar";
```

There are two things that we have already seen but haven't explicitly described throughout this article: We can use node references outside the root node to define _additional_ properties for a node or change a node's existing properties, and we can use a different syntax for specifying array values:

In the above example, the property `int` of the node `/node_with_equivalent_arrays` is overwritten with the value _2_, and a value `"bar"` is added to the node's property `string` using its label `label_equivalent`.

We also see that we don't have to provide all values of an `array` within a single set of `<..>`, but can instead place each value into its own pair of `<..>` and separate the values via commas. We also see the alternative syntax for `uint8-arrays`. In the semantics article, we'll see that the output is indeed the same, for now, you have to trust me on this.

The following is the generated output for the two nodes `/node_with_props` and `/node_with_equivalent_arrays`:

`/build/zephyr/zephyr.dts`

```dts
/ {
  node_with_props {
    existent-boolean;
    int = < 0x1 >;
    array = < 0x1 0x2 0x3 >;
    uint8-array = [ 12 34 ];
    string = "foo";
    string-array = "foo", "bar", "baz";
  };
  label_equivalent: node_with_equivalent_arrays {
    uint8-array = [ 12 34 ];
    array = < 0x1 >, < 0x2 >, < 0x3 >;
    int = < 0x2 >;
    string = "bar";
  };
};
```

The second overlay file shows the use of `path`s, `phandle`, `phandle`s, and `phandle-array`, including the alternative array syntax that we've just seen. Here, for the property `phandle-array-of-refs`, you can see how this syntax can sometimes result in a more readable Devicetree source file.

`dts/playground/props-phandles.overlay`

```dts
/ {
  label_a: node_a {
    // The value assignment for the cells is redundant in Zephyr, since
    // the binding already specifies all names and thus the size.
    #phandle-array-of-ref-cells = <2>;
  };
  label_b: node_b {
    #phandle-array-of-ref-cells = <1>;
  };
  node_refs {
    // Properties of type `path`
    path-by-path = &{/node_a};
    path-by-label = &label_a;

    // Properties of type `phandle`
    phandle-by-path = <&{/node_a}>;
    phandle-by-label = <&label_a>;

    // Array of phandle, type `phandles`
    phandles = <&{/node_a} &label_b>;
    // Array of phandles _with metadata_, type `phandle-array`
    phandle-array-of-refs = <&{/node_a} 1 2 &label_b 1>;
  };

  node_refs_equivalents {
    phandles = <&{/node_a}>, <&label_b>;
    phandle-array-of-refs = <&{/node_a} 1 2>, <&label_b 3>;
  };

  node_with_phandle {
    // It is allowed to explicitly provide the phandle's value, but the
    // DTS generator does this for us.
    phandle = <0xC0FFEE>;
  };
};
```

You can add those files to your own sources and provide both files using the `EXTRA_DTC_OVERLAY_FILE` option by separating the paths using a semicolon:

```bash
$ west build --board nrf52840dk_nrf52840 --build-dir ../build -- \
  -DEXTRA_DTC_OVERLAY_FILE="dts/playground/props-basics.overlay;dts/playground/props-phandles.overlay"
```


## Zephyr's DTS skeleton and addressing

Ok - we've seen nodes, labels, properties, and value types _including_ `phandles`, we know how Zephyr compiles the Devicetree and we've even seen the standard nodes `/aliases` and `/chosen`. Are we done yet? Almost. There's the famous _one last thing_ that is worth looking into before wrapping up on the Devicetree _basics_. Yes, these are the basics, there's more coming in the next article!

> **Note:** Technically, we're again describing **semantics** even though I've promised that we'll wait with that until the next article about _bindings_, but one of our goals was being able to read Zephyr Devicetree files. Without understanding the standard properties `#address-cells`, `#size-cells`, and `reg`, we'd hardly be able to claim that.

When [exploring Zephyr's build](#devicetree-includes-and-sources-in-zephyr), we've seen that our DTS file "include" tree ends up including a skeleton file `zephyr/dts/common/skeleton.dtsi`. The following is a stripped-down version of the full "include" tree that we've seen in the [introduction](#devicetree-includes-and-sources-in-zephyr) when building our empty application for the nRF52840 development kit:

```
nrf52840dk_nrf52840.dts
└── nordic/nrf52840_qiaa.dtsi
    └── nordic/nrf52840.dtsi
        └── arm/armv7-m.dtsi
            └── skeleton.dtsi
```

This `skeleton.dtsi` contains the minimal set of nodes and properties in a Zephyr Devicetree:

```dts
/ {
  #address-cells = <1>;
  #size-cells = <1>;
  chosen { };
  aliases { };
};
```

We've already seen the `/chosen` and `/aliases` nodes. The `#address-cells` and `#size-cells` properties look a lot like the [specifier cells](https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#specifier-cell-names-cells) we've seen when looking at [the `phandle-array` type](#path-phandles-and-phandle-array), right?

Even though they match the naming convention, their purpose, however, is a different one: They provide the necessary _addressing_ information for nodes that use a _unit-addresses_. Too many new terms in one article? Let's quickly repeat what we've already seen in the section about [node names](#node-names-unit-addresses-reg-and-labels):

- Nodes can have addressing information in their node name. Such nodes use the name format `node-name@unit-address` and **must** have the property `reg`.
- Nodes _without_ a _unit-address_ in the node name do _not_ have the property `reg`.

Let's first have a look at what we can find out about `reg` in the [DTSpec](https://www.devicetree.org/specifications/):

> Property name `reg`, value type _32-bit integer cells_ encoded as an arbitrary number of _(address, length)_ pairs.
>
> The `reg` property describes the address of the device’s resources within the address space defined by its parent [...]. Most commonly this means the offsets and lengths of memory-mapped IO register blocks [...].
>
> The value consists of _32-bit integer cells_, composed of an arbitrary number of pairs of _address and length_, `<address length>`. The number of 32-bit cells required to specify the address and length are [...] specified by the `#address-cells` and `#size-cells` properties in the parent of the device node. If the parent node specifies a value of _0_ for `#size-cells`, the length field in the value of `reg` shall be omitted.

Bit much? Let's bring up our good old `uart@40002000` node from the nRF52840's DTS include file:

`zephyr/dts/arm/nordic/nrf52840.dtsi`

```dts
#include <arm/armv7-m.dtsi>
#include "nrf_common.dtsi"

/ {
  soc {
    uart0: uart@40002000 { reg = <0x40002000 0x1000>; };
    uart1: uart@40028000 { reg = <0x40028000 0x1000>; };
  };
};
```

Using the information from the [DTSpec](https://www.devicetree.org/specifications/), we should now be able to tell for sure that `0x40002000` is the _address_ and `_0x1000` the _length_, right? Yes, but no: We've also learned that a 64-bit integer value is represented using two cells, thus `<0x40002000 0x1000>` could technically be a single 64-bit integer, and _length_ could be omitted. To be _really_ sure how `uart@40002000` is addressed, we need to look at the parent node's `#address-cells` and `#size-cells` properties. So what are those properties? Let's look them up in the [DTSpec](https://www.devicetree.org/specifications/):

> Property names `#address-cells`, `#size-cells`, value type _32-bit integer_.
>
> The `#address-cells` and `#size-cells` properties [...] describe how child device nodes should be addressed.
> - The `#address-cells` property defines the number of _32-bit integers_ used to encode the **address** field in a child node’s `reg` property.
> - The `#size-cells` property defines the number of _32-bit integers_ used to encode the **size** field in a child node’s `reg` property.
>
> The `#address-cells` and `#size-cells` properties are not inherited from ancestors in the Devicetree. [...] A DTSpec-compliant boot program shall supply `#address-cells` and `#size-cells` on **all** nodes that have children. If missing, a client program should assume a default value of _2_ for `#address-cells`, and a value of _1_ for `#size-cells`.

Ok, now we're getting somewhere: We need to look at the parent's properties to know how many cells in the `reg` property's value are used to encode the _address_ and the _length_. In the DTS file `zephyr/dts/arm/nordic/nrf52840.dtsi`, however, there are no such properties for `soc`.

`zephyr/dts/arm/nordic/nrf52840.dtsi`

```dts
/ {
  soc {
    /* Just nodes, no properties ... */
  };
};
```

So, do we need to assume the defaults defined in the [DTSpec](https://www.devicetree.org/specifications/)? We might have to, but we've learned that node properties don't all need to be provided in one place, and therefore the `#address-cells` and `#size-cells` of the `soc` might be provided by some includes.

Instead of going through all includes, we've seen that in Zephyr all includes are resolved by the preprocessor, and therefore a single Devicetree file is available in the build directory. You can therefore find the combined `soc`'s properties in `build/zephyr/zephyr.dts`. Since our include graph is rather easy to traverse, though, we'll also find the missing information in the CPU architecture's DTS file:

`zephyr/dts/arm/armv7-m.dtsi`

```dts
#include "skeleton.dtsi"

/ {
  soc {
    #address-cells = <1>;
    #size-cells = <1>;
    /* ... */
  };
};
```

Now we really can tell for sure that for our `uart@40002000` node, the `reg`'s value `<0x40002000 0x1000>` indeed refers to the _address_ `0x40002000` and the _length/size_ `0x1000`. This is also not surprising since the nRF52840 uses **32-bit architecture**.

We can also find an example of a **64-bit architecture** in Zephyr's DTS files:

`zephyr/dts/riscv/sifive/riscv64-fu740.dtsi`

```dts
/ {
  soc {
    #address-cells = <2>;
    #size-cells = <2>;

    uart0: serial@10010000 {
      reg = <0x0 0x10010000 0x0 0x1000>;
    };
  };
};
```

Here, `uart0`'s _address_ is formed by the 64-bit integer value `<0x0 0x10010000>` and the _length_ by the 64-bit integer value `<0x0 0x1000>`.

Addressing is not only used for register mapped devices but, e.g., also for _bus_ addresses. For this, let's have a look at [Nordic's Thingy:53](https://www.nordicsemi.com/Products/Development-hardware/Nordic-Thingy-53), a more complex board with several I2C peripherals connected to the nRF5340 MCU. The I2C bus of the nRF5340 uses the `#address-cells` and `#size-cells` properties to indicate that I2C nodes are uniquely identified via their address, no size is needed:

`zephyr/dts/arm/nordic/nrf5340_cpuapp_peripherals.dtsi`

```dts
i2c1: i2c@9000 {
  #address-cells = <1>;
  #size-cells = <0>;
};
```

In the _board_ DTS file, we find which I2C peripherals are actually hooked up to the I2C interface:
- The `BMM150` geomagnetic sensor uses the I2C address `0x10`,
- the `BH1749` ambient light sensor uses the I2C address `0x38`,
- and the `BME688` gas sensor the address `0x76`.

`zephyr/boards/arm/thingy53_nrf5340/thingy53_nrf5340_common.dts`

```dts
&i2c1 {
  bmm150: bmm150@10 { reg = <0x10>; };
  bh1749: bh1749@38 { reg = <0x38>; };
  bme688: bme688@76 { reg = <0x76>; };
};
```

With this, we're really done with our Devicetree introduction.


## Conclusion

In this article, we've learned what a _devicetree_ is, its most important syntax, and how it is used in Zephyr. We've seen:

- What a Devicetree actually is,
- how a Devicetree is structured and how it represents the hardware,
- all basic property types and `phandle`s, including `phandle-array`s,
- we've seen the standard nodes `/chosen` and `/aliases`,
- and we've learned about addressing within Devicetree.

Even without looking at _bindings_ yet, this is a lot to take in, especially if you've never worked with *Devicetree* before. If you feel overwhelmed, don't worry, we'll look into each property type again in the next article about _bindings_ and thus **semantics**. Practice is key!

> **Note:** The full example application including all the Devicetree files that we've seen throughout this article is available in the [`02_devicetree_basics` folder of the accompanying GitHub repository](https://github.com/lmapii/practical-zephyr/tree/main/02_devicetree_basics).

Just like _Kconfig_, in your future Zephyr projects, _Kconfig_ will become especially important once you're starting to build your own device drivers. With this article, I hope I can give you an easy start and I hope to see you in the next article about Devicetree _bindings_!



## Further reading

The following are great resources when it comes to Zephyr and is worth a read _or watch_:

- Of course, [Linaro](https://www.linaro.org/blog/introducing-devicetree-org/) deserves an honorable mention as the inventor of *Devicetree*.
- Second, of course, [Zephyr's official documentation on Devicetree](https://docs.zephyrproject.org/latest/build/dts/index.html).
- Reading or at least skimming through the [official Devicetree specification](https://www.devicetree.org/specifications/) is very highly recommended.
- Zephyr's official documentation also includes a more practical information article in the form of a [Devicetree how-tos](https://docs.zephyrproject.org/latest/build/dts/index.html).
- As usual, the [nRF Connect SDK Fundamentals](https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals) course in Nordic's [DevAcademy](https://academy.nordicsemi.com/) also covers _devicetree_.
- The official Raspberry PI documentation also has a [great section about Devicetree and the DTS syntax](https://www.raspberrypi.com/documentation/computers/configuration.html#device-trees-overlays-and-parameters).
- I can highly recommend watching the presentation [Zephyr Devicetree Mysteries, Solved](https://www.youtube.com/watch?v=w8GgP3h0M8M&list=PLzRQULb6-ipFDwFONbHu-Qb305hJR7ICe) by Martí Bolívar in case you can't wait for the next article.
- `elinux.org` has an interesting wiki page about [Devicetree mysteries](https://elinux.org/Device_Tree_Mysteries), though this is mainly targeted at Linux.

Finally, have a look at the files in the [accompanying GitHub repository](https://github.com/lmapii/practical-zephyr).



<!-- References

[practical-zephyr]: https://github.com/lmapii/practical-zephyr

[nordicsemi]: https://www.nordicsemi.com/
[nordicsemi-dev-academy]: https://academy.nordicsemi.com/
[nordicsemi-academy-devicetree]: https://academy.nordicsemi.com/topic/devicetree/
[nordicsemi-nrf52840-dk]: https://www.nordicsemi.com/Products/Development-hardware/nrf52840-dk
[nordicsemi-thingy53]: https://www.nordicsemi.com/Products/Development-hardware/Nordic-Thingy-53
[nordicsemi-academy]: https://academy.nordicsemi.com/
[nrf-connect-sdk-fundamentals]: https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals

[rpi-devicetree]: https://www.raspberrypi.com/documentation/computers/configuration.html#device-trees-overlays-and-parameters

[devicetree]: https://www.devicetree.org/
[devicetree-spec]: https://www.devicetree.org/specifications/
[linaro-devicetree]: https://www.linaro.org/blog/introducing-devicetree-org/

[zephyr-kconfig]: https://docs.zephyrproject.org/latest/build/kconfig/index.html#configuration-system-kconfig
[zephyr-dts]: https://docs.zephyrproject.org/latest/build/dts/index.html
[zephyr-dts-howto]: https://docs.zephyrproject.org/latest/build/dts/howtos.html
[zephyr-dts-bindings-api]: https://docs.zephyrproject.org/latest/build/dts/api/bindings.html
[zephyr-dts-bindings-types]: https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#type
[zephyr-dts-intro-bindings-properties]: https://docs.zephyrproject.org/latest/build/dts/intro-syntax-structure.html#properties
[zephyr-dts-intro-input-and-output]: https://docs.zephyrproject.org/latest/build/dts/intro-input-output.html
[zephyr-dts-intro-property-values]: https://docs.zephyrproject.org/latest/build/dts/intro-syntax-structure.html#writing-property-values
[zephyr-dts-phandles]: https://docs.zephyrproject.org/latest/build/dts/phandles.html
[zephyr-dts-overlays]: https://docs.zephyrproject.org/latest/build/dts/howtos.html#set-devicetree-overlays
[zephyr-dts-bindings-location]: https://docs.zephyrproject.org/latest/build/dts/bindings-intro.html#where-bindings-are-located
[zephr-dts-bindings-specifier-cells]: https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#specifier-cell-names-cells
[zephyr-dts-api-chosen]: https://docs.zephyrproject.org/latest/build/dts/api/api.html#devicetree-chosen-nodes
[zephyr-kconfig]: https://docs.zephyrproject.org/latest/build/kconfig/index.html#configuration-system-kconfig
[zephyr-dt-vs-kconfig]: https://docs.zephyrproject.org/latest/build/dts/dt-vs-kconfig.html

[zephyr-summit-22-devicetree]: https://www.youtube.com/watch?v=w8GgP3h0M8M&list=PLzRQULb6-ipFDwFONbHu-Qb305hJR7ICe

[elinux-dt-mysteries]: https://elinux.org/Device_Tree_Mysteries

-->