---
title: Practical Zephyr - Devicetree semantics (Part 4)
description: Article series "Practical Zephyr", the fourth part about Devicetree bindings.
author: lampacher
tags: [zephyr, devicetree, practical-zephyr-series]
---

<!-- excerpt start -->

Having covered the *Devicetree basics* in the [previous article]({% post_url 2024-02-01-practical_zephyr_dt %}), we now add **semantics** to our *Devicetree* using so-called _bindings_: For each supported type, we'll create a corresponding _binding_ and look at the generated output to understand how it can be used with Zephyr's Devicetree API.

<!-- excerpt end -->

👉 Find the other parts of the *Practical Zephyr* series [here](/tags#practical-zephyr-series).

Notice that we'll only look at Zephyr's _basic_ Devicetree API and won't analyze specific subsystems such as `gpio` in detail. We're saving this for a practice round in the next article of this _Practical Zephyr_ series.

> 🎬
> [Listen to Martin on Interrupt Live](https://www.youtube.com/watch?v=ls_Y45WsTiA?feature=share)
> talk about the content and motivations behind writing this series.

{% include newsletter.html %}

{% include toc.html %}

## Prerequisites

This article is part of the _Practical Zephyr_ article series. In case you haven't read the previous articles, please go ahead and have a look. In this article we're building up on what we've seen in the [Devicetree basics]({% post_url 2024-02-01-practical_zephyr_dt %}): We'll add _bindings_ to the same nodes that we've created in the [previous article]({% post_url 2024-02-01-practical_zephyr_dt %}). If you have some experience with _Devicetree_ and Zephyr, you should be able to follow along just fine without reading the previous articles.

> **Note:** A full example application including all files that we'll see throughout this article is available in the [`03_devicetree_semantics` folder of the accompanying GitHub repository](https://github.com/lmapii/practical-zephyr/tree/main/03_devicetree_semantics).

We'll again be using the [development kit for the nRF52840](https://www.nordicsemi.com/Products/Development-hardware/nrf52840-dk) as a reference and exploring its files in Zephyr, but you can follow along with any target - real or virtual.



## Warm-up

Before we get started, let's quickly review what we've seen when we had a look at the UART nodes of the [nRF52840 Development Kit from Nordic](https://www.nordicsemi.com/). We're using the same old freestanding application with an empty `prj.conf` and the following file tree:

```bash
$ tree --charset=utf-8 --dirsfirst
.
├── src
│   └── main.c
├── CMakeLists.txt
└── prj.conf
```

The `CMakeLists.txt` only includes the necessary boilerplate to create a freestanding Zephyr application. As an application, we'll again use the same old `main` function that outputs the string _"Message in a bottle."_ each time it is called, and thus each time the device starts.

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

The following command builds this application for the [nRF52840 Development Kit from Nordic](https://www.nordicsemi.com/) that we're using as a reference. As usual, you can follow along with any board (or emulation target) and you should see a similar output.

```bash
$ west build --board nrf52840dk_nrf52840 --build-dir ../build
```

Using the `--board` parameter, Zephyr selects the matching Devicetree source file `zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts`, which specifies the properties `status` and `current-speed` for the node with the label `uart0`, as follows:

`zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts`
```dts
&uart0 {
  compatible = "nordic,nrf-uarte";
  status = "okay";
  current-speed = <115200>;
  /* other properties */
};
```

Zephyr's DTS generator script produces the matching output in `devicetree_generated.h` for the specified properties:

`build/zephyr/include/generated/devicetree_generated.h`
```c
#define DT_N_S_soc_S_uart_40002000_P_current_speed 115200
#define DT_N_S_soc_S_uart_40002000_P_status "okay"
```

Can we get a similar output for Devicetree nodes that we define from scratch? In the [previous article]({% post_url 2024-02-01-practical_zephyr_dt %}), we've been using _overlay_ files to define custom nodes with their own properties to showcase Zephyr's Devicetree types. We'll see if we can get some output for the nodes and properties in these overlays, but before we go ahead and make blind use of _overlays_ yet again, let's finally have a look and learn how they work.



## Devicetree overlays

_Overlays_ are used to extend or modify the board's Devicetree source file. Even though by convention overlay files use the `.overlay` file extension, they're just plain old Devicetree source (DTS) files. The build system combines the board's `.dts` file and any `.overlay` files by concatenating them, with the overlays put last. Thus, the contents of the `.overlay` file have priority over any definitions in the board's `.dts` file or its includes.

### Automatic overlays

The Zephyr build system automatically picks up additional _overlays_ based on their location and file name. The following repeats the steps [in Zephyr's official documentation](https://docs.zephyrproject.org/latest/build/dts/howtos.html#set-devicetree-overlays) used by the build system to detect _overlay_ files.

Before we have a look at the steps, there's one detail that we haven't seen yet. So far, we've specified the board as a command-line argument in the format `--board <board>`. Zephyr also supports [building for a board revision](https://docs.zephyrproject.org/latest/develop/application/index.html#application-board-version) in case you have multiple _revisions_ or versions of a specific board. In such a case, you can use the format `--board <board>@<revision>`.

> **Note:** As we've seen in previous articles, there are several ways to specify the board, but for the sake of simplicity we're only using `west` and its `--board` argument here. You can specify the board in your path using the variable `BOARD`, you can provide it directly in your `CMakeLists.txt` file, or even pass it to an explicit `cmake` call.

With this detail out of the way, here's the search performed by the CMake module `zephyr/cmake/modules/dts.cmake`:

- In case the CMake variable `DTC_OVERLAY_FILE` is set, the build system uses the specified file(s) and stops the search.
- If the file `boards/<BOARD>.overlay` exists in the application's root directory, the build system selects the provided file as overlay and proceeds with the following step.
- If a specific revision has been specified for the `BOARD` in the format `<board>@<revision>` and `boards/<BOARD>_<revision>.overlay` exists in the application's root directory, this file is used in _addition_ to `boards/<BOARD>.overlay`, if both exist.
- If _overlays_ have been encountered in any of the previous steps, the search stops.
- If no files have been found and `<BOARD>.overlay` exists in the application's root directory, the build system uses the overlay and stops the search.
- Finally, if none of the above overlay files exist but `app.overlay` exists in the application's root directory, the build system uses the overlay.

On top of the _overlay_ files that have or haven't been discovered by the build process, the CMake variable `EXTRA_DTC_OVERLAY_FILE` allows to specify additional _overlay_ files that are added regardless of the outcome of the overlay search.

The important thing to remember is, that the Devicetree overlay files that were detected _last_ have the _highest_ precedence, since they may overwrite anything in the previously added overlay files. The precedence is always visible in the build output, where Zephr lists all overlay files using the output `Found devicetree overlay: <name>.overlay` in the order that they are detected and thus added. The precedence _increases_ with the given list.

### Overlays by example

Let's try and visualize this list using an imaginary file tree and board "dummy_board". I've annotated the files with precedence numbers, even though - as we've learned before - not all files will be used by the build:

```bash
$ tree --charset=utf-8 --dirsfirst.
.
├── boards
│   ├── dummy_board_123.overlay -- #3
│   └── dummy_board.overlay     -- #4
├── dts
│   └── extra
│       ├── extra_0.overlay -- #2
│       └── extra_1.overlay -- #1
├── src
│   └── main.c
├── CMakeLists.txt
├── app.overlay         -- #6
├── dummy_board.overlay -- #5
└── prj.conf
```

Let's assume we would use the following command that doesn't include the CMake variable `DTC_OVERLAY_FILE`:

```bash
$ west build --board dummy_board@123 -- \
  -DEXTRA_DTC_OVERLAY_FILE="dts/extra/extra_0.overlay;dts/extra/extra_1.overlay"
```

The overlay files would be detected and added as follows, depending on whether or not they exist. As mentioned, the precedence _increases_ and thus the files listed _last_ have the _highest_ precedence:

- `boards/dummy_board.overlay`
- `boards/dummy_board_123.overlay`
- if none of the previous files exist, `dummy_board.overlay`
- if none of the previous files exist, `app.overlay`
- `dts/extra/extra_0.overlay`
- `dts/extra/extra_1.overlay`

> **Note:** It is recommended to use the `boards` directory for board overlay files. You should no longer place your board's overlay files in the application's root directory.

If, instead, we specify the CMake variable `DTC_OVERLAY_FILE` as `app.overlay` in our command as follows, the automatic detection is skipped and the build process only picks the selected DTC overlay files:

```bash
$ west build --board dummy_board@123 -- \
  -DTC_OVERLAY_FILE="app.overlay" \
  -DEXTRA_DTC_OVERLAY_FILE="dts/extra/extra_0.overlay;dts/extra/extra_1.overlay"
```

The overlay files would thus be added as follows and with _increasing_ precedence:
- `app.overlay`
- `dts/extra/extra_0.overlay`
- `dts/extra/extra_1.overlay`

What does that mean in practice? If you provide the CMake variable `DTC_OVERLAY_FILE` in your build, the board overlays will no longer be picked up automatically. For quick builds that might be helpful, but in case you're building an application that should run on multiple boards, you should **not** use `DTC_OVERLAY_FILE` but maybe rather list additional overlays using `EXTRA_DTC_OVERLAY_FILE`.



## Towards bindings

Now that we finally know what _overlays_ are and how we can use them in our build, let's find out what Zephyr produces for our own overlays in its `devicetree_generated.h` file.


### Extending the example application

We start by creating our own overlay file `dts/playground/props-basics.overlay`:

```bash
$ tree --charset=utf-8 --dirsfirst .
├── dts
│   └── playground
│       └── props-basics.overlay
├── src
│   └── main.c
├── CMakeLists.txt
└── prj.conf
```

In the development kit's Devicetree source file `nrf52840dk_nrf52840.dts`, the `&uart0` node has two properties: A property `current-speed` of type `int`, and the property `status` of type `string`. We've seen that Zephyr creates some output in `devicetree_generated.h` for the UART node, so let's try the same with a custom node:

`dts/playground/props-basics.overlay`
```dts
/ {
  node_with_props {
    int = <1>;
    string = "foo";
  };
};
```

When running a build whilst specifying the path to the overlay using the CMake variable `EXTRA_DTC_OVERLAY_FILE`, we can verify that the overlay is indeed picked up by the build system, since it informs us about the overlays it found using the output `Found devicetree overlay`:

```bash
$ west build --board nrf52840dk_nrf52840 --build-dir ../build -- \
  -DEXTRA_DTC_OVERLAY_FILE="dts/playground/props-basics.overlay"
```
```
-- Found Dtc: /opt/nordic/ncs/toolchains/4ef6631da0/bin/dtc (found suitable version "1.6.1", minimum required is "1.4.6")
-- Found BOARD.dts: /opt/nordic/ncs/v2.4.0/zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts
-- Found devicetree overlay: dts/playground/props-basics.overlay
-- Generated zephyr.dts: /path/to/build/zephyr/zephyr.dts
-- Generated devicetree_generated.h: /path/to/build/zephyr/include/generated/devicetree_generated.h
-- Including generated dts.cmake file: /path/to/build/zephyr/dts.cmake
```

Checking the output file `build/zephyr/zephyr.dts` we also see that our `node_with_props` can be found in the Devicetree that Zephyr is using as input for its generator script, that in turn produces `devicetree_generated.h`:

`build/zephyr/zephyr.dts`
```dts
/ {
  /* ... */
  node_with_props {
    int = < 0x1 >;
    string = "foo";
  };
};
```

Let's see if we can find anything in `devicetree_generated.h` by searching for the quite unique value `foo` of our `string` property:

```bash
$ grep foo ../build/zephyr/include/generated/devicetree_generated.h
$
```

The search yields no results! Did we miss something? One thing that we can easily verify, is that the `devicetree_generated.h` at least contains our node: We can search using the node's full path `/node_with_props`. You should find a large comment, separating the macros generated for our node, containing a list of definitions:

`build/zephyr/include/generated/devicetree_generated.h`
```c
/*
 * Devicetree node: /node_with_props
 *
 * Node identifier: DT_N_S_node_with_props
 */

/* Node's full path: */
#define DT_N_S_node_with_props_PATH "/node_with_props"
// ---snip ---
/* (No generic property macros) */
```

The omitted lines contain lots of generic macros for our node. Don't worry, we'll skim through these soon enough. However, we won't find anything that is even remotely related to the two properties that we defined for our node. In fact, the comment _"(No generic property macros)"_ seems to hint that the generator did not encounter the properties `int` and `string` in our node `/node_with_props`.


### Understanding Devicetree macro names

Before we dig deeper, let's try to gain a better understanding of the macro names in `devicetree_generated.h`. Once we understand those, we should be able to know which macro (or macros) Zephyr should generate for our node's properties.

In Zephyr's `doc` folder, you can find the _"RFC 7405 ABNF grammar for Devicetree macros"_ `zephyr/doc/build/dts/macros.bnf`. This RFC describes the macros that are directly generated out of the Devicetree sources. In simple words, the following rules apply (we've seen some of those already in the [previous article]({% post_url 2024-02-01-practical_zephyr_dt %}), but it really can't hurt to repeat them):

- `DT` is the common prefix for Devicetree macros,
- `S` is a forward slash `/`,
- `N` refers to a _node_,
- `P` is a _property_,
- all letters are converted to lowercase,
- and non-alphanumerics characters are converted to underscores "`_`"

Let's look at our favorite `/soc/uart@40002000` node, specified in the nRF52840's DTS file, and modified by the nRF52840 development kit's DTS file:

`zephyr/dts/arm/nordic/nrf52840.dtsi`
```dts
/ {
  soc {
    uart0: uart@40002000 {
      compatible = "nordic,nrf-uarte";
      reg = <0x40002000 0x1000>;
    };
  };
};
```

`zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts`
```dts
&uart0 {
  compatible = "nordic,nrf-uarte";
  status = "okay";
  current-speed = <115200>;
  /* other properties */
};
```

Following the previous rules, we can transform the paths and property names into tokens:
- The node path `/soc/uart@40002000` is transformed to `_S_soc_S_uart_40002000`.
- The property name `current-speed` is transformed to `current_speed`.
- The property name `status` stays the same.

Since node paths are unique, by combining the node's path with its property names we can create a unique macro for each property - and that's exactly what Zephyr's Devicetree generator does. The leading `_N_` is used to indicate that it is followed by a node's path, `_P_` separates the node's path from its property. For `uart@40002000`, we get the following:

- `/soc/uart@40002000`, property `current-speed`
  becomes `N_S_soc_S_uart_40002000_P_current_speed`
- `/soc/uart@40002000`, property `status`
  becomes `N_S_soc_S_uart_40002000_P_status`

Adding the `DT_` prefix, we can indeed find those macros in `devicetree_generated`:

```bash
$ grep -sw DT_N_S_soc_S_uart_40002000_P_current_speed \
  ../build/zephyr/include/generated/devicetree_generated.h
#define DT_N_S_soc_S_uart_40002000_P_current_speed 115200

$ grep -sw DT_N_S_soc_S_uart_40002000_P_status \
  ../build/zephyr/include/generated/devicetree_generated.h
#define DT_N_S_soc_S_uart_40002000_P_status "okay"
```

For our little demo overlay we can do the same:

`dts/playground/props-basics.overlay`
```dts
/ {
  node_with_props {
    int = <1>;
    string = "foo";
  };
};
```

For `/node_with_props`' properties, the generator should create the following macros:
- `DT_N_S_node_with_props_P_int` for the property `int`,
- `DT_N_S_node_with_props_P_string` for the property `string`.

Thus, in our `devicetree_generated.h` we should be able to find the above macros - but we don't:

```bash
$ grep -sw DT_N_S_node_with_props_P_int \
  ../build/zephyr/include/generated/devicetree_generated.h

$ grep -sw DT_N_S_node_with_props_P_string \
  ../build/zephyr/include/generated/devicetree_generated.h
```

Something's still missing.


### Matching `compatible` bindings

What's the difference between `/soc/uart@40002000` and our `/node_with_props`? Admittedly, there are _lots_ of differences, but the significant difference is that `/soc/uart@40002000` has a **compatible** property. `compatible` is a standard property defined in the [DTSpec](https://www.devicetree.org/specifications/), so let's look it up:

> Property name `compatible`, value type `<stringlist>`.
>
> The `compatible` property value consists of one or more strings that define the specific programming model for the device. This list of strings should be used by a client program for device driver selection. The property value consists of a concatenated list of null-terminated strings, from most specific to most general. They allow a device to express its compatibility with a family of similar devices [...].
>
> The recommended format is `"manufacturer,model"`. [...] The compatible string should consist only of lowercase letters, digits and dashes, and should start with a letter. [...]

Let's rephrase this: `compatible` is a list of strings, where each string is essentially a reference to some _model_. The [DTSpec](https://www.devicetree.org/specifications/) uses a specific term for such models: They are called **bindings**.

The [DTSpec](https://www.devicetree.org/specifications/) defines _bindings_ as _"requirements [...] for how specific types and classes of devices are represented in the Devicetree."_ In very simple words, a binding defines the properties that a node (or its children) can or even must have, their exact type, and their **meaning**.

How is this any different from what we've been doing until now in our Devicetree source files? Well, without a _binding_, we can give a node any number of properties and can assign any property any value of any type, as long as all nodes, properties, and values are _syntactically_ sound. Also, any property name must be considered random. Let's take our own `node_with_props`:

`dts/playground/props-basics.overlay`
```dts
/ {
  node_with_props {
    int = <1>;
    string = "foo";
  };
};
```

The Devicetree compiler won't complain if we assign a `string` value to the property `int`. In fact, it doesn't even know whether or not `node_with_props` should have the `int` property at all. We could delete it and the compiler won't complain. We also don't know what the purpose of `int` or `string` is, what they mean, or what they're used for.

While the property names `int` or `string` are not very revealing as such, the same is also true for the `current-speed` property of the `/soc/uart@40002000` node in the nRF52840 Devicetree: Without any additional information, we can only _assume_ that this is the baud rate in bits/s, but we can't know for sure.

By providing _compatible bindings_, we're telling the Devicetree compiler to check whether the given node really matches the properties and types defined in a _binding_, and we're telling it what to do with the information provided in the Devicetree. Bindings also add **semantics to a Devicetree** by giving properties a _meaning_.

The [DTSpec](https://www.devicetree.org/specifications/) includes some standard bindings, e.g., bindings for serial devices such as our UART device. It thus defines how serial devices need to look like in a Devicetree. This binding includes the `current-speed` property:

| Field       | Details                                                                                                                                             |
| :---------- | :-------------------------------------------------------------------------------------------------------------------------------------------------- |
| Property    | `current-speed`                                                                                                                                     |
| Value type  | `<u32>`                                                                                                                                             |
| Description | Specifies the current speed of a serial device in bits per second. A boot program should set this property if it has initialized the serial device. |
| Example     | 115,200 Baud: `current-speed = <115200>;`                                                                                                           |

> **Note:** In case you're wondering why `current-speed` is not listed in the _"Standard Properties"_ section in the [DTSpec](https://www.devicetree.org/specifications/), but all of a sudden appears in the section about "Serial devices" in the "Device Bindings", there's a very short answer for that: `current-speed` is only relevant for certain node types. It isn't a property that is globally defined and can be applied to any kind of node.

Finally, we're getting somewhere! We now know what the `current-speed` property is all about: We know its type, its physical unit, and its _meaning_. But this is just some table in the [DTSpec](https://www.devicetree.org/specifications/), how is this information represented in Zephyr?

_Bindings_ live outside the Devicetree. The [DTSpec](https://www.devicetree.org/specifications/) doesn't specify any file format or syntax that is used for bindings. Zephyr, like [Linux](https://docs.kernel.org/devicetree/bindings/writing-schema.html), uses `.yaml` files for its bindings. I'm assuming you're familiar with `YAML` - in case you're not, have a quick look at its [online documentation](https://yaml.org/).


### Bindings in Zephyr

[Zephyr's official documentation](https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html) provides a comprehensive explanation of the syntax used for bindings. Unless you're adding your own devices and drivers, you'll hardly ever need to create bindings yourself. Therefore, in this section, we'll walk through some existing bindings in Zephyr to gain a general understanding, and then create [our own example bindings](#bindings-by-example) in the next section.

Let's have a look at what we can find out about `/soc/uart@40002000`'s `current-speed` and `status` properties in Zephyr: Via its `compatible` property, the node `/soc/uart@40002000` claims compatibility with a binding `nordic,nrf-uarte`. The comma is used to specify a _vendor prefix_ (we'll see cover this when looking at [binding names](#naming)), so we're looking for `nordic`'s model (binding) for `nrf-uarte` devices:

`zephyr/dts/arm/nordic/nrf52840.dtsi`
```dts
/ {
  soc {
    uart0: uart@40002000 {
      compatible = "nordic,nrf-uarte";
      /* ... */
    };
  };
};
```

Zephyr recursively looks for Devicetree bindings (`.yaml` files) in `zephyr/dts/bindings`. Bindings are matched against the strings provided in the `compatible` property of a node. Thus, for our UART node, Zephyr looks for a binding that matches `nordic,nrf-uarte`. Conveniently, binding files in Zephyr use the same basename as the `compatible` string, and thus we can find the correct binding by searching for a file called `nordic,nrf-uarte.yaml`, which can be found in the `serial` bindings subfolder:

`zephyr/dts/bindings/serial/nordic,nrf-uarte.yaml`
```yaml
description: Nordic nRF family UARTE (UART with EasyDMA)
compatible: "nordic,nrf-uarte"
include: ["nordic,nrf-uart-common.yaml", "memory-region.yaml"]
```

Checking the _compatible_ key, we see that it indeed matches the node's _compatible_ property. This _compatible_ key is what Zephyr really matches the Node's _compatible_ property against: In theory, the file could have a different name, but _by convention_, the filename matches what's in its _compatible_ key. Zephyr doesn't care about the convention.

Apart from _compatible_, the binding also has a textual _description_ and some *include*s. As the name suggests, the _include_ key allows to include the content of other bindings. Files are included by filename without specifying any paths or directories - Zephyr determines the search paths, one of which includes all subfolders of `zephyr/dts/bindings`. For the exact syntax, have a look at the [official documentation](https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#include).

The contents of included files are essentially merged using a recursive dictionary merge. In short: Everything that's declared in included bindings is available in the including file.

> **Note:** In case you're wondering what happens with duplicated keys and/or values, try it out based on what we'll learn in the next section, where we'll create our own bindings.

Since `nordic,nrf-uarte.yaml` doesn't seem to define anything related to our properties - just like in the [article about Kconfig]({% post_url 2024-01-24-practical_zephyr_kconfig %}) - we once again have to follow the include tree. Let's try `nordic,nrf-uart-common.yaml`:

`zephyr/dts/bindings/serial/nordic,nrf-uart-common.yaml`
```yaml
include: [uart-controller.yaml, pinctrl-device.yaml]
properties:
  # --snip--
  current-speed:
    description: |
      Initial baud rate setting for UART. Only a fixed set of baud
      rates are selectable on these devices.
    enum:
      - 1200
      - 2400
      # --snip--
      - 921600
      - 1000000
```

This must be it! There's now a key _properties_, with a child element that matches our `current-speed` property. And sure enough, the _properties_ key is used to define all [properties for nodes with the matching binding may or must contain](https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#properties).

Here, Nordic seems to restrict the allowed baud rates using an *enum*eration: We can only specify values from the given list. But how does the Devicetree compiler know the type of the property? Couldn't we also use an *enum*eration to pre-define `string`s? Yes, we could, and there's indeed something missing: A key for the property's _type_.

To find out the property's type, we need to step further down the include tree, into `uart-controller.yaml`. This is Zephyr's base model for UART controllers, which is used regardless of the actual vendor:

`zephyr/dts/bindings/serial/uart-controller.yaml`
```yaml
include: base.yaml

bus: uart

properties:
  # --snip--
  current-speed:
    type: int
    description: Initial baud rate setting for UART
  # --snip--
```

Now, we finally know that `current-speed` is of type `int` and is used to configure the initial baud rate setting for UART (though the description fails to mention that the baud rate is specified in _bits per second_).

Combining this with the information in `nordic,nrf-uart-common.yaml`, we know that we can only select from a pre-defined list of baud rates and cannot specify our own custom baud rate - at least not in the Devicetree. Given this _binding_, the Devicetree compiler now rejects any but the allowed values, and it is therefore not possible to specify a _syntactically_ correct value that is not an _integer_ of the given list.

> **Note:** `bus: uart` is a special key/value pair that allows you to associate devices with a bus system, e.g., I2C, SPI or UART. This feature is especially useful if a device supports multiple bus types, e.g., a sensor that can be connected either via SPI or I2C. This is out of the scope of this article, though, but is explained nicely in the [official documentation](https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#bus).

We now know about `current-speed`, but what about `status`? As you might have guessed, we need to take yet another step down the include tree and have a look at the `base.yml` binding. This binding contains common fields used by _all_ devices in Zephyr. Here, we do not only encounter the `status` property but also `compatible`:

`zephyr/dts/bindings/base/base.yaml`
```yaml
include: [pm.yaml]

properties:
  status:
    type: string
    description: indicates the operational status of a device
    enum:
      - "ok" # Deprecated form
      - "okay"
      - "disabled"
      - "reserved"
      - "fail"
      - "fail-sss"

  compatible:
    type: string-array
    required: true
    description: compatible strings
  # --snip--
```

`status` therefore is simply a property of type `string` with pre-defined values that can be assigned in the Devicetree. It indicates the operational status of a device, which we'll see in action the practical example in the next article. You can, however, imagine that a device with the status "disabled" won't be functioning.

While going through binding files may seem tedious, the include tree depth for bindings is usually quite limited, and once you know about the base bindings such as `base.yaml`, it typically comes down to a handful of files. There is, however, no conveniently "flattened" output file like `build/zephyr/zephyr.dts`, and therefore it is always necessary to walk through the bindings. We'll have a quick look at [Nordic's](https://www.nordicsemi.com/) plugin for _Visual Studio Code_ in the next article, but thus far you'll always need to look at the source files. The following shows the include tree of `nordic,nrf-uarte.yaml` at the time of writing:

```
nordic,nrf-uarte.yaml
├── nordic,nrf-uart-common.yaml
│   ├── uart-controller.yaml
│   │   └── base.yaml
│   │       └── pm.yaml
│   └── pinctrl-device.yaml
└── memory-region.yaml
```

There's one more important thing about `compatible` and matching bindings: If a node has more than one string in its `compatible` property, the build system looks for compatible bindings in the listed order and uses the **first** match.


### Bindings directory

Before we can go ahead and experiment using our own bindings, we need to know where to place them. Just like the `dts/bindings` in _Zephyr's_ root directory, the [build process](https://docs.zephyrproject.org/latest/build/dts/intro-input-output.html) also picks up any bindings in `dts/bindings` in the _application's_ root directory.

In contrast to overlays, however, detected bindings are not listed in the build output.



## Bindings by example

This is it, we're finally there! We'll now add bindings for our [extended example application](#extending-the-example-application) to get some generated output for our node's properties. We've just seen that we can place our bindings in the [`dts/bindings` directory](#bindings-directory).

> **Note:** A full example application including all files that we'll see throughout this article is available in the [`03_devicetree_semantics` folder of the accompanying GitHub repository](https://github.com/lmapii/practical-zephyr/tree/main/03_devicetree_semantics).


### Naming

Before we start, we need to solve one of the hardest problems in engineering: Finding a good _name_ for our bindings. The [Devicetree specification](https://www.devicetree.org/specifications/) contains a guideline on the value for the `compatible` property of a node - and therefore the name of the binding:

> "The `compatible` string should consist only of lowercase letters, digits, and dashes, and should start with a letter. A single comma is typically only used following a vendor prefix. Underscores should not be used." [DTSpec](https://www.devicetree.org/specifications/)

Let's try this with the binding name _custom,props-basic_, and thus vendor prefix _custom_. We'll follow the convention and use the binding's name as filename and create a new file `custom,props-basics.yaml` in the application's `dts/bindings` directory:

```bash
$ tree --charset=utf-8 --dirsfirst.
├── dts
│   ├── bindings
│   │   └── custom,props-basics.yaml
│   └── playground
│       └── props-basics.overlay
├── src
│   └── main.c
├── CMakeLists.txt
└── prj.conf
```

In our binding, we set the `compatible` key as `"custom,props-basics"`, and add the two properties `int` and `string` of the matching type, without providing any description:

`dts/bindings/custom,props-basics.yaml`
```yaml
description: Custom properties
compatible: "custom,props-basics"

properties:
  int:
    type: int
  string:
    type: string
```

As we've seen before, bindings define a node's properties under the key _properties_. The template given below is the simplest form for a property in a binding:

```yaml
properties:
  <property-name>:
    type: <property-type>
    # required: false -> omitted by convention if false
```

Properties have a _name_ and are therefore unique within the node, and each property is assigned a _type_ using the corresponding key. Other keys such as _required_ are optional.

There are several other keys and "features", e.g., it is possible to define the properties for _children_ of a node with the matching `compatible` property, but we'll only have a look at the very basics. Definitely dive into [Zephyr's official documentation](https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html) once you're through with this article!

Finally, we create a new property `compatible = "custom,props-basic"` for our existing `node_with_props` ...

`dts/playground/props-basics.overlay`
```dts
/ {
  node_with_props {
    compatible = "custom,props-basic"
    int = <1>;
    string = "foo bar baz";
  };
};
```

... and recompile:

```bash
$ rm -rf ../build
$ west build --board nrf52840dk_nrf52840 --build-dir ../build -- \
  -DEXTRA_DTC_OVERLAY_FILE="dts/playground/props-basics.overlay"
```
```
-- Found devicetree overlay: dts/playground/props-basics.overlay
node '/node_with_props' compatible 'custom,props-basics' has unknown vendor prefix 'custom'
-- Generated zephyr.dts: /path/to/build/zephyr/zephyr.dts
-- Generated devicetree_generated.h: /path/to/build/zephyr/include/generated/devicetree_generated.h
```

> **Note:** As mentioned, at the time of writing and in contrast to overlay files, bindings are not listed in the build output, not even for bindings in the application's `dts/bindings` directory.

Even though the build passes, it seems that the Devicetree compiler is not too happy about our _"custom"_ vendor prefix. Zephyr warns us here since it maintains a _"Devicetree binding vendor prefix registry"_ `zephyr/dts/bindings/vendor-prefixes.txt` to avoid name-space collisions for properties and bindings. If you're a vendor, you can of course add your name upstream, but for this article, we'll simply skip the vendor prefix and use the binding name _custom-props-basics_ instead.

```bash
$ mv dts/bindings/custom,props-basics.yaml dts/bindings/custom-props-basics.yaml
$ sed -i .bak 's/custom,/custom-/g' dts/bindings/custom-props-basics.yaml
$ sed -i .bak 's/custom,/custom-/g' dts/playground/props-basics.overlay
$ rm dts/**/*.bak

$ tree --charset=utf-8 --dirsfirst.
├── dts
│   ├── bindings
│   │   └── custom-props-basics.yaml
│   └── playground
│       └── props-basics.overlay
├── src
│   └── main.c
├── CMakeLists.txt
└── prj.conf
```

After recompiling, we can check whether the generator script has added our properties to `devicetree_generated.h`. The value _foo_ is unique enough for a quick `grep`; for our property _int_ we'll use what we've learned and expect to find some macro containing `node_with_props_P_int`. And indeed, we **finally** have our generated output!

```bash
$ grep foo ../build/zephyr/include/generated/devicetree_generated.h
#define DT_N_S_node_with_props_P_string "foo bar baz"
#define DT_N_S_node_with_props_P_string_STRING_UNQUOTED foo bar baz
#define DT_N_S_node_with_props_P_string_STRING_TOKEN foo_bar_baz
$ grep node_with_props_P_int ../build/zephyr/include/generated/devicetree_generated.h
#define DT_N_S_node_with_props_P_int 1
#define DT_N_S_node_with_props_P_int_EXISTS 1
```

We'll learn how to use those macros in the section about [Zephyr's Devicetree API](#zephyrs-devicetree-api). In this section, we just make sure that our bindings lead to some generated output for all supported types. In case you can't wait and want to have a more detailed look instead, I suggest you have a look at [Zephyr's great introduction to Devicetree bindings](https://docs.zephyrproject.org/latest/build/dts/bindings-intro.html).

> **Note:** If you're experimenting and don't see any output, make sure that the `compatible` property is set correctly. The Devicetree compiler does **not** complain in case it doesn't find a matching binding. E.g., in case you have a typo in your `compatible` property, the application builds without warnings, but `devicetree_generated.h` won't have any content for the desired properties.


### Basic types

In the previous article, we've seen all of the types supported by devicetrees in Zephyr. We'll now use the same node, but extend it by two properties called `enum-int` and `enum-string`, since *enum*erations are represented differently in bindings. In the previous article, we've seen that Zephyr's Devicetree generator ignores _value_ labels. We'll throw in two of those, named `second_value` and `string_value`, for good practice, and also add the `label_with_props` for `/node_with_props`:

`dts/playground/props-basics.overlay`
```dts
/ {
  label_with_props: node_with_props {
    compatible = "custom-props-basics";
    existent-boolean;
    int = <1>;
    array = <1 second_value: 2 3>;
    uint8-array = [ 12 34 ];
    string = string_value: "foo bar baz";
    string-array = "foo", "bar", "baz";
    enum-int = <200>;
    enum-string = "whatever";
  };
};
```

Now we need to extend our binding by adding an entry for each of the node's properties under the _properties_ key:

`dts/bindings/custom-props-basics.yaml`
```yaml
description: Custom properties
compatible: "custom-props-basics"

properties:
  existent-boolean:
    type: boolean
  int:
    type: int
    required: true
  array:
    type: array
  uint8-array:
    type: uint8-array
  string:
    type: string
  string-array:
    type: string-array
  enum-int:
    type: int
    enum:
      - 100
      - 200
      - 300
  enum-string:
    type: string
    enum:
      - "whatever"
      - "works"
```

After recompiling ...

```bash
$ rm -rf ../build
$ west build --board nrf52840dk_nrf52840 --build-dir ../build -- \
  -DEXTRA_DTC_OVERLAY_FILE="dts/playground/props-basics.overlay"
```

... we can now find macros for all of our properties in `devicetree_generated.h` for the node `/node_with_props`, and the comment block also indicates which binding was selected to generate the macros.

`build/zephyr/include/generated/devicetree_generated.h`
```c
/*
 * Devicetree node: /node_with_props
 *
 * Node identifier: DT_N_S_node_with_props
 *
 * Binding (compatible = custom-props-basics):
 *   /path/to/dts/bindings/custom-props-basics.yaml
 */

/* Node's full path: */
#define DT_N_S_node_with_props_PATH "/node_with_props"
/* --snip-- */
/* Generic property macros: */
/* --snip-- */
```

Let's have a look at the generated macros listed beyond the marker _Generic property macros_.

#### `boolean`

For the property `existent-boolean` of type `boolean`, the Devicetree generator produces the following macros:

```c
#define DT_N_S_node_with_props_P_existent_boolean 1
#define DT_N_S_node_with_props_P_existent_boolean_EXISTS 1
```

Using what we've learned in the section about [understanding Devicetree macro names](#understanding-devicetree-macro-names), we can easily understand the basename `DT_N_S_node_with_props_P_existent_boolean` of the generated macros:

`DT` is the Devicetree prefix, `N` indicates that what follows is a node's path, `S` is a forward slash `/`, and finally `P` indicates the start of a property. Thus `DT_N_S_node_with_props_P_existent_boolean` essentially translates to `node=/node_with_props`, `property=existent_boolean`.

Since the `existent-boolean` property is present in the node in our overlay, its value translates to `1`. If we'd _remove_ the property from our node, we'd end up with the following:

```c
#define DT_N_S_node_with_props_P_existent_boolean 0
#define DT_N_S_node_with_props_P_existent_boolean_EXISTS 1
```

Thus, the value of *boolean*s is `0` if the property is _false_, or `1` if it is _true_.

What about `_EXISTS`? Remember that any path or property name is transformed to its _lowercase_ form in the Devicetree macros. `_EXIST` is all uppercase, which indicates that it isn't something that we defined, but a macro that is generated for use with the [Devicetree API](#zephyrs-devicetree-api).

For properties of any type but `boolean`s, Zephyr's Devicetree generator creates a matching `_EXISTS` macro _only_ if the property _exists_ in the Devicetree. If a property is not present, no macros are generated. *Boolean*s are an exception where this macro is _always_ generated since a missing property value means that the property is set to _false_.

> **Note:** In case you're wondering if it is possible to _unset_ or delete a boolean that is defined somewhere else in the Devicetree - yes it is, and we'll try it out in the section ["deleting properties"](#deleting-properties).

#### `int`

For the property `int` of type `int` of our node `/node_with_props`, Zephyr's Devicetree generator produces the following macros:

```c
#define DT_N_S_node_with_props_P_int 1
#define DT_N_S_node_with_props_P_int_EXISTS 1
```

Unsurprisingly, for properties of type `int` the value of the macro is an _integer literal_ (in decimal format). As mentioned in the [previous section](#boolean), the macro `_EXISTS` is created for every property that _exists_ in the Devicetree. If we tried to remove the property from our Devicetree node, however, we'd get the following error when rebuilding since we specified `int` as a _required_ property in our binding:

```
...
-- Found devicetree overlay: dts/playground/props-basics.overlay
devicetree error: 'int' is marked as required in 'properties:' in /path/to/dts/bindings/custom-props-basics.yaml, but does not appear in <Node /node_with_props in '/opt/nordic/ncs/v2.4.0/zephyr/misc/empty_file.c'>
```

If we'd remove `required: true` from the binding file _and_ delete the node's property in the overlay, _both_ macros would indeed be removed from `devicetree_generated.h`; any search for `N_S_node_with_props_P_int` would fail.

> **Note:** Knowing how macros work in `C`, you might be curious why Zephyr _removes_ the `_EXISTS` macro instead of defining its value to _0_. After all, without using the compile time switches `#ifdef` or `#if defined()` you can't check a macro's value if the macro is not defined - or can you? Turns out there is a neat trick that allows to do this, and Zephyr makes use of it, but we won't go into detail about this trick in this article. If you still want to know how this works, have a look at the documentation of the `IS_ENABLED` macro in `zephyr/include/zephyr/sys/util_macro.h` and the macros it expands to. It is explained nicely in the macros' documentation!

> **Note:** In case you specify the value of a node's property of type `int` in the Devicetree in the _hexadecimal_ format, at the time of writing the integer literal is converted to its _decimal_ value in `devicetree_generated.h`.

#### `array` and `uint8-array`

The following macros are produced for our `array` property of type `array`:

```c
#define DT_N_S_node_with_props_P_array {10 /* 0xa */, 11 /* 0xb */, 12 /* 0xc */}
#define DT_N_S_node_with_props_P_array_IDX_0 10
#define DT_N_S_node_with_props_P_array_IDX_0_EXISTS 1
#define DT_N_S_node_with_props_P_array_IDX_1 11
#define DT_N_S_node_with_props_P_array_IDX_1_EXISTS 1
#define DT_N_S_node_with_props_P_array_IDX_2 12
#define DT_N_S_node_with_props_P_array_IDX_2_EXISTS 1
/* array_FOREACH_ ... */
#define DT_N_S_node_with_props_P_array_LEN 3
#define DT_N_S_node_with_props_P_array_EXISTS 1
```

For properties of the type `array` and `uint8-array`, the Devicetree generator produces _initializer expressions_ in braces, whose elements are integer literals. In the [Devicetree API section](#zephyrs-devicetree-api), we'll use those expressions as, well, initialization values for our variables or constants. The macro with the suffix `_LEN` defines the number of elements in the array.

For each element and its position _n_ within the array, the generator also produces the macros `_IDX_n` and `_IDX_n_EXISTS`. In addition, several `_FOREACH` macros (hidden in the snippet) are generated that expand an expression for each element in the array _at compile time_.

For the node's `uint8-array` property with the corresponding type, a similar set of macros is generated. Only the `_FOREACH` macros are slightly different, but that doesn't concern us right now (in case you're still curious, check out the documentation of `DT_FOREACH_PROP_ELEM` in the [Zephyr's Devicetree API documentation](https://docs.zephyrproject.org/latest/build/dts/api/api.html)).

```c
#define DT_N_S_node_with_props_P_uint8_array {18 /* 0x12 */, 52 /* 0x34 */}
#define DT_N_S_node_with_props_P_uint8_array_IDX_0 18
#define DT_N_S_node_with_props_P_uint8_array_IDX_0_EXISTS 1
#define DT_N_S_node_with_props_P_uint8_array_IDX_1 52
#define DT_N_S_node_with_props_P_uint8_array_IDX_1_EXISTS 1
/* --snip-- uint8_array_FOREACH_ ... */
#define DT_N_S_node_with_props_P_uint8_array_LEN 2
#define DT_N_S_node_with_props_P_uint8_array_EXISTS 1
```

Just like for our `int` property, if we'd remove the `array` or `uint8-array` property from our node in the Devicetree, no macros (not even the `_EXISTS` macros) are generated for the property.

#### `string`

The following is a snipped of our `devicetree_generated.h` for the property `string`:

```c
#define DT_N_S_node_with_props_P_string "foo bar baz"
#define DT_N_S_node_with_props_P_string_STRING_UNQUOTED foo bar baz
#define DT_N_S_node_with_props_P_string_STRING_TOKEN foo_bar_baz
#define DT_N_S_node_with_props_P_string_STRING_UPPER_TOKEN FOO_BAR_BAZ
/* --snip-- string_FOREACH_ ... */
#define DT_N_S_node_with_props_P_string_EXISTS 1
```

For properties of the type `string`, the Devicetree generator produces _string literals_. The generator also produces macros with a special suffix:

- `_STRING_UNQUOTED` contains the string literals without quotes and thus all values as _tokens_.
- `_STRING_TOKEN` produces a single token out of the string literals. Special characters and spaces are replaced by underscores.
- `_STRING_UPPER_TOKEN` produces the same token as `_STRING_TOKEN`, but in uppercase letters.

In addition, the generator also produces `_FOREACH` macros, which expand for each _character_ in the string. E.g., for our value "foo bar baz" with the string length _11_, the `_FOREACH` macro would expand _11_ times.

> **Note:** The characters used by the token in `_STRING_TOKEN` are not converted to lowercase. If we use the value _"Foo Bar Baz"_ for `string`, the generated token would be `Foo_Bar_Baz`. Only the `_STRING_UPPER_TOKEN` is always all uppercase.

A string's value as a token can be useful, e.g., when using the token to form a `C` variable or code. It is very rarely used, though (in case you're curious, try looking for `DT_INST_STRING_TOKEN` in the Zephyr repository).

Again, no macros are generated in case the property does not exist in the Devicetree.

#### `string-array`

The following macros are produced for our `string-array` property of the same-named type:

```c
#define DT_N_S_node_with_props_P_string_array {"foo", "bar", "baz"}
#define DT_N_S_node_with_props_P_string_array_IDX_0 "foo"
#define DT_N_S_node_with_props_P_string_array_IDX_0_STRING_UNQUOTED foo
#define DT_N_S_node_with_props_P_string_array_IDX_0_STRING_TOKEN foo
#define DT_N_S_node_with_props_P_string_array_IDX_0_STRING_UPPER_TOKEN FOO
#define DT_N_S_node_with_props_P_string_array_IDX_0_EXISTS 1
/* --snip-- the same IDX_n_ macros are generated for "bar" and "baz" */
/* --snip-- string_array_FOREACH_ ... */
#define DT_N_S_node_with_props_P_string_array_LEN 3
#define DT_N_S_node_with_props_P_string_array_EXISTS 1
```

`string-array`s are handled similarly to `array` and `uint8-array`: Instead of _integer_ literals, the Devicetree generator produces _initializer expressions_ in braces whose elements are _string_ literals. The macro with the suffix `_LEN` defines the number of elements in the array.

For each element and its position _n_ within the array, the generator also produces the macros `_IDX_n` like we've seen for properties of type `string` - except that the generator won't produce `_FOREACH` macros for the characters within each string literal. Instead, several `_FOREACH` macros (hidden in the snippet) are generated that expand an expression for each element in the array _at compile time_.

No macros are generated in case the property does not exist in the Devicetree.

#### `enum`

For enumerations, the generator produces the same macros as it would for the corresponding base type, e.g., `int` or `string`, and in addition it generates `_ENUM` macros that indicate the position within the enumeration (and some more for `string`s).

For our enumeration `enum-int` with the allowed values _100_, _200_, and _300_, the generator produced the following macros for the selected value _200_:

```c
#define DT_N_S_node_with_props_P_enum_int 200
#define DT_N_S_node_with_props_P_enum_int_ENUM_IDX 1
#define DT_N_S_node_with_props_P_enum_int_EXISTS 1
```

Thus, in addition to the macros that would also have been generated if the property was a plain `int`, the generator produced an additional `_ENUM` macro indicating the index of the selected value within the enumeration.

The following is produced for `enum-string` with the allowed values _"whatever"_ and _"works"_, for the selected value _"whatever"_:

```c
#define DT_N_S_node_with_props_P_enum_string "whatever"
#define DT_N_S_node_with_props_P_enum_string_STRING_UNQUOTED whatever
#define DT_N_S_node_with_props_P_enum_string_STRING_TOKEN whatever
#define DT_N_S_node_with_props_P_enum_string_STRING_UPPER_TOKEN WHATEVER
#define DT_N_S_node_with_props_P_enum_string_ENUM_IDX 0
#define DT_N_S_node_with_props_P_enum_string_ENUM_TOKEN whatever
#define DT_N_S_node_with_props_P_enum_string_ENUM_UPPER_TOKEN WHATEVER
/* --snip-- enum_string_FOREACH_ ... */
#define DT_N_S_node_with_props_P_enum_string_EXISTS 1
```

Here, the generator produced the same macros that we'd get if `enum-string` was a plain `string` (including the character-based `_FOREACH` macros), and again we get the index of the chosen value within the enumeration. In addition, the generator produces `TOKEN` and `TOKEN_UPPER` macros with an `ENUM` prefix - to make the tokens accessible using the Devicetree API macros for enumerations.


### Labels and paths

Before we move on to properties using `phandle` types, let's have a look at labels and paths. In the previous article, we've seen that the [Devicetree specification](https://www.devicetree.org/specifications/) allows _labels_ not only to refer to _nodes_ but also to other positions in the Devicetree, e.g., in front of property _values_. In our example, we have three labels:

- One label `label_with_props` for our node `/node_with_props`,
- Two labels `second_value` and `string_value` for some property values.

`dts/playground/props-basics.overlay`
```dts
/ {
  label_with_props: node_with_props {
    compatible = "custom-props-basics";
    /* ... */
    array = <1 second_value: 2 3>;
    /* ... */
    string = string_value: "foo bar baz";
    string-array = "foo", "bar", "baz";
    /* ... */
  };
};
```

Zephyr's DTS generator accepts this input and - as we've seen - won't complain about the labels `second_value` and `string_value` referring to the property values. However, Zephyr simply ignores any label that doesn't refer to a _node_ and therefore doesn't generate anything usable for the property labels. Only the _node_ label `label_with_props` has a macro:

```bash
$ grep second_value ../build/zephyr/include/generated/devicetree_generated.h
$ grep string_value ../build/zephyr/include/generated/devicetree_generated.h
$ grep label_with_props ../build/zephyr/include/generated/devicetree_generated.h
#define DT_N_NODELABEL_label_with_props DT_N_S_node_with_props
```

> **Note:** In case you want some practice, look at the [Devicetree specification](https://www.devicetree.org/specifications/) and try to use references to value labels in your Devicetree. I promise it won't work either.

The following macros are generated to represent a node and its labels:

`build/zephyr/include/generated/devicetree_generated.h`
```c
/*
 * Devicetree node: /node_with_props
 *
 * Node identifier: DT_N_S_node_with_props
 * --snip--
 */
/* Node's full path: */
#define DT_N_S_node_with_props_PATH "/node_with_props"
/* Node's name with unit-address: */
#define DT_N_S_node_with_props_FULL_NAME "node_with_props"
// --snip--
/* Existence and alternate IDs: */
#define DT_N_S_node_with_props_EXISTS 1
#define DT_N_INST_0_custom_props_basics DT_N_S_node_with_props
#define DT_N_NODELABEL_label_with_props DT_N_S_node_with_props
```

The _node identifier_ that is used for our `/node_with_props` is `DT_N_S_node_with_props`, which is exactly what we'd expect knowing the node's path and from what we've learned about [understanding Devicetree macro names](#understanding-devicetree-macro-names). This _node identifier_ is itself just a _token_: No macro with the name `DT_N_S_node_with_props` exists!

_Identifiers_ are used by the macros of the [Devicetree API](#zephyrs-devicetree-api). As we'll see, these macros essentially do nothing but paste together different _tokens_. E.g., accessing a node's property's value, all we need to do is paste together _tokens_:

- Using the node identifier `DT_N_S_node_with_props`,
- and knowing that `_P_` is used for property names,
- we end up with `DT_N_S_node_with_props_P_int` for the property `int`.

In turn, a _label_ is nothing else than a different name for our node's identifier, as we can see in the above snippet of `devicetree_generated.h`. Labels use the format `DT_N_NODELABEL_<label_in_lowercase>` as the macro name and the node's identifier as the value.

In addition, the generator also produces macros with the suffix `_PATH` and `_FULL_NAME` that contain the node's full path and name including the unit address as string literals.

#### `/aliases` and `/chosen`

In the previous article about Devicetree basics, we've also seen the standard nodes `/aliases` and `/chosen`, defined by the [Devicetree specification](https://www.devicetree.org/specifications/). These nodes allow referring to a node using a _phandle_ (either using a reference to its label or the `${/full/path}` syntax), or the node's full path as a plain string. Let's try this out in our overlay file:

`dts/playground/props-basics.overlay`
```dts
/ {
  aliases {
    alias-by-label = &label_with_props;
    alias-by-path = &{/node_with_props};
    alias-as-string = "/node_with_props";
  };

  chosen {
    chosen-by-label = &label_with_props;
    chosen-by-path = &{/node_with_props};
    chosen-as-string = "/node_with_props";
  };
  /* ... previous content ... */
};
```

The node's aliases are translated to macros with the format `DT_N_ALIAS_<alias_in_lowercase>` in the node's section _"Existence and alternate IDs"_ that we've seen just before. Looking at the generated output it is now also clear why Zephyr makes no clear distinction between the terms _"references"_ and _"phandle"_:

```c
/* Existence and alternate IDs: */
#define DT_N_S_node_with_props_EXISTS 1
#define DT_N_ALIAS_alias_by_label       DT_N_S_node_with_props
#define DT_N_ALIAS_alias_by_path        DT_N_S_node_with_props
#define DT_N_ALIAS_alias_as_string      DT_N_S_node_with_props
#define DT_N_INST_0_custom_props_basics DT_N_S_node_with_props
#define DT_N_NODELABEL_label_with_props DT_N_S_node_with_props
```

Whether we reference a node using its label, a path reference, or its path as a string, it all translates to the node's _identifier_ token. The only difference is [the Devicetree API macro](#zephyrs-devicetree-api) that we use to obtain this token (or _node identifier_), as we'll see later.

Finally, `/chosen` nodes are listed in their own output section in `devicetree_generated.h`. The only difference to `/aliases` is, that each macro also gets its own `_EXISTS` pair:

```c
/*
 * Chosen nodes
 */
// --snip--
#define DT_CHOSEN_chosen_by_label         DT_N_S_node_with_props
#define DT_CHOSEN_chosen_by_label_EXISTS  1
#define DT_CHOSEN_chosen_by_path          DT_N_S_node_with_props
#define DT_CHOSEN_chosen_by_path_EXISTS   1
#define DT_CHOSEN_chosen_as_string        DT_N_S_node_with_props
#define DT_CHOSEN_chosen_as_string_EXISTS 1
```


### Phandles

Having seen labels and the `/chosen` and `/aliases` nodes, we can guess what Zephyr's output for the types `phandle` and `phandles` is - maybe even `path` and `phandle-array` is. Nevertheless, we'll walk through the types using a similar example to the one we've used in the previous article.

#### `path`, `phandle`, and `phandles`

Let's get started by creating two new files `custom-props-phandles.yaml` and `props-phandles.overlay`:

```bash
$ tree --charset=utf-8 --dirsfirst.
├── dts
│   ├── bindings
│   │   ├── custom-props-basics.yaml
│   │   └── custom-props-phandles.yaml
│   └── playground
│       ├── props-basics.overlay
│       └── props-phandles.overlay
├── src
│   └── main.c
├── CMakeLists.txt
└── prj.conf
```

Our Devicetree overlay is pretty much identical to what we've used in the previous article, but for now, we're skipping `phandle-array` properties. Thus, we're creating properties of type `path`, `phandle`, and `phandles`:

```dts
/ {
  label_a: node_a { /* Empty. */ };
  label_b: node_b { /* Empty. */ };

  node_refs {
    compatible = "custom-props-phandles";

    path-by-path = &{/node_a};
    path-by-label = &label_a;

    phandle-by-path = <&{/node_a}>;
    phandle-by-label = <&label_a>;

    // Simple array of phandles
    phandles = <&{/node_a} &label_b>;
  };
};
```

The _compatible_ binding has the following contents:

```yaml
description: Custom properties
compatible: "custom-props-phandles"

properties:
  path-by-path:
    type: path
  path-by-label:
    type: path
  phandle-by-path:
    type: phandle
  phandle-by-label:
    type: phandle
  phandles:
    type: phandles
```

In the build, we could just switch out our overlay and ignore `props-basics.overlay`, but let's include both of them instead, remembering that we need to separate the paths by semicolon (or a space):

```bash
$ rm -rf ../build
$ west build --board nrf52840dk_nrf52840 --build-dir ../build -- \
  -DDTC_OVERLAY_FILE="dts/playground/props-basics.overlay;dts/playground/props-phandles.overlay"
```

The build succeeds without any problems and both overlays are detected. In the `devicetree_generated.h` we now find all of our new nodes, but we're especially interested in `node_refs` which is using our new binding `custom-props-phandles.yaml`:

```c
/*
 * Devicetree node: /node_refs
 *
 * Node identifier: DT_N_S_node_refs
 *
 * Binding (compatible = custom-props-phandles):
 *   /path/to/dts/bindings/custom-props-phandles.yaml
 */
// --snip--
/* Generic property macros: */
// --snip--
```

The properties `path-by-path` and `path-by-label` both have the type `path`. We might therefore expect a similar output that we got when specifying paths in the `/chosen` node, namely node identifiers. However, the only output we get for those nodes is the following:

```c
#define DT_N_S_node_refs_P_path_by_path_EXISTS 1
#define DT_N_S_node_refs_P_path_by_label_EXISTS 1
```

So basically, the `type: path` exists in Zephyr, but at the time of writing does not produce any output - except for the `_EXISTS` macros, which are useless without any additional macros. This may seem surprising but makes a lot of sense recalling that Zephyr's Devicetree is not compiled into a Devicetree blob `dtb`, but is instead used or resolved at _compile time_. Paths are of no use since nodes exist only as _node identifiers_ and are therefore nothing but _tokens_ that are used to access a node's properties. At the time of writing, there is in fact not a single binding in Zephyr that uses a `path`.

Let's move on to the generated output for the properties `phandle-by-path` and `phandle-by-label`:

```c
#define DT_N_S_node_refs_P_phandle_by_path DT_N_S_node_a
#define DT_N_S_node_refs_P_phandle_by_path_IDX_0 DT_N_S_node_a
#define DT_N_S_node_refs_P_phandle_by_path_IDX_0_PH DT_N_S_node_a
#define DT_N_S_node_refs_P_phandle_by_path_IDX_0_EXISTS 1
#define DT_N_S_node_refs_P_phandle_by_path_LEN 1
#define DT_N_S_node_refs_P_phandle_by_path_EXISTS 1

#define DT_N_S_node_refs_P_phandle_by_label DT_N_S_node_a
#define DT_N_S_node_refs_P_phandle_by_label_IDX_0 DT_N_S_node_a
#define DT_N_S_node_refs_P_phandle_by_label_IDX_0_PH DT_N_S_node_a
#define DT_N_S_node_refs_P_phandle_by_label_IDX_0_EXISTS 1
#define DT_N_S_node_refs_P_phandle_by_label_LEN 1
#define DT_N_S_node_refs_P_phandle_by_label_EXISTS 1
```

Zephyr's generator translated both property values into the node's identifier, just like we've seen for the `/aliases` and `/chosen` nodes. The format in which the property's value is provided in the Devicetree does not matter for the generated output - which is of the same type `phandle`.

It is a bit surprising, though, that Zephyr also generated macros containing `_IDX_0` and the macro with the suffix `_LEN`: After all, `phandle`-typed properties only support a single value (try adding more!). The reason for this becomes clear when looking at the output for our property `phandles` of the same-named type:

```c
#define DT_N_S_node_refs_P_phandles_IDX_0 DT_N_S_node_a
#define DT_N_S_node_refs_P_phandles_IDX_0_PH DT_N_S_node_a
#define DT_N_S_node_refs_P_phandles_IDX_0_EXISTS 1
#define DT_N_S_node_refs_P_phandles_IDX_1 DT_N_S_node_b
#define DT_N_S_node_refs_P_phandles_IDX_1_PH DT_N_S_node_b
#define DT_N_S_node_refs_P_phandles_IDX_1_EXISTS 1
/* --snip-- phandles_FOREACH */
#define DT_N_S_node_refs_P_phandles_LEN 2
#define DT_N_S_node_refs_P_phandles_EXISTS 1
```

Thus, the generated output for `phandle` and `phandles` is essentially handled in the same way. The only difference is, that for `phandles` _only_ the `_IDX_n` macros are generated, and thus the _node identifiers_ can only be accessed by index, whereas for the `phandle` type it is possible to retrieve the node's identifier using the node identifier and property name. We'll see this when we'll access _phandles_ via [the Devicetree API in a later section](#zephyrs-devicetree-api).

In contrast to `array`, `uint8-array`, and `string-array`, no _initializer expression_ is generated. Makes sense: _phandles_ are not _values_ but only _tokens_ and thus cannot be used as such by an application.

In summary: `path`s practically don't exist in Zephyr, `phandle` and `phandles` essentially produce the same output, and all Devicetree references and paths translate to node _identifiers_ - which are just _tokens_.

#### `phandle-array`

Let's briefly recall why we need a `phandle-array`: In the previous article, we've seen how `gpios` are represented by the nRF52840's Devicetree (for a detailed explanation, please read the corresponding section in the previous article):

##### `phandle-array` in Zephyr

`zephyr/dts/arm/nordic/nrf52840.dtsi`
```dts
/ {
  soc {
    gpio0: gpio@50000000 {
      compatible = "nordic,nrf-gpio";
      #gpio-cells = <2>;
      port = <0>;
    };

    gpio1: gpio@50000300 {
      compatible = "nordic,nrf-gpio";
      #gpio-cells = <2>;
      port = <1>;
    };
  };
};
```

Each instance handles _all_ pins in the corresponding port. However, some other Devicetree node, e.g., an LED, needs a way to control and configure a single pin of a given port. We can't do this using a normal _phandle_, e.g., `<&gpio0>`, we need a mechanism to pass some _parameters_ or _metadata_ along with the _phandle_. `phandle-array`s implement this exact use case, allowing to pass a predefined number of cells with the _phandle_, e.g., as we can see in the nRF52840's development kit DTS file for `/leds/led_0`:

`zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts`
```dts
/ {
  leds {
    led0: led_0 {
      gpios = <&gpio0 13 GPIO_ACTIVE_LOW>;
    };
  };
};
```

What we haven't seen yet, is how we can know that the first cell _13_ is the _pin_ within `gpio0`, and the second cell _GPIO_ACTIVE_LOW_ is a configuration _flag_. This is part of the _semantics_ defined by the _binding_ of the _gpio_ nodes that are compatible with `nordic,nrf-gpio`:

`zephyr/dts/bindings/gpio/nordic,nrf-gpio.yaml`
```yaml
description: NRF5 GPIO node
compatible: "nordic,nrf-gpio"
# --snip--

gpio-cells:
  - pin
  - flags
```

Here we see that `gpio-cells` consists of two items `pin` and `flags`, which are matched exactly against the provided metadata in the given order. Thus, _13_ refers to the `pin` and `GPIO_ACTIVE_LOW` to its `flags`.

You may have noticed that the `gpio` nodes do not conform to the naming convention that we've seen in the last article. This makes it a bit awkward to explain what needs to be provided for a `phandle-array` and the referenced nodes. Therefore, we'll build our own from scratch.

> **Note:** Have a look at [Zephyr's documentation about specifier cells](https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#specifier-cell-names-cells) in case you want to know how `gpios` opts out of the mentioned naming convention.

##### A `phandle-array` from scratch

Let's briefly review the rules for using `phandle-array`s that we've seen in the previous article. Notice that there's no need to understand them out of context, we'll see all of them in action in our example:

- By convention, a `phandle-array` property is plural and its name must thus end in _s_.
- The value of a `phandle-array` property is an array of phandles, but each phandle is followed by the pre-defined number of cells for each referenced node.
- The number of cells that can follow a node's reference is specified by the node's _specifier cells_ property
- The _specifier cells_ property has a defined naming convention: The name is formed by removing the plural '_s_' and attaching '_-cells_' to the name of the `phandle-array` property: `#<name-without-s>-cells`

Let's extend our `props-phandles.overlay` with a new property `phandle-array-of-refs`. Within this property, we'll reference `/node_a` and pass along two cells, and `/node_b` with just one cell as metadata:

`dts/playground/props-phandles.overlay`
```dts
/ {
  label_a: node_a { /* Empty. */ };
  label_b: node_b { /* Empty. */ };
  node_refs {
    compatible = "custom-props-phandles";
    /* --snip-- */
    phandle-array-of-refs = <&{/node_a} 1 2 &label_b 1>;
  };
};
```

Within our binding for `custom-props-phandles`, we can now add the property `phandle-array-of-refs` of type `phandle-array`:

`dts/bindings/custom-props-phandles.yaml`
```yaml
description: Custom properties
compatible: "custom-props-phandles"

properties:
  # --snip--
  phandle-array-of-refs:
    type: phandle-array
```

With that, we fulfill our first rule, namely that our property name ends with an 's'. If we violate this rule, e.g., by using `phandle-array-of-ref` as the property name, the Devicetree compiler would reject the binding with the following error:

```
-- Found devicetree overlay: dts/playground/props-phandles.overlay
devicetree error: 'phandle-array-of-ref' in 'properties:'
in /path/to/dts/bindings/custom-props-phandles.yaml has type 'phandle-array'
and its name does not end in 's', but no 'specifier-space' was provided.
```

The mentioned _specifier-space_ is a way to work around the plural 's' rule and is used for properties that would otherwise result in weird names. We won't explain the workings of _specifier-space_ in this article, so I'll leave you with a reference to [Zephyr's documentation on specifier-space](https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#specifier-space) in case you want to know how this works in detail.

For now, let's revert the name change and use `phandle-array-of-refs` as the property name and try to compile the project (the command didn't change). Now the Devicetree compiler complains that our _specifier-cells_ are missing:

```
-- Found devicetree overlay: dts/playground/props-phandles.overlay
devicetree error: <Node /node_a in '/opt/nordic/ncs/v2.4.0/zephyr/misc/empty_file.c'>
lacks #phandle-array-of-ref-cells
```

Looking at our value assignment `phandle-array-of-refs = <&{/node_a} 1 2 &label_b 1>;` this error makes perfect sense since the compiler has no way of knowing how many _cells_ are supposed to follow each node's reference. We need to update our overlay with the required _specifier cells_ property for each referenced node:

`dts/playground/props-phandles.overlay`
```dts
/ {
  label_a: node_a { #phandle-array-of-ref-cells = <2>; };
  label_b: node_b { #phandle-array-of-ref-cells = <1>; };
  node_refs {
    /* --snip-- */
    phandle-array-of-refs = <&{/node_a} 1 2 &label_b 1>;
  };
};
```

Now the Devicetree compiler knows, that any reference to `/node_a` in a property of type `phandle-array` must be followed by exactly _2_ cells, whereas a reference to `/node_b` only expects one cell. Does this do the trick? Let's try to recompile the example:

```
-- Found devicetree overlay: dts/playground/props-phandles.overlay
devicetree error: phandle-array-of-ref controller
<Node /node_a in '/opt/nordic/ncs/v2.4.0/zephyr/misc/empty_file.c'>
for <Node /node_refs in '/opt/nordic/ncs/v2.4.0/zephyr/misc/empty_file.c'>
lacks binding
```

For any node referenced in a `phandle-array`, Zephyr requires a binding since it otherwise isn't able to generate the required macros. Without the bindings for our referenced nodes, Zephyr _could_ have been able to generate a partial set of macros, but thankfully it was decided that such incomplete bindings are rejected entirely.

Since the number of required cells in our nodes is different, we need _two separate_ bindings. We'll again use the binding name as a filename and create two files `dts/bindings/custom-cells-a.yaml` and `dts/bindings/custom-cells-b.yaml`:

```bash
$ tree --charset=utf-8 --dirsfirst.
├── dts
│   ├── bindings
│   │   ├── custom-cells-a.yaml
│   │   ├── custom-cells-b.yaml
│   │   ├── custom-props-basics.yaml
│   │   └── custom-props-phandles.yaml
│   └── playground
│       ├── props-basics.overlay
│       └── props-phandles.overlay
├── src
│   └── main.c
├── CMakeLists.txt
└── prj.conf
```

Within the binding files, as usual, we declare compatibility using `compatible` key. We might expect that `#phandle-array-of-ref-cells` are placed under the _properties_ key in the binding. They are, however, [top-level keys](https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#top-level-keys) and give a [_name_](https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#specifier-cell-names-cells) to each cell that follows the reference:

`dts/bindings/custom-cells-a.yaml`
```yaml
description: Dummy for matching "cells"
compatible: "custom-cells-a"

phandle-array-of-ref-cells:
  - name-of-cell-one
  - name-of-cell-two
```

`dts/bindings/custom-cells-a.yaml`
```yaml
description: Dummy for matching "cells"
compatible: "custom-cells-b"

phandle-array-of-ref-cells:
  - name-of-cell-one
```

The above binding thus defines a _name_ and **semantics** for each value that follows the corresponding _phandle_ when used in a `phandle-array`, just like we've seen for the `gpios` example and the cells `pin` and `flags`. E.g., for `phandle-array-of-refs = <&{/node_a} 1 2 &label_b 1>;`, we now know that:
- For `/node_a`, the value _1_ is assigned to a cell named `name-of-cell-one` and the value _2_ to a cell named `name-of-cell-two`;
- for `/node_a`, the cell `name-of-cell-one` is assigned the value _1_.

We're therefore providing the means for the [Devicetree API](#zephyrs-devicetree-api) to access the _metadata_ passed with the _phandle_ using a descriptive name.

> **Note:** One thing that is not immediately clear is, that number of elements in the `-cells` key of the _binding_ **must** match the number of `-cells = <n>` specified in the _Devicetree_. It is, e.g., **not** possible to provide _more_ elements in a binding's list and restrict the exact number of cells using the matching Devicetree property. Thus, for any node that has a binding, the property `-cells` in the Devicetree is actually redundant.

We can now go ahead and assign the bindings to our nodes with the `compatible` property, and recompile the project.

`dts/playground/props-phandles.overlay`
```dts
/ {
  label_a: node_a {
    compatible = "custom-cells-a";
    #phandle-array-of-ref-cells = <2>;
  };
  label_b: node_b {
    compatible = "custom-cells-b";
    #phandle-array-of-ref-cells = <1>;
  };
  node_refs {
    compatible = "custom-props-phandles";
    /* --snip-- */
    phandle-array-of-refs = <&{/node_a} 1 2 &label_b 1>;
  };
};
```

For our node `/node_refs`, Zephyr's generator now produces the following macros:

```c
#define DT_N_S_node_refs_P_phandle_array_of_refs_IDX_0_EXISTS 1
#define DT_N_S_node_refs_P_phandle_array_of_refs_IDX_0_PH DT_N_S_node_a
#define DT_N_S_node_refs_P_phandle_array_of_refs_IDX_0_VAL_name_of_cell_one 1
#define DT_N_S_node_refs_P_phandle_array_of_refs_IDX_0_VAL_name_of_cell_one_EXISTS 1
#define DT_N_S_node_refs_P_phandle_array_of_refs_IDX_0_VAL_name_of_cell_two 2
#define DT_N_S_node_refs_P_phandle_array_of_refs_IDX_0_VAL_name_of_cell_two_EXISTS 1
#define DT_N_S_node_refs_P_phandle_array_of_refs_IDX_1_EXISTS 1
#define DT_N_S_node_refs_P_phandle_array_of_refs_IDX_1_PH DT_N_S_node_b
#define DT_N_S_node_refs_P_phandle_array_of_refs_IDX_1_VAL_name_of_cell_one 1
#define DT_N_S_node_refs_P_phandle_array_of_refs_IDX_1_VAL_name_of_cell_one_EXISTS 1
/* --snip-- phandle_array_of_refs_FOREACH */
#define DT_N_S_node_refs_P_phandle_array_of_refs_LEN 2
#define DT_N_S_node_refs_P_phandle_array_of_refs_EXISTS 1
```

Unsurprisingly, the output looks a lot like what we get for `phandles`: We get a set of macros for each index _n_ `_IDX_n`, but now the length _n_ `_LEN` is not the total number of _cells_ within the array but matches the number of "tuples" of _phandles and metadata_.

Along the macro with the suffix `_PH` containing the _phandle_ and thus node's identifier, `_VAL_x` macros are generated for each cell that follows the _phandle_; the specifier name _x_ matches the name provided for each specifier in the binding.

Due to the fact that the index within a `phandle-array` does not refer to a cell's value, values for `phandle-array` properties are sometimes assigned using the alternative syntax that we've already seen in the previous article:
- `phandle-array-of-refs = <&{/node_a} 1 2>, <&label_b 1>;` is used instead of
- `phandle-array-of-refs = <&{/node_a} 1 2 &label_b 1>;`.

The generated output is identical.

Admittedly, the chosen names `name_of_cell_x` are not very descriptive, and the fact that `one` is assigned the value _1_ and `two` the value _2_ doesn't help, so let's have a quick lock at what's generated for the `gpios = <&gpio0 13 GPIO_ACTIVE_LOW>;` property of the `/leds/led0`:

```c
#define DT_N_S_leds_S_led_0_P_gpios_IDX_0_EXISTS 1
#define DT_N_S_leds_S_led_0_P_gpios_IDX_0_PH DT_N_S_soc_S_gpio_50000000
#define DT_N_S_leds_S_led_0_P_gpios_IDX_0_VAL_pin 13
#define DT_N_S_leds_S_led_0_P_gpios_IDX_0_VAL_pin_EXISTS 1
#define DT_N_S_leds_S_led_0_P_gpios_IDX_0_VAL_flags 1
#define DT_N_S_leds_S_led_0_P_gpios_IDX_0_VAL_flags_EXISTS 1
/* --snip-- gpios_FOREACH */
#define DT_N_S_leds_S_led_0_P_gpios_LEN 1
#define DT_N_S_leds_S_led_0_P_gpios_EXISTS 1
```

Here, we clearly see that we'll be using the _pin_ with the number _13_ from the selected node `/soc/gpio@50000000`, and we'll use the value `1` for its _flags_ - which is what `GPIO_ACTIVE_LOW` expands to.


### Deleting properties

In our `props-basics.overlay` example we've seen the boolean property `existent-boolean`. We've learned that a boolean is set to _true_ if it exists in the Devicetree, otherwise, it is _false_. Let's assume that a boolean property exists in some include file, how could you set this property to _false_ in your overlay? You'd somehow need to be able to tell the compiler to _remove_ the property since that is the only way to set it to _false_.

We can try this in our existing `dts/playground/props-basics.overlay`, where `node_with_props` has `existent-boolean` in its properties and thus sets its value to _true_. Without modifying `node_with_props` directly, we can _delete_ the property after the node's declaration using `/delete-property/`. We'll do this for both, `existent-boolean` and `string`, as follows:

`dts/playground/props-basics.overlay`
```dts
/ {
  label_with_props: node_with_props {
    compatible = "custom-props-basics";
    existent-boolean;
    /* --snip-- */
  };
};

&label_with_props {
  /delete-property/ existent-boolean;
  /delete-property/ string;
}
```

This leads to the same generated output as if the properties were not defined in the node at all. As we've seen, for `boolean`s the property's value is simply set to _0_. For the property `string`, no more macros are generated.

```c
#define DT_N_S_node_with_props_P_existent_boolean 0
#define DT_N_S_node_with_props_P_existent_boolean_EXISTS 1
/* No macros DT_N_S_node_with_props_P_string ... */
```


### Full example

Instead of providing the full example as text, I'll leave you with a [link to the example application `03_devicetree_semantics` on GitHub](https://github.com/lmapii/practical-zephyr/tree/main/03_devicetree_semantics). The overlays and bindings have the same name and content as used throughout the previous sections.



## Zephyr's Devicetree API

Now that we've seen both, DTS files and their bindings, we'll wrap up this article by looking at how the generated macros are accessed in an application. We'll cover the most common macros from `zephyr/include/zephyr/devicetree.h`. Those macros are the basis for most Zephyr modules, e.g., `gpio`s, that in turn provide their own Devicetree API built on top of the mentioned macros.

Except for the macros used to access a node's identifier, in practice you'll use macros provided by the corresponding subsystem. E.g., the `gpio` subsystem has its own Devicetree macros. It is nevertheless important to understand the basics, especially if you're planning to write your own drivers. In the next article, we'll see some examples of practical, subsystem-specific Devicetree APIs.

> **Note:** For details, you'll always need to refer to the [official Devicetree API documentation](https://docs.zephyrproject.org/latest/build/dts/api/api.html) and to the corresponding subsystems. The documentation is really great and has **lots** of examples. We'll cover the basics here so that you can easily navigate the documentation. There are simply too many macros to cover them all!

In the following sections, we'll be accessing the properties of the nodes that we've created throughout this article. In case you didn't follow along but still would like to try it out yourself, use [the freestanding application in the reference repository](https://github.com/lmapii/practical-zephyr/tree/main/03_devicetree_semantics). It also contains most code snippets in its `main.c` file.


### Macrobatics

Having seen the macros provided by Zephyr's generator script, it is now pretty obvious how nodes and property values are accessed by Zephyr's Devicetree API: The big "mystery" behind the Devicetree API is _token pasting_. In its simplest form, tokens are pasted or concatenated using the token-pasting operator `##`:

```c
// Let's assume we use macros with the format <node identifier>_<property>,
// e.g., a "node" with property "string_value" set to "123-test"
#define node_string_value "123-test"

// Tokens in C macros are pasted using `##`.
#define TOKEN_CAT_3(a1, a2, a3)      a1##a2##a3
#define NODE_PROP(node_id, property) TOKEN_CAT_3(node, _, property)

// The following resolves to `node_string_value` and thus to "123-test"
static const char *str = NODE_PROP(node, string_value);
```

Zephyr takes token pasting to another level in its Devicetree API, using variadic macro arguments, macro repetitions, and other neat tricks. In his presentation ["Zephyr Devicetree Mysteries, Solved"](https://www.youtube.com/watch?v=w8GgP3h0M8M&list=PLzRQULb6-ipFDwFONbHu-Qb305hJR7ICe) from the _Zephyr Development Summit 2022_, Martí Bolívar fittingly describes those mechanisms as _"macrobatics"_.

> **Note:** I highly recommend watching [his presentation](https://www.youtube.com/watch?v=w8GgP3h0M8M&list=PLzRQULb6-ipFDwFONbHu-Qb305hJR7ICe) (maybe after reading through this article), and if you're curious, digging into `util_macro.h` and `util_loops.h` in `zephyr/include/zephyr/sys`. The files are very well documented, and the macros `MACRO_MAP_CAT` (used by the `DT_PATH`) and `IS_ENABLED` (resolving the `_EXISTS` macros) are especially interesting.


### Node identifiers

Let's review how we can reference a node in the Devicetree using our `props-basics.overlay`:

`dts/playground/props-basics.overlay`
```dts
/ {
  aliases {
    alias-by-label = &label_with_props;
    alias-by-path = &{/node_with_props};
    alias-as-string = "/node_with_props";
  };

  chosen {
    chosen-by-label = &label_with_props;
    chosen-by-path = &{/node_with_props};
    chosen-as-string = "/node_with_props";
  };

  label_with_props: node_with_props {
    /* ... */
  };
};
```

> **Note:** We're deliberately skipping references to a node using `phandle` properties since those are _properties_, and properties are accessed differently, as we'll see in the next section (yes, technically node references in `/chosen` and `/aliases` are properties too, but those nodes are predefined by the [Devicetree specification](https://www.devicetree.org/specifications/)).

Having thoroughly explored Devicetree, we know that we have the following possibilities to refer to a node:
- Using the `/aliases` or `/chosen` nodes,
- using a globally unique _node label_,
- or via the node's full _path_.

In the previous article, we've learned that it is up to you to decide which way to reference nodes in your application. It typically makes sense to use `/aliases` or `/chosen` nodes, since paths and even node labels can be different for, e.g., different boards.

How can we reference nodes? We know that nodes as such do not exist in Zephyr's generated Devicetree: Nodes are translated into _tokens_ and thus do not have any value on their own. In Zephyr, these tokens are referred to as _node identifiers_ and they are used as parameters by other macros, e.g., to retrieve a property value. Such tokens are practically just prefixes for property macros.

Enough talk. Let's start with `/aliases`: Zephyr provides the `DT_ALIAS` macro to get a node's identifier via its alias:

```c
// Node identifier by /aliases node.
// DT_ALIAS(alias) = DT_N_ALIAS_ ## alias
#define NODE_PROPS_ALIAS_BY_LABEL   DT_ALIAS(alias_by_label)
#define NODE_PROPS_ALIAS_BY_PATH    DT_ALIAS(alias_by_path)
#define NODE_PROPS_ALIAS_BY_STRING  DT_ALIAS(alias_as_string)
```

The `DT_ALIAS` macro simply pastes `DT_N_ALIAS_` and the given node identifier and therefore resolves to the macros that we've already seen in the section about [`/aliases` and `/chosen`](#aliases-and-chosen). In turn, all alias macros resolve to the node identifier `DT_N_S_node_with_props`.

| API                         | Pasted macro                 | Node identifier          |
| :-------------------------- | :--------------------------- | :----------------------- |
| `DT_ALIAS(alias_by_label)`  | `DT_N_ALIAS_alias_by_label`  | `DT_N_S_node_with_props` |
| `DT_ALIAS(alias_by_path)`   | `DT_N_ALIAS_alias_by_path`   | `DT_N_S_node_with_props` |
| `DT_ALIAS(alias_as_string)` | `DT_N_ALIAS_alias_as_string` | `DT_N_S_node_with_props` |

You might have noticed that the properties in the DTS file use _dashes_, e.g., "`alias-by-label`", whereas in the code we need to replace the dashes with _underscores_ for the alias name, resulting in `alias_by_label`. We've learned this transformation in the section about [understanding Devicetree macro names](#understanding-devicetree-macro-names). The macros in the API, however, cannot change the format of their parameters and thus identifiers. Therefore, _you_ have to perform the **"lowercase-and-underscores"** transformation for all identifiers passed to Devicetree API macros:

- All letters are converted to lowercase,
- and non-alphanumeric characters are converted to underscores "`_`".

No other transformations are needed, e.g., you'll never need to provide a path with `/` replaced by `_S_` or prefix Devicetree macros yourself. The Devicetree API does that for you and provides macros for __all__ such use cases.

As mentioned before, the resulting _node identifier_ `DT_N_S_node_with_props` is only a _token_. No macro with the same name exists, the following search does not yield any results:

```bash
$ grep -E 'DT_N_S_node_with_props[ \t]' \
  ../build/zephyr/include/generated/devicetree_generated.h

```

Therefore, we cannot use a node identifier on its own anywhere in the code. Well, since it is a _token_, technically we could use it, e.g., as variable name, but that's not the point (and please don't do that). The point is, that the `C` preprocessor cannot further resolve the token, and wherever you'd use the node identifier `DT_N_S_node_with_props`, it would be placed as text (not as string literal) in code.

Moving on! Just like for `/aliases`, Zephyr provides the `DT_CHOSEN` macro to get a node's identifier via the `/chosen` node:

```c
// Node identifier by /chosen node.
// DT_CHOSEN(chosen) = DT_N_CHOSEN_ ## chosen
#define NODE_PROPS_CHOSEN_BY_LABEL  DT_CHOSEN(chosen_by_label)
#define NODE_PROPS_CHOSEN_BY_PATH   DT_CHOSEN(chosen_by_path)
#define NODE_PROPS_CHOSEN_AS_STRING DT_CHOSEN(chosen_as_string)
```

The `DT_CHOSEN` macro simply pastes `DT_N_CHOSEN_` and the given node identifier and again resolves to the macros that we've seen in the section about [`aliases` and `chosen`](#aliases-and-chosen). Just like the aliases, all chosen nodes resolve to the node identifier `DT_N_S_node_with_props`:

| API                           | Pasted macro                   | Node identifier          |
| :---------------------------- | :----------------------------- | :----------------------- |
| `DT_CHOSEN(chosen_by_label)`  | `DT_N_CHOSEN_chosen_by_label`  | `DT_N_S_node_with_props` |
| `DT_CHOSEN(chosen_by_path)`   | `DT_N_CHOSEN_chosen_by_path`   | `DT_N_S_node_with_props` |
| `DT_CHOSEN(chosen_as_string)` | `DT_N_CHOSEN_chosen_as_string` | `DT_N_S_node_with_props` |

This leaves us with node labels and retrieving a node's identifier by its path. Zephyr provides the two macros `DT_NODELABEL` and `DT_PATH` for this:

```c
// Node identifier by label.
// DT_NODELABEL(label) = DT_N_NODELABEL_ ## label
#define NODE_PROPS_BY_LABEL DT_NODELABEL(label_with_props)

// Node identifier by path.
// DT_PATH(...) = DT_N_ pasted with S_<node> for all nodes in the path.
#define NODE_PROPS_BY_PATH  DT_PATH(node_with_props)
```

The `DT_NODELABEL` is again a simple token pasting of `DT_N_NODELABEL_` and the provided _label_, resulting in the macro `DT_N_NODELABEL_label_with_props` that we've seen in the section about [labels and paths](#labels-and-paths), which again resolves to the node identifier `DT_N_S_node_with_props`.

The macro `DT_PATH` is a _variadic_ macro that allows retrieving a node's identifier using its full path. The path to a node is specified by the sequence of nodes, starting at the root node `/`. Each argument is thus a _node identifier_; the root node `/` is omitted. The Devicetree API simply pastes `DT_N` and `_S_<node>` for each node (in the "lowercase-and-underscores" form) in the path, resulting in the _nodde identifier_ `DT_N_S_node_with_props`.

> **Note:** The path `/soc/uart@40002000` is an example of a node that is two levels deep, obtained using `DT_PATH(soc, uart_40002000)`. Notice that it is **not** possible to use node labels in paths.


### Property values

Using the [node identifiers](#node-identifiers) and the property's name in the "lowercase-and-underscores" form, we can access the property's value using the macro `DT_PROP`. The macro is very straight-forward since all it needs to do is paste the node ID, separated by `_P_` with the provided property name.

> **Note:** Have a look at the documentation of `DT_PROP`. You'll find that its documentation matches with what we've seen for each type in the section about [basic types](#basic-types) (only enumerations are not explicitly mentioned in `DT_PROP`'s documentation).

In `props-basics.overlay`, we defined several properties that we'll now access using the Devicetree API:

`dts/playground/props-phandles.overlay`
```dts
/ {
  label_with_props: node_with_props {
    compatible = "custom-props-basics";
    existent-boolean;
    int = <1>;
    array = <0xA second_value: 0xB 0xC>;
    uint8-array = [ 12 34  ];
    string = string_value: "foo bar baz";
    string-array = "foo", "bar", "baz";
    enum-int = <200>;
    enum-string = "whatever";
  };
};
```


#### Simple values

An unsurprising but common pattern that you'll see in Zephr modules that use the Devicetree API are structures that map to a node's properties. This is especially prominent in device drivers that use [instance-based APIs](https://docs.zephyrproject.org/latest/build/dts/api/api.html#devicetree-inst-apis), e.g., `DT_INST_PROP`.

> **Note:** Instance-based APIs are in general recommended for device drivers. For the sake of simplicity, we've only specified individual nodes and don't use multiple instances. Refer to the [official documentation](https://docs.zephyrproject.org/latest/build/dts/api/api.html#devicetree-inst-apis) for details.

The following is an overly simplified example that retrieves all one-dimensional values from `/node_with_props`. For demonstration purposes, we're passing different macros for the node's identifier:

```c
typedef struct
{
    bool boolean_exists;
    int32_t int_value;
    char *string_value;
    int32_t enum_int_value;
    char *enum_string_value;
} values_t;

const values_t values = {
    .boolean_exists    = DT_PROP(NODE_PROPS_ALIAS_BY_LABEL, existent_boolean),
    .int_value         = DT_PROP(NODE_PROPS_ALIAS_BY_PATH, int),
    .string_value      = DT_PROP(NODE_PROPS_ALIAS_BY_STRING, string),
    .enum_int_value    = DT_PROP(NODE_PROPS_CHOSEN_BY_PATH, enum_int),
    .enum_string_value = DT_PROP(NODE_PROPS_CHOSEN_BY_LABEL, enum_string),
};

printk("values = {\n");
printk("  .boolean_exists     = %d\n", values.boolean_exists);    // = 1
printk("  .int_value          = %d\n", values.int_value);         // = 1
printk("  .string_value       = %s\n", values.string_value);      // = "foo bar baz"
printk("  .enum_int_value     = %d\n", values.enum_int_value);    // = 200
printk("  .enum_string_value  = %s\n", values.enum_string_value); // = "whatever"
printk("}\n");
```

Knowing that all of the given node identifiers resolve to `DT_N_S_node_with_props`, we can easily determine the macros and thus values pasted by `DT_PROP` that we've seen in the [section about basic types](#basic-types):

- `DT_N_S_node_with_props_P_existent_boolean`
- `DT_N_S_node_with_props_P_int`
- `DT_N_S_node_with_props_P_string`
- `DT_N_S_node_with_props_P_enum_int`
- `DT_N_S_node_with_props_P_enum_string`

For our enumerations `enum-int` and `enum-string`, additional macros such as `DT_ENUM_IDX` exist to access the index of the selected value within its list of possible values. This can be useful, e.g., to avoid string comparisons in enumerations that use the type `string`.

```c
// DT_ENUM_IDX resolves to DT_N_S_node_with_props_P_enum_int_ENUM_IDX
// "The index within 'enum_int' of the selected value '200' is 1."
printk(
    "The index within 'enum_int' of the selected value '%d' is %d.\n",
    values.enum_int_value,
    DT_ENUM_IDX(NODE_PROPS_CHOSEN_AS_STRING, enum_int));

// DT_ENUM_IDX resolves to DT_N_S_node_with_props_P_enum_string_ENUM_IDX
// "The index within 'enum_string' of the selected value 'whatever' is 0."
printk(
    "The index within 'enum_string' of the selected value '%s' is %d.\n",
    values.enum_string_value,
    DT_ENUM_IDX(NODE_PROPS_CHOSEN_AS_STRING, enum_string));
```

Enumerations and strings also allow to retrieve a _token_ matching its value. Such tokens can be used, e.g., to create unique variable names during compile time. Tokens can also be used to declare structure fields (though this is mostly done using [specifier cells](#specifier-cells)). For the sake of completeness, we'll declare a variable using the `string` property's value as a token:

```c
// Resolves to DT_N_S_node_with_props_P_string_STRING_TOKEN
// and thus the variable name foo_bar_baz.
#define STRING_TOKEN DT_STRING_TOKEN(NODE_PROPS_BY_PATH, string)

uint8_t STRING_TOKEN = 0U;
STRING_TOKEN += 1U;
printk("STRING_TOKEN = %d\n", STRING_TOKEN);
```


#### Scalar values

As we've seen in the [basic types section](#basic-types), the macros for scalar types provide initializer lists and can therefore also be assigned to variable-length arrays. In addition, a `_LEN` macro is generated, accessible via the `DT_PROP_LEN`:

```c
// cell_array = {10 /* 0xa */, 11 /* 0xb */, 12 /* 0xc */};
const uint32_t cell_array[] = DT_PROP(NODE_PROPS_BY_LABEL, array);
// bytestring = {18 /* 0x12 */, 52 /* 0x34 */};
const uint8_t bytestring[]  = DT_PROP(NODE_PROPS_BY_PATH, uint8_array);

const size_t cell_array_exp_length = DT_PROP_LEN(NODE_PROPS_BY_LABEL, array);
const size_t bytestring_exp_length = DT_PROP_LEN(NODE_PROPS_BY_LABEL, uint8_array);

if ((cell_array_exp_length != (sizeof(cell_array) / sizeof(uint32_t))) ||
    (bytestring_exp_length != (sizeof(bytestring) / sizeof(uint8_t))))
{
    // This is unreachable code, since the `_LEN` macro matches the
    // number of elements in the generated initializer list.
    printk("Something's wrong!\n");
}
```

Scalar types also provide `_FOREACH` macros. These macros are expanded for each element in the scalar type during compile time and thus apply a given macro for each element. The following shows how to use the `DT_FOREACH_PROP_ELEM` macro, which in turn uses the generated macro of the corresponding property:

```c
#define PRINTK_STRING(node_id, prop, idx)                                \
    do                                                                   \
    {                                                                    \
        printk("[%d] -- %s\n", idx, DT_PROP_BY_IDX(node_id, prop, idx)); \
    } while (0);

// This expands to one printk statement for each element in /node_with_props's
// property string-array _at compile-time_.
DT_FOREACH_PROP_ELEM(NODE_PROPS_BY_LABEL, string_array, PRINTK_STRING);
// [0] -- foo
// [1] -- bar
// [2] -- baz
```


### `phandle`s

We've seen that properties of the types [`phandle` and `phandles`](#path-phandle-and-phandles) produce the same output, with the only difference being that the output of `phandle` only generates a single set of macros. Since the values of _phandle_ macros are still just _node identifiers_, we need to make a tiny adaption for our referenced `/node_a` to show how they are used; we're adding the property `dummy-value` of type `int` in the overlay:

`dts/playground/props-phandles.overlay`
```dts
/ {
  label_a: node_a {
    compatible = "custom-cells-a";
    #phandle-array-of-ref-cells = <2>;
    dummy-value = <0xc0ffee>;
  };
  label_b: node_b {
    compatible = "custom-cells-b";
    #phandle-array-of-ref-cells = <1>;
  };

  node_refs {
    compatible = "custom-props-phandles";
    // --snip--
    phandle-by-path = <&{/node_a}>;
    phandle-by-label = <&label_a>;
    phandles = <&{/node_a} &label_b>;
    // --snip--
  };
};
```

An update of the binding is also necessary since otherwise Zephyr's generator won't produce any output:

`dts/bindings/custom-cells-a.yaml`
```yaml
description: Dummy for matching "cells"
compatible: "custom-cells-a"

properties:
  dummy-value:
    type: int
# --snip--
```

For _phandles_ from properties of type `phandle` or `phandles` it is possible to retrieve the node identifier using the `DT_PHANDLE` and `DT_PHANDLE_BY_IDX` Devicetree macros. The former can only be used for `phandle` types, whereas the latter is usable by both since both types provide the required macros including indices `_IDX`:

```c
// Identifier of /node_refs.
#define NODE_REFS DT_PATH(node_refs)

// Identifiers of /node_a and /node_b via the phandle properties in /node_refs.
#define NODE_A_PHANDLE_BY_LABEL DT_PHANDLE(NODE_REFS, phandle_by_label)
#define NODE_A_PHANDLE_BY_PATH  DT_PHANDLE(NODE_REFS, phandle_by_path)
#define NODE_A_PHANDLES         DT_PHANDLE_BY_IDX(NODE_REFS, phandle_array, 0)
#define NODE_B_PHANDLES         DT_PHANDLE_BY_IDX(NODE_REFS, phandle_array, 1)
```

The macros with the prefix `NODE_A_` resolve to `DT_N_S_node_a`, and `NODE_B_PHANDLES` to `DT_N_S_node_b`. Once you have the node's identifier, you can access its properties as usual, e.g., our newly added `dummy-value`, using the `DT_PROP` macro:

```c
uint32_t val_from_prop = DT_PROP(NODE_A_PHANDLE_BY_LABEL, dummy_value);
```

Since this is a very common use case, Zephyr's Devicetree API also contains the macros `DT_PROP_BY_PHANDLE` and `DT_PROP_BY_PHANDLE_IDX`, which allow reading a property's value without having to retrieve the node identifier using the corresponding `DT_PHANDLE` macros first - which is the recommended approach:

```c
uint32_t val_from_phandle_by_label = DT_PROP_BY_PHANDLE(NODE_REFS, phandle_by_label, dummy_value);
uint32_t val_from_phandle_by_path  = DT_PROP_BY_PHANDLE(NODE_REFS, phandle_by_path, dummy_value);
uint32_t val_from_phandles         = DT_PROP_BY_PHANDLE_IDX(NODE_REFS, phandles, 0, dummy_value);
```


### Specifier cells in `phandle-array`s

We've seen how `phandle-array`s are specified in DTS files, and how to create the [corresponding bindings](#phandle-array). Now, we finally see how to _access_ the entries in `phandle-array-of-ref-cells` in our `props-phandles.overlay`:

`dts/playground/props-phandles.overlay`
```dts
/ {
  label_a: node_a {
    compatible = "custom-cells-a";
    #phandle-array-of-ref-cells = <2>;
    // - name for cell 0: "name-of-cell-one"
    // - name for cell 1: "name-of-cell-two"
    dummy-value = <0xc0ffee>;
  };
  label_b: node_b {
    compatible = "custom-cells-b";
    #phandle-array-of-ref-cells = <1>;
    // - name for cell 0: "name-of-cell-one"
  };

  node_refs {
    compatible = "custom-props-phandles";
    // --snip--
    phandle-array-of-refs = <&{/node_a} 1 2 &label_b 1>;
  };
};
```

In the above Devicetree source we see that `phandle-array-of-refs` has two entries:
- The _phandle_ for `/node_a` is followed by two specifier cells _1_ and _2_,
- the _phandle_ for `/node_b` is followed by the specifier cell _1_.

> **Note:** Remember that in order to know where one entry starts and the next entry ends, it is necessary to check the `#<name>-cells` property of the referenced node. Typically, a _phandle_ marks the start of a new entry. The alternative syntax `<&{/node_a} 1 2>, <&label_b 1>` can be used to make the separation more visible.

Looking into the binding, we can also retrieve the _names_ of the corresponding cells:
- In the metadata for `/node_a`:
  - _1_ is the value for the cell with the name _name-of-cell-one_
  - _2_ is the value for the cell with the name _name-of-cell-two_
- In the metadata for `/node_b`:
  - _1_ is the value for the cell with the name _name-of-cell-one_

The above example is a bit unfortunate since it is very uncommon for a `phandle-array` to contain specifiers for different node types: `/node_a` and `/node_b` in the above example have nothing in common, and the specifier cell with index _0_ wouldn't even need to have the same name.

Nevertheless, we'll now squash them into the same self-defined structure using the Devicetree macros `DT_PHA_BY_IDX` and `DT_PHA_BY_IDX_OR`:

```c
typedef struct
{
    uint32_t cell_one;
    uint32_t cell_two;
} node_spec_t;

#define NODE_DT_SPEC_GET_BY_IDX(node_id, prop, idx)                            \
    {                                                                          \
        .cell_one = DT_PHA_BY_IDX(node_id, prop, idx, name_of_cell_one),       \
        .cell_two = DT_PHA_BY_IDX_OR(node_id, prop, idx, name_of_cell_two, 0), \
    }

// node_a = {.cell_one = 1, .cell_two = 2};
node_spec_t node_a = NODE_DT_SPEC_GET_BY_IDX(NODE_REFS, phandle_array_of_refs, 0);
// node_b = {.cell_one = 1, .cell_two = 0};
node_spec_t node_b = NODE_DT_SPEC_GET_BY_IDX(NODE_REFS, phandle_array_of_refs, 1);

// configure_node(node_a);
// configure_node(node_b);
```

As we can see, the _names_ of the specifier cells must be known by the application; it is not possible to access the specifier cells by their index. The above example is again a simplified version of how Devicetree APIs are used by device drivers: The node's specification is retrieved using a Devicetree macro, and the specification is then passed along to the corresponding API function.

We'll see how this works in detail in the next article, where we'll retrieve the associated `gpio` using `GPIO_DT_SPEC_GET`, and pass the specification to the matching `gpio_pin_configure_dt` function. We could, e.g., imagine writing a driver for a board's LEDs:

`zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts`
```dts
/ {
  leds {
    led0: led_0 {
      // gpio0: compatible = "nordic,nrf-gpio";
      gpios = <&gpio0 13 GPIO_ACTIVE_LOW>;
      // gpio-cells:
      //   - pin
      //   - flags
    };
  };
};
```

The following is some pseudo code, showing how a driver could provide a macro to assemble the GPIO configuration for the LED pin:

```c
#define LED_GPIO_SPEC_GET_BY_IDX(node_id, idx)                 \
    {                                                          \
        .pin   = DT_PHA_BY_IDX(node_id, gpios, idx, pin),      \
        .flags = DT_PHA_BY_IDX_OR(node_id, gpios, idx, flags), \
    }

// The application can then assemble its GPIO device specification ...
const device_spec_t led_gpio = LED_GPIO_SPEC_GET_BY_IDX(DT_NODELABEL(led0), 0);
// ... and use it in the corresponding API function.
(void) led_gpio_configure_dt(&led_gpio);
```

What's also shown in our own example is an `_OR` type macro: Such macros exist for practically all "normal" Devicetree macros and allow providing a default value in case the property does not exist - meaning its `_EXISTS` macro is _not defined_. Since `/node_b` does not have a specifier cell named `name-of-cell-two`, the field `cell_two` in its `node_spec_t` is always set to zero.

Without using conditional compilation or the `_OR` macro, any access would not compile, e.g.:

```c
(void) DT_PHA_BY_IDX(NODE_REFS, phandle_array_of_refs, 1, name_of_cell_two);
```

With that said and done, let's wrap up this article.



## Conclusion


In this article, we've learned about Devicetree _bindings_ and how they are used by Zephyr's generator script to produce code that can, in turn, be used by the application using Zephyr's Devicetree API. We've seen:

- How Devicetree nodes are associated with bindings,
- what macros are generated for all property types,
- and how to use the generated macros with Zephyr's Devicetree API.

We haven't used specific subsystems, such as `gpio`, which add yet another layer of complexity on top of the basic Devicetree API. There are also other, more advanced concepts in Devicetree still that we haven't seen, e.g., _nexus_ nodes.

However, with what you've learned from this and the previous article about _Devicetree_ and _bindings_, you should have no problems looking things up in [Zephyr's official documentation](https://docs.zephyrproject.org/latest/build/dts/index.html), without having to follow too many links due to the missing basics.

> **Note:** The full example application including all the Devicetree files that we've seen throughout this article is available in the [`03_devicetree_semantics` folder of the accompanying GitHub repository](https://github.com/lmapii/practical-zephyr/tree/main/03_devicetree_semantics).



## Further reading

The following are great resources when it comes to Zephyr and are worth a read _or watch_:

- As we've seen, the [official Devicetree specification](https://www.devicetree.org/specifications/) also contains information on Devicetree _bindings_.
- Second, of course, [Zephyr's official documentation on Devicetree](https://docs.zephyrproject.org/latest/build/dts/index.html) is still a go-to reference.
- The course [nRF Connect SDK Fundamentals](https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/) in Nordic's [DevAcademy](https://academy.nordicsemi.com/) guides you through some examples using the _Devicetree API_ and is therefore highly recommended.
- Definitely watch the presentation [Zephyr Devicetree Mysteries, Solved](https://www.youtube.com/watch?v=w8GgP3h0M8M&list=PLzRQULb6-ipFDwFONbHu-Qb305hJR7ICe) by Martí Bolívar.
- More _Devicetree_ examples can also be found in the [API documentation](https://docs.zephyrproject.org/latest/build/dts/api/api.html), but also in Zephyr's tests, e.g., `zephyr/dts/bindings/test` and `zephyr/tests/lib/devicetree/api/src/main.c`.
- Advanced usage of the _Devicetree API_ is shown in the [documentation about instance-based APIs](https://docs.zephyrproject.org/latest/build/dts/api/api.html#devicetree-inst-apis).
- The [Tutorial: Mastering Zephyr Driver Development](https://www.youtube.com/watch?v=o-f2qCd2AXo) by Gerard Marull Paretas from the Zephyr Development Summit 2022 is also a great resource if you want to learn more about device driver development in Zephyr.

Finally, have a look at the files in the [accompanying GitHub repository](https://github.com/lmapii/practical-zephyr) and I hope I'll see you again in the next article, where we'll finally see a practical application of everything we've learned in this article!



<!-- References

[practical-zephyr]: https://github.com/lmapii/practical-zephyr

[nordicsemi]: https://www.nordicsemi.com/
[nordicsemi-academy-devicetree]: https://academy.nordicsemi.com/topic/devicetree/
[nordicsemi-nrf52840-dk]: https://www.nordicsemi.com/Products/Development-hardware/nrf52840-dk

[devicetree-spec]: https://www.devicetree.org/specifications/
[linux-dts-bindings]: https://docs.kernel.org/devicetree/bindings/writing-schema.html
[yaml]: https://yaml.org/

[zephyr-build-board-revision]: https://docs.zephyrproject.org/latest/develop/application/index.html#application-board-version
[zephyr-dts]: https://docs.zephyrproject.org/latest/build/dts/index.html
[zephyr-dts-overlays]: https://docs.zephyrproject.org/latest/build/dts/howtos.html#set-devicetree-overlays
[zephyr-dts-intro-input-and-output]: https://docs.zephyrproject.org/latest/build/dts/intro-input-output.html
[zephyr-dts-bindings-intro]: https://docs.zephyrproject.org/latest/build/dts/bindings-intro.html
[zephyr-dts-bindings-syntax]: https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html
[zephyr-dts-bindings-syntax-include]: https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#include
[zephyr-dts-bindings-syntax-properties]: https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#properties
[zephyr-dts-bindings-syntax-bus]: https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#bus
[zephr-dts-bindings-specifier-space]: https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#specifier-space
[zephyr-dts-bindings-top-level]: https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#top-level-keys
[zephyr-dts-bindings-specifier-names]: https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#specifier-cell-names-cells
[zephr-dts-bindings-specifier-cells]: https://docs.zephyrproject.org/latest/build/dts/bindings-syntax.html#specifier-cell-names-cells
[zephyr-dts-api]: https://docs.zephyrproject.org/latest/build/dts/api/api.html
[zephyr-dts-api-instance]: https://docs.zephyrproject.org/latest/build/dts/api/api.html#devicetree-inst-apis

[zephyr-summit-22-devicetree]: https://www.youtube.com/watch?v=w8GgP3h0M8M&list=PLzRQULb6-ipFDwFONbHu-Qb305hJR7ICe
[zephyr-ds-2022-driver-dev]: https://www.youtube.com/watch?v=o-f2qCd2AXo

-->
