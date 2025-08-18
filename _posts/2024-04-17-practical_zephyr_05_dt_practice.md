---
title: Practical Zephyr - Devicetree practice (Part 5)
description: Article series "Practical Zephyr", the fifth part about Devicetree practice.
author: lampacher
tags: [zephyr, devicetree, practical-zephyr-series, nordic]
---

<!-- excerpt start -->

In the previous articles, we covered _Devicetree_ in great detail: We've seen how we can create our own nodes, we've seen the supported property types, we know what bindings are, and we've seen how to access the Devicetree using Zephyr's `devicetree.h` API. In this fifth article of the _Practical Zephyr_ series, we'll look at how Devicetree is used in _practice_ by dissecting the _Blinky_ application.

<!-- excerpt end -->

Using a practical example, we'll see that our deep dive into Devicetree was really only covering the basics that we need to understand more advanced concepts, such as [pin control](https://docs.zephyrproject.org/latest/hardware/pinctrl/index.html) and device drivers.

👉 Find the other parts of the *Practical Zephyr* series [here](/tags#practical-zephyr-series).

> 🎬
> [Listen to Martin on Interrupt Live](https://www.youtube.com/watch?v=ls_Y45WsTiA?feature=share)
> talk about the content and motivations behind writing this series.

{% include newsletter.html %}

{% include toc.html %}



## Prerequisites

This article is part of the _Practical Zephyr_ article series. In case you haven't read the previous articles, please go ahead and have a look. This article requires that you're able to build and flash a Zephyr application to the board of your choice and that you're familiar with _Devicetree_.

We'll be mainly using the [development kit for the nRF52840](https://www.nordicsemi.com/Products/Development-hardware/nrf52840-dk) but will also refer to example files from the [STM32 Nucleo-64 development board](https://www.st.com/en/evaluation-tools/nucleo-c031c6.html) towards the end of the article. You can follow along with any target - real or virtual.

> **Note:** A full example application including all files that we'll see throughout this article is available in the [`04_practice` folder of the accompanying GitHub repository](https://github.com/lmapii/practical-zephyr/tree/main/04_practice).



## Warm-up

If you've been following along, you know the drill by now: Let's create a new freestanding application with the files listed below:

```bash
$ tree --charset=utf-8 --dirsfirst
.
├── src
│   └── main.c
├── CMakeLists.txt
└── prj.conf
```

The `prj.conf` remains empty for now, and the `CMakeLists.txt` only includes the necessary boilerplate to create a Zephyr application with a single `main.c` source file. As an application, we have a `main` function that simply puts the MCU back to sleep at fixed intervals:

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

As usual, I'll build the application for my [nRF52840 Development Kit from Nordic](https://www.nordicsemi.com/), but you can use any of [Zephyr's long list of supported boards](https://docs.zephyrproject.org/latest/boards/index.html) - or an emulation target.

```bash
$ west build --board nrf52840dk_nrf52840 --build-dir ../build
```



## Devicetree with `gpio`

Let's start with a classic: The blinking LED. The easiest way to control a low-power LED is using a GPIO, and one obvious way to approach this problem is to jump straight to the [`gpio` API](https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html#gpio-api) documentation. In contrast to [Zephyr's OS service documentation](https://docs.zephyrproject.org/latest/services/index.html), however, the documentation for [peripherals](https://docs.zephyrproject.org/latest/hardware/peripherals/index.html) is typically restricted to the API, without further explaining the subsystem.

Instead, Zephyr's chosen approach is to provide a list of related **code samples** for each subsystem, showing the API in action. And Zephyr comes with _hundreds_ of [samples and demos](https://docs.zephyrproject.org/latest/samples/index.html)! In the list of related samples for the `gpio` subsystem, we also find the good old [blinky example](https://docs.zephyrproject.org/latest/samples/basic/blinky/README.html).

> **Note:** [Blinky](https://docs.zephyrproject.org/latest/samples/basic/blinky/README.html) is an example for a [Zephyr _repository_ application](https://docs.zephyrproject.org/latest/develop/application/index.html#zephyr-repo-app). We continue using a [_freestanding_ application](https://docs.zephyrproject.org/latest/develop/application/index.html#zephyr-freestanding-app), and in the next article, we'll finally create a [_workspace_ application](https://docs.zephyrproject.org/latest/develop/application/index.html#zephyr-workspace-app).

Straight from the documentation, we follow the _"Open in GitHub"_ link to find the application in Zephyr's repository and simply copy the contents from [`main.c`](https://github.com/zephyrproject-rtos/zephyr/blob/main/samples/basic/blinky/src/main.c) into our own application. We'll now compare the `gpio` Devicetree API with the "plain" API in `zephyr/include/zephyr/devicetree.h` that we've seen in the [previous article]({% post_url 2024-02-15-practical_zephyr_dt_semantics %}).


### Blinky with a `/chosen` LED node

The *Blinky* example chooses the LED Devicetree node using the _alias_ `led0`. Zephyr keeps its Devicetrees clean and we've seen that aliases, including `led0` are usually consistent throughout supported boards. Thus, if there's a board supported by Zephyr that has at least one light on it that works like LED, you can be sure that there's also a matching `led0` alias:

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
  aliases {
    led0 = &led0;
  };
};
```

Since we're practicing, however, we'll use a `/chosen` node called `app-led` instead. We add this node using a new board overlay file `boards/nrf52840dk_nrf52840.overlay` for the nRF52840 development kit:

```bash
$ tree --charset=utf-8 --dirsfirst
.
├── boards
│   └── nrf52840dk_nrf52840.overlay
├── src
│   └── main.c
├── CMakeLists.txt
└── prj.conf
```

`boards/nrf52840dk_nrf52840.overlay`

```dts
/ {
    chosen {
        app-led = &led0;
    };
};
```

In the example application, we use `DT_CHOSEN(app_led)` instead of `DT_ALIAS(led0)`, but that's about the only change for the original *Blinky* sources that we need. Notice that again we specify the chosen node's name using its "lowercase-and-underscore" form `app_led` instead of the node's name `app-led` in the Devicetree. With a few minor adaptions, our application looks as follows:

```c
/** \file main.c */

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#define SLEEP_TIME_MS 1000U
#define LED_NODE      DT_CHOSEN(app_led)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

void main(void)
{
    int err = 0;

    if (!gpio_is_ready_dt(&led)) { return; }

    err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (err < 0) { return; }

    while (1)
    {
        (void) gpio_pin_toggle_dt(&led);
        k_msleep(SLEEP_TIME_MS);
    }
}
```

Building and flashing the application we indeed end up with a blinking LED.

```bash
$ west build --board nrf52840dk_nrf52840 --build-dir ../build
$ west flash --build-dir ../build
```

![]({% img_url practical-zephyr/nrf-blinky.gif %})

I don't know about you, but even after all those years a blinking LED still puts a smile on my face.



## Dissecting the GPIO pin information type

Looking through the function prototypes of `gpio_is_ready_dt`, `gpio_pin_configure_dt`, and `gpio_pin_toggle_dt`,  you'll notice that they take a "_GPIO specification_" `const struct gpio_dt_spec *spec` as a parameter. We find the matching declaration in Zephyr's `gpio.h`:

`zephyr/include/zephyr/drivers/gpio.h`

```c
struct gpio_dt_spec {
  /** GPIO device controlling the pin */
  const struct device *port;
  /** The pin's number on the device */
  gpio_pin_t pin;
  /** The pin's configuration flags as specified in devicetree */
  gpio_dt_flags_t dt_flags;
};
```

> **Disclaimer:** This section is a nose dive deep into Zephyr's use of the generated Devicetree files. In case you don't want - or don't need - to know about the inner workings, I highly recommend sticking to [Zephyr's samples and demos](https://docs.zephyrproject.org/latest/samples/index.html) or the official documentation in general. Skip ahead to the [next section about the `status` property](#the-status-property) instead. In case I couldn't talk you out of it, hold on tight!


### Applying the Devicetree API

Let's ignore the `gpio_dt_spec` structure's contents for now. Instead, let's see how `GPIO_DT_SPEC_GET` is used to populate the structure and how it compares with what we've learned about the Devicetree API in the previous articles:

So far, we've only used the macros from the Devicetree API `zephyr/include/zephyr/devicetree.h`. Now we see how Zephyr's drivers create their own Devicetree macros on top of this basic API. We can find the macro declaration in Zephyr's `gpio.h` header file.

`zephyr/include/zephyr/drivers/gpio.h`

```c
#define GPIO_DT_SPEC_GET(node_id, prop) GPIO_DT_SPEC_GET_BY_IDX(node_id, prop, 0)
// At some other location ...
#define GPIO_DT_SPEC_GET_BY_IDX(node_id, prop, idx)                 \
  {                                                                 \
    .port = DEVICE_DT_GET(DT_GPIO_CTLR_BY_IDX(node_id, prop, idx)), \
    .pin = DT_GPIO_PIN_BY_IDX(node_id, prop, idx),                  \
    .dt_flags = DT_GPIO_FLAGS_BY_IDX(node_id, prop, idx),           \
  }
```

Similar to what we've seen for our own example in the [previous article]({% post_url 2024-02-15-practical_zephyr_dt_semantics %}), Zephyr uses Devicetree macros to create an initializer expression for the matching type that should be used with the corresponding macro. The macros `DT_GPIO_PIN_BY_IDX` and `DT_GPIO_FLAGS_BY_IDX` simply expand to the `DT_PHA_<x>` macros that we used when accessing `phandle-array`s:

```c
#define DT_GPIO_PIN_BY_IDX(node_id, gpio_pha, idx) \
  DT_PHA_BY_IDX(node_id, gpio_pha, idx, pin)
#define DT_GPIO_FLAGS_BY_IDX(node_id, gpio_pha, idx) \
  DT_PHA_BY_IDX_OR(node_id, gpio_pha, idx, flags, 0)
```

Ignoring the `.port` field (we'll get to that, don't worry), `GPIO_DT_SPEC_GET` expands to a very familiar format, where `.pin` and `.dt_flags` are initialized using the `pin` and `flags` _specifier cells_ from our Devicetree:

```c
#define GPIO_DT_SPEC_GET(node_id, prop)                              \
  {                                                                  \
    .port = DEVICE_DT_GET(/* --snip-- */),                           \
    .pin = DT_PHA_BY_IDX(node_id, prop, /*idx*/0, pin),              \
    .dt_flags = DT_PHA_BY_IDX_OR(node_id, prop, /*idx*/0, flags, 0), \
  }
```

> **Note:** The downside of using C macros is that, well, they're macros. Macros are expanded by the preprocessor, and the preprocessor doesn't care about types. It therefore also doesn't care if you use `GPIO_DT_SPEC_GET` as an initializer for an incompatible type. This _typically_ fails during compile time in case the assigned variable or constant doesn't have a compatible type.


### Reviewing *phandle-array*s

Properties of type `phandle-array` are heavily used in Devicetrees. Since we now have one at hand with our *Blinky* example, let's use it to review what we've learned about `phandle-array`s - practice and repetition is the key to learning new concepts! The `/leds/led0` is defined in the board's DTS file as follows:

`(reduced) zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts`

```dts
/ {
  leds {
    compatible = "gpio-leds";
    led0: led_0 {
      gpios = <&gpio0 13 GPIO_ACTIVE_LOW>;
    };
  };
};
```

The matching binding specifies that all child nodes of `led` have the required property `gpios` of type `phandle-array`:

`(reduced) zephyr/dts/bindings/led/gpio-leds.yaml`

```yaml
compatible: "gpio-leds"
child-binding:
  description: GPIO LED child node
  properties:
    gpios:
      type: phandle-array
      required: true
```

Nothing unexpected here: Since `gpios` is of type `phandle-array`, we can use it to refer to any of the board's GPIO instances. Looking at the above binding you might ask yourself: Except for the property's name, how do we know which nodes we can refer to in this `phandle-array`?

We don't.

Looking at the *Devicetree* in isolation, you can, in fact, use references to _any_ node, as long as the node has the matching `#gpio-cells` property. You could, e.g., create your own node and binding, where `#gpio-cells` doesn't use `pin` and `flags` as specifiers. E.g., we could define our own binding `custom-cells-a`:

`dts/bindings/custom-cells-a.yaml`

```yaml
description: Dummy for matching "cells"
compatible: "custom-cells-a"
gpio-cells:
  - name-of-cell-one
  - name-of-cell-two
  - name-of-cell-three
```

In our board overlay, we can create a dummy `node_a` with the above binding, and overwrite the property `gpio` of `led0` with a reference to this dummy node:

`boards/nrf52840dk_nrf52840.overlay`

```dts
/ {
    chosen {
        app-led = &led0;
    };
    node_a {
        compatible = "custom-cells-a";
        #gpio-cells = <3>;
    };
};
&led0 {
    gpios = <&{/node_a} 1 2 3>;
};
```

If we were to revert our `main.c` file to the original dummy application that doesn't use the `gpio` subsystem, the build passes without warnings and we'd find our `/leds/led_0` node with the following properties in the merged `zephyr.dts` file:

`build/zephyr/zephyr.dts`

```dts
/ {
  leds {
    compatible = "gpio-leds";
    led0: led_0 {
      gpios = < &{/node_a} 0x1 0x2 0x3 >;
      label = "Green LED 0";
    };
  };
};
```

There is no mechanism in *Devicetree* that allows declaring a `phandle-array` which is _"generic over a type"_ or _"something GPIO compatible"_. E.g., there's no such thing as a _base class_, _interface_, or _trait_ that you might know from programming languages.

Even if there was an annotation `phandle-array<T>`, e.g., to specify the required `compatible` property, what would we provide for `T`? GPIO nodes use different models depending on the vendor, e.g., the following snippets show the `compatible` properties of the GPIO nodes of the nRF52840 and STM32:

`zephyr/dts/arm/nordic/nrf52840.dtsi`

```dts
/ {
  soc {
    gpio0: gpio@50000000 { compatible = "nordic,nrf-gpio"; };
    gpio1: gpio@50000300 { compatible = "nordic,nrf-gpio"; };
  };
};
```

`zephyr/dts/arm/st/c0/stm32c0.dtsi`

```dts
/ {
  soc {
    pinctrl: pin-controller@50000000 {
      gpioa: gpio@50000000 { compatible = "st,stm32-gpio"; };
      gpiob: gpio@50000400 { compatible = "st,stm32-gpio"; };
      gpioc: gpio@50000800 { compatible = "st,stm32-gpio"; };
      gpiof: gpio@50001400 { compatible = "st,stm32-gpio"; };
    };
  };
};
```

Sure, both "nordic,nrf-gpio" and "st,stm32-gpio" could claim compatibility with some "base binding", but there'd always be the odd corner case that doesn't entirely match the model.

By discarding such a requirement, we gain flexibility - at the cost of "loose typing" _within the Devicetree_. If we were to try and use the Zephyr GPIO API with the above assignment to `gpios`, the application would fail to compile since the required _specifier cells_ do not have the names `pin` and `flags` read using the `DT_PHA_BY_IDX[_OR]` macros in the `GPIO_DT_SPEC_GET` macro. There's one more thing within `GPIO_DT_SPEC_GET` that is related to this discussion, can you see it?

```c
#define GPIO_DT_SPEC_GET(node_id, prop)                              \
  {                                                                  \
    .port = DEVICE_DT_GET(/* --snip-- */),                           \
    .pin = DT_PHA_BY_IDX(node_id, prop, /*idx*/0, pin),              \
    .dt_flags = DT_PHA_BY_IDX_OR(node_id, prop, /*idx*/0, flags, 0), \
  }
```

Looking at `dt_flags`, we see that `GPIO_DT_SPEC_GET` uses the `_OR` variant of the phandle macro `DT_PHA_BY_IDX` to read the `flags` from the Devicetree:

* For nodes with a _compatible binding_ that have both, `pin` and `flags` specifiers, the Devicetree compiler ensures that values are provided for both, `pin` and `flags` whenever the node is references in a `phandle-array`; providing a value for `flags` is **not** optional for such phandles.
* For nodes with a _compatible binding_ that do **not** have the `flags` specifier, the value _0_ is used in the specification. The compatible driver can also completely ignore the _flags_ field in any API call.

This adds another level of flexibility to the generic binding `gpio-leds` for LEDs, supporting any kind of GPIO node LEDs, regardless of whether or not they support using `flags`, e.g., as follows:

```dts
/ {
  leds {
    compatible = "gpio-leds";
    led0: led_0 { gpios = <&gpio0 13>; };
  };
};
```


### Device objects and driver compatibility

We now know how the `.pin` and `.dt_flags` fields of the structure are populated, but what about the `.port`? Let's try to find out what the macro expands to.

```c
#define GPIO_DT_SPEC_GET(node_id, prop)                                  \
  {                                                                      \
    .port = DEVICE_DT_GET(DT_GPIO_CTLR_BY_IDX(node_id, prop, /*idx*/0)), \
    .pin = DT_PHA_BY_IDX(node_id, prop, /*idx*/0, pin),                  \
    .dt_flags = DT_PHA_BY_IDX_OR(node_id, prop, /*idx*/0, flags, 0),     \
  }
```

The macro parameters are given in our application and expand as described in the following snippet:

```c
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_CHOSEN(app_led), gpios);
// Knowing that `DT_CHOSEN(app_led)` expands to `DT_CHOSEN_app_led`,
// we can look up `DT_CHOSEN_app_led` in `devicetree_generated.h`,
// which is defined as `DT_N_S_leds_S_led_0`.

// Thus:
// - node_id = DT_N_S_leds_S_led_0
// - prop    = gpios
//
// .port = DEVICE_DT_GET(DT_GPIO_CTLR_BY_IDX(DT_N_S_leds_S_led_0, gpios, /*idx*/0)),
```

#### Macrobatics: Resolving device objects with `DEVICE_DT_GET`

Before resolving macros manually, it is always worth looking into the documentation. We have two nested macros, so it makes sense to check the inner `DT_GPIO_CTLR_BY_IDX` first. The API documentation claims that it can be used to "_get the node identifier for the controller phandle from a gpio phandle-array property_". For the assignment `gpios = <&gpio0 13 GPIO_ACTIVE_LOW>;` in our LED node, we'd thus expect to get the node identifier for the phandle `&gpio0`.

Let's have a look at the macro and its expansion:

```c
// We want to know the expansion of the following macro:
#define DT_GPIO_CTLR_BY_IDX(node_id, gpio_pha, idx) \
  DT_PHANDLE_BY_IDX(node_id, gpio_pha, idx)

// Knowing that DT_PHANDLE_BY_IDX is defined as follows:
#define DT_PHANDLE_BY_IDX(node_id, prop, idx) \
  DT_CAT6(node_id, _P_, prop, _IDX_, idx, _PH)

// Given:
// node_id = DT_N_S_leds_S_led_0
// prop    = gpios
// idx     = 0

// DT_GPIO_CTLR_BY_IDX
//    = DT_N_S_leds_S_led_0 ## _P_ ## gpios ## _IDX_ ## 0 ## _PH
//    = DT_N_S_leds_S_led_0_P_gpios_IDX_0_PH
```

Thus, `DT_GPIO_CTLR_BY_IDX` resolves to `DT_N_S_leds_S_led_0_P_gpios_IDX_0_PH`. We can find a macro for that token in `devicetree_generated.h`, which indeed resolves to the GPIO node's identifier `DT_N_S_soc_S_gpio_50000000`:

```bash
$ grep DT_N_S_leds_S_led_0_P_gpios_IDX_0_PH ../build/zephyr/include/generated/devicetree_generated.h
#define DT_N_S_leds_S_led_0_P_gpios_IDX_0_PH DT_N_S_soc_S_gpio_50000000
```

On we go! `DEVICE_DT_GET` takes this node identifier as its parameter `node_id`.

```c
// Original macro from zephyr/include/zephyr/device.h
#define DEVICE_DT_GET(node_id) (&DEVICE_DT_NAME_GET(node_id))
```

What can the documentation tell us about this macro?

> Returns a pointer to a device object created from a Devicetree node, if any device was allocated by a driver. If no such device was allocated, this will fail at linker time [with an error like `undefined reference to __device_dts_ord_<N>`]

Since we don't know yet what "_device objects_" are, this is a bit cryptic. There is, however, one good hint in the linker error message: It seems like this macro is trying to provide a reference to a symbol called `__device_dts_ord_<N>`, where `N` is a number.

Without the need for _macrobatics_ on your side, your editor of choice should be able to resolve this macro. The following is a screenshot from `vscode`, where the last line indeed shows that - for my application - `.port` is assigned a reference to the symbol `__device_dts_ord_11`:

![]({% img_url practical-zephyr/vscode-macrobatics.png %})

We could be satisfied with this resolution, but we're not, are we? Aren't you curious how we somehow get from a node identifier to this weird symbol with the ordinal _11_? Well, I am so it's time for some more macrobatics!

In the following snippet we replace referenced macros step by step _without_ showing their definition, and thus - to some extent - perform the same expansion as the preprocessor. Since the steps are not _exactly_ the same, however, we're calling them "_replacements_" and not _"expansions"_:

```c
// Original macro from zephyr/include/zephyr/device.h
#define DEVICE_DT_GET(node_id) (&DEVICE_DT_NAME_GET(node_id))

// 1st replacement: Given DEVICE_DT_NAME_GET = DEVICE_NAME_GET(Z_DEVICE_DT_DEV_ID(node_id))
#define DEVICE_DT_GET_1st(node_id) (&DEVICE_NAME_GET(Z_DEVICE_DT_DEV_ID(node_id)))

// 2nd replacement: Given DEVICE_NAME_GET = _CONCAT(__device_, dev_id)
#define DEVICE_DT_GET_2nd(node_id) (&_CONCAT(__device_, ,Z_DEVICE_DT_DEV_ID(node_id)))

// 3rd replacement: Z_DEVICE_DT_DEV_ID = _CONCAT(dts_ord_, DT_DEP_ORD(node_id))
#define DEVICE_DT_GET_3rd(node_id) (&_CONCAT(__device_, _CONCAT(dts_ord_, DT_DEP_ORD(node_id))))

// 4th replacement: DT_DEP_ORD = DT_CAT(node_id, _ORD)
#define DEVICE_DT_GET_4th(node_id) (&_CONCAT(__device_, _CONCAT(dts_ord_, DT_CAT(node_id, _ORD))))
```

> **Note:** In case you're wondering what the difference between `_CONCAT` and `DT_CAT` is, experiment with token pasting. Hint: One of the macros simply pastes tokens, whereas the other one also expands the pasted token.

It's easy enough to see how we end up with the prefix `__device_dts_ord_`, but it is not entirely clear yet how we get to the ordinal _11_. For this, we still need to resolve `DT_CAT(node_id, _ORD)`. Remembering that we already resolved our input parameter `node_id` to `DT_N_S_soc_S_gpio_50000000`, `DT_CAT` simply pastes the two tokens, and we end up with `DT_N_S_soc_S_gpio_50000000_ORD`. But what's that? This is a macro that is again provided by Zephyr's Devicetree generator:

```bash
$ grep DT_N_S_soc_S_gpio_50000000_ORD ../build/zephyr/include/generated/devicetree_generated.h
#define DT_N_S_soc_S_gpio_50000000_ORD 11
```

In simple words, Zephyr's Devicetree generator assigns _ordinals_ to device instances. This is a fundamental concept of Zephyr's _device_ API `zephyr/include/zephyr/device.h`, which is used to represent devices and their instances. Since this is a rather elaborate concept that goes beyond the scope of this article series, I'll leave you with a link to Zephyr's official documentation about the [device driver model](https://docs.zephyrproject.org/latest/kernel/drivers/index.html) and [instance-based APIs](https://docs.zephyrproject.org/latest/build/dts/api/api.html#devicetree-inst-apis).

> **Note:** You might ask yourself why we even need _ordinals_ if Devicetree nodes are unique. One simple reason is that nodes can support multiple _drivers_, e.g., an IC might support SPI as well as I2C, and therefore two instances exist for a single node.

For now, it is enough to know that Zephyr creates symbols for each device instance in the Devicetree. In fact, in `devicetree_generated.h` we can find a list of the node ordering and thus ordinals of each instance at the very beginning of the file. Here, we find the ordinal _11_ for the device of the GPIO node `/soc/gpio@50000000`, and also the ordinal _104_ of the device for our second GPIO node `/soc/gpio@50000300`:

`build/zephyr/include/generated/devicetree_generated.h`

```c
/*
 * Generated by gen_defines.py
 *
 * DTS input file:
 *   /path/to/build/zephyr/zephyr.dts.pre
 *
 * Directories with bindings:
 *   /opt/nordic/ncs/v2.4.0/nrf/dts/bindings, /path/to/dts/bindings, $ZEPHYR_BASE/dts/bindings
 *
 * Node dependency ordering (ordinal and path):
 *   0   /
 *   1   /aliases
 *   2   /analog-connector
 * --snip--
 *   11  /soc/gpio@50000000
 * --snip--
 *   104 /soc/gpio@50000300
 * --snip--
 */

// --snip--
#define DT_N_S_soc_S_gpio_50000300_ORD 104
// --snip--
#define DT_N_S_soc_S_gpio_50000000_ORD 11
```

Now we can finalize the macro expansion of `DEVICE_DT_GET`:

```c
// given `DT_N_S_soc_S_gpio_50000000` for `node_id`:
// DT_CAT(node_id, _ORD)) resolves to `DT_N_S_soc_S_gpio_50000000_ORD`

#define DEVICE_DT_GET_4th(node_id) (&_CONCAT(__device_, _CONCAT(dts_ord_, DT_CAT(node_id, _ORD))))
#define DEVICE_DT_GET_x_1st()      (&_CONCAT(__device_, _CONCAT(dts_ord_, DT_N_S_soc_S_gpio_50000000_ORD)))
#define DEVICE_DT_GET_x_2nd()      (&_CONCAT(__device_, dts_ord_11))
#define DEVICE_DT_GET_x_3rd()      (&__device_dts_ord_11)
```

With this, we finally pieced together the complete assignment for our LED's GPIO specification:

```c
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
// =  {
//    .port = (&__device_dts_ord_11),
//    .pin = 13 /* DT_N_S_leds_S_led_0_P_gpios_IDX_0_VAL_pin */,
//    .dt_flags = 1 /* DT_N_S_leds_S_led_0_P_gpios_IDX_0_VAL_flags */,
//  }
```

That's all good and well, but where is this symbol `__device_dts_ord_11` coming from? With all this macro magic, we can imagine that it is very unlikely that a search for `__device` within Zephyr's codebase provides any valuable hint about the symbol's location. Instead, we can use a much more reliable method: Let's look for the symbol in the `.map` file:

```bash
$ grep -sw __device_dts_ord_11 ../build/zephyr/zephyr.map -A 1 -B 3
 .z_device_PRE_KERNEL_140_
    0x0000000000006550  0x30 zephyr/drivers/gpio/libdrivers__gpio.a(gpio_nrfx.c.obj)
    0x0000000000006550           __device_dts_ord_104
    0x0000000000006568           __device_dts_ord_11
 .z_device_PRE_KERNEL_155_
```

> **Note:** Your `grep` mileage may vary, I've just added `-A 1` and `-B 3` since I know that, in my application, the corresponding object and all the remaining instances are visible if I include one line _after_ (`-A 1`) and three lines _before_ (`-B 3`) the occurrence.

We found it! It seems to be declared in Nordic's GPIO driver `zephyr/drivers/gpio/gpio_nrfx.c`.


#### Macrobatics: Declaring compatilble drivers and device object

I promised that we wouldn't go into detail about [Zephyr's device driver model](https://docs.zephyrproject.org/latest/kernel/drivers/index.html) - and we won't. In this section, we'll only look at how the device instances are defined and how the connection with the nodes in the Devicetree is established.

The two responsible parts within `gpio_nrfx.c` for defining the instances are the following:

```c
#define DT_DRV_COMPAT nordic_nrf_gpio
/* --snip-- the entire driver implementation */

#define GPIO_NRF_DEVICE(id)                 \
  /* --snip-- */                            \
  DEVICE_DT_INST_DEFINE(id, gpio_nrfx_init, \
      /* --snip-- */                        \
      &gpio_nrfx_drv_api_funcs);

DT_INST_FOREACH_STATUS_OKAY(GPIO_NRF_DEVICE)
```

The macro definition `DT_DRV_COMPAT` is placed at the beginning of the file. It is the device driver's equivalent to the `compatible` property of Devicetree nodes and the same-named key in the Devicetree binding: It is defined to the "lowercase-and-underscore" form `nordic_nrf_gpio` of `nordic,nrf-gpio`.

At the end of the file, the device driver creates instances for all GPIO devices with the `status` property set to "okay" using the macro `DT_INST_FOREACH_STATUS_OKAY`. And that's where somehow the "_device objects_" `__device_dts_ord_104` and `__device_dts_ord_11` of the corresponding GPIO nodes are created, pretty likely through `DEVICE_DT_INST_DEFINE`.

Let's dissect these macros one by one, starting with `DT_INST_FOREACH_STATUS_OKAY`, which is more concise but also more complex:

```c
#define DT_INST_FOREACH_STATUS_OKAY(fn)                       \
  COND_CODE_1(DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT),       \
    (UTIL_CAT(DT_FOREACH_OKAY_INST_, DT_DRV_COMPAT)(fn)),     \
    ()                                                        \
  )
```

For the expansion of `COND_CODE_1` we rely on the docs, which state that this macro _"insert[s] code depending on whether _flag expands to 1 or not."_. The first parameter is the *_flag*, the second parameter is the code in case *_flag* expands to 1, and finally, the third parameter is the code that is used in case the *_flag* is **not** 1 or undefined.

To get to the flag, we need to check `DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)`:

```c
#define DT_HAS_COMPAT_STATUS_OKAY(compat) \
  IS_ENABLED(DT_CAT(DT_COMPAT_HAS_OKAY_, compat))
```

We've seen the `IS_ENABLED` macro already: It is a more intricate macro that allows evaluating tokens even if they are not even defined. Since we know that in our case `compat` is `nordic_nrf_gpio`, we're looking for a macro `DT_COMPAT_HAS_OKAY_nordic_nrf_gpio`, which, as usual, we can find in `devicetree_generated.h`:

```bash
grep -w DT_COMPAT_HAS_OKAY_nordic_nrf_gpio ../build/zephyr/include/generated/devicetree_generated.h
#define DT_COMPAT_HAS_OKAY_nordic_nrf_gpio 1
```

Zephyr's Devicetree generator creates a lot more information than just macros for the property values of our Devicetree nodes! Here, we see that the generator also provides macros indicating whether it encountered some nodes that claim compatibility with "`nordic,nrf-gpio`". If it doesn't encounter any such node, it won't create the corresponding macros and we'd know that the content of any driver compatible with "`nordic,nrf-gpio`" is unused and can thus be discarded. Pretty neat!

The Devicetree generator does this for each and every driver and node. This allows heavy optimizations in the codebase as we've seen above.

Now we found out that our driver is used, and thus the second parameter of `COND_CODE_1` is expanded. We've already seen that `UTIL_CAT` concatenates and expands the provided macros, which are `DT_FOREACH_OKAY_INST_` and our `DT_DRV_COMPAT` macro, which expands to `nordic_nrf_gpio`. Thus, we're now looking for a macro `DT_FOREACH_OKAY_INST_nordic_nrf_gpio`:

```bash
grep -w DT_FOREACH_OKAY_INST_nordic_nrf_gpio ../build/zephyr/include/generated/devicetree_generated.h
#define DT_FOREACH_OKAY_INST_nordic_nrf_gpio(fn) fn(0) fn(1)
```

Also here Zephyr's Devicetree generator creates another useful macro based on the number of devices in the Devicetree claiming compatibility with the `nordic,nrf-gpio` device driver. For each device it encounters it adds a `fn(n)` to the macro, where `n` is the _instance_ number of the device, one for each of our GPIO nodes. Given this macro, we now know the complete expansion of the `DT_INST_FOREACH_STATUS_OKAY` macro:

```c
// `DT_INST_FOREACH_STATUS_OKAY(GPIO_NRF_DEVICE)`
// given DT_DRV_COMPAT = nordic_nrf_gpio
// expands to:
GPIO_NRF_DEVICE(0) GPIO_NRF_DEVICE(1)
```

The above macros are expanded for the corresponding device _instance_ number, but we still don't know how to associate the instance number with the _ordinals_, which are _11_ and _104_ for our GPIO devices. So far, we can only establish a connection to the nodes `/soc/gpio@50000000` and `/soc/gpio@50000300` via the _ordinals_.

This leads us to the expansion of `DEVICE_DT_INST_DEFINE` used by `GPIO_NRF_DEVICE`:

```c
#define GPIO_NRF_DEVICE(id)                 \
  /* --snip-- */                            \
  DEVICE_DT_INST_DEFINE(id, gpio_nrfx_init, \
      /* --snip-- */                        \
      &gpio_nrfx_drv_api_funcs);

// 1st replacement:
// DEVICE_DT_INST_DEFINE(inst, ...) =
//   DEVICE_DT_DEFINE(DT_DRV_INST(inst), __VA_ARGS__)
#define GPIO_NRF_DEVICE_1st(id)                     \
  /* --snip-- */                                    \
  DEVICE_DT_DEFINE(DT_DRV_INST(id), gpio_nrfx_init, \
      /* --snip-- */                                \
      &gpio_nrfx_drv_api_funcs);
```

The documentation of `DT_DRV_INST` states that it is used to get the "_node identifier for an instance of a `DT_DRV_COMPAT` compatible_". That's exactly what we'd expect, but we want to know _how_ this is done: How do we match a node (identifier) to a device _instance_ number? Looks like we'll have to do a quick macro expansion for `DT_DRV_INST` as well:

```c
// Expansion of DT_DRV_INST, given
// DT_INST(inst, compat) = UTIL_CAT(DT_N_INST, DT_DASH(inst, compat))
// and DT_DASH pastes underscores before and between the arguments.
#define DT_DRV_INST_1st(inst)  DT_INST(inst, DT_DRV_COMPAT)
#define DT_DRV_INST_2nd(inst)  UTIL_CAT(DT_N_INST, DT_DASH(inst, DT_DRV_COMPAT))
#define DT_DRV_INST_3rd(inst)  UTIL_CAT(DT_N_INST, _ ## inst ## DT_DRV_COMPAT)

// Given:
// - DT_DRV_COMPAT = nordic_nrf_gpio
// - inst = 0 (and analog for 1, from the expansion of DT_INST_FOREACH_STATUS_OKAY)
#define DT_DRV_INST_0_1st  UTIL_CAT(DT_N_INST, DT_DASH(0, nordic_nrf_gpio))
#define DT_DRV_INST_0_2nd  UTIL_CAT(DT_N_INST, _0_nordic_nrf_gpio)
#define DT_DRV_INST_0_3rd  DT_N_INST_0_nordic_nrf_gpio
// and  DT_DRV_INST_1_3rd  DT_N_INST_1_nordic_nrf_gpio  ... for inst = 1
```

```bash
$ grep -w DT_N_INST_0_nordic_nrf_gpio ../build/zephyr/include/generated/devicetree_generated.h
#define DT_N_INST_0_nordic_nrf_gpio DT_N_S_soc_S_gpio_50000000
$ grep -w DT_N_INST_1_nordic_nrf_gpio ../build/zephyr/include/generated/devicetree_generated.h
#define DT_N_INST_1_nordic_nrf_gpio DT_N_S_soc_S_gpio_50000300
```

Yet again Zephyr's Devicetree generator provides the necessary macros to map the instance number `inst` to the matching node that claims compatibility with a given device driver:

- `/soc/gpio@50000000` is instance _0_, and
- `/soc/gpio@50000300` is instance _1_ of the driver `nordic,nrf-gpio`.

This is it! We now have the connection from our _instance_ number to _node identifier_ and thus the Devicetree node. We've already seen how we can get from the _node identifier_ to the _"device object's"_ name, and can therefore apply that knowledge to declare the symbol.

Through several expansions, the device macro `DEVICE_DT_DEFINE` effectively does exactly that: It declares a global constant using the name `__device_dts_ord_<N>`, where `N` is the ordinal it obtains using the _node identifier_ of the corresponding instance. The following is an oversimplified definition for the macro `DEVICE_DT_DEFINE`:

```c
// Simplified definition of DEVICE_DT_DEFINE for the device object:
#define DEVICE_DT_DEFINE_simplified(node_id, ...)                     \
  const struct device DEVICE_NAME_GET(Z_DEVICE_DT_DEV_ID(node_id)) =  \
    Z_DEVICE_INIT(/* --snip-- */)

// Remember the expansion of DEVICE_NAME_GET(Z_DEVICE_DT_DEV_ID(node_id)
// in the expansion for DEVICE_DT_GET that we've seen before.

// For node_id = DT_N_S_soc_S_gpio_50000000 ->
// const struct device __device_dts_ord_11 = ...
// For node_id = DT_N_S_soc_S_gpio_50000300 ->
// const struct device __device_dts_ord_104 = ...
```

In practice, Zephyr also creates other symbols that are used to populate the device object. This device object contains, e.g., a function table for the entire GPIO API, such that a call to a `gpio` function leads to the correct call of the vendor's function. E.g., a call to `gpio_pin_toggle_dt` for a GPIO that is compatible with `nordic,nrf-gpio` eventually leads to a call to `gpio_nrfx_port_toggle_bits`.

For details, however, you'll need to refer to Zephyr's official documentation about the [device driver model](https://docs.zephyrproject.org/latest/kernel/drivers/index.html) and [instance-based APIs](https://docs.zephyrproject.org/latest/build/dts/api/api.html#devicetree-inst-apis), or watch the tutorial ["Mastering Zephyr Driver Development"](https://www.youtube.com/watch?v=o-f2qCd2AXo) by Gerard Marull Paretas from the Zephyr Development Summit 2022.

There's also a blog post on this Interrupt blog about [building drivers on Zephyr](https://interrupt.memfault.com/blog/building-drivers-on-zephyr).


### Summary

We now not only know how the GPIO subsystem uses the macros from `devicetree.h` to get the _specifiers_ for `gpio` properties of type `phandle-array`, but we also know what the device objects are and how they're resolved:

```c
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
// =  {
//    .port = (&__device_dts_ord_11),
//    .pin = 13 /* DT_N_S_leds_S_led_0_P_gpios_IDX_0_VAL_pin */,
//    .dt_flags = 1 /* DT_N_S_leds_S_led_0_P_gpios_IDX_0_VAL_flags */,
//  }
```

Zephyr's Devicetree generator provides much more than just macros for accessing property values in the Devicetree. It also creates the necessary macros to _associate_ driver instances with nodes, and to _create_ the corresponding instances in the device driver itself. And then some ...



## The `status` property

One important property that we've touched already is the `status` property. While our LED node has no such property, the referenced GPIO node in its `gpios` property does. The nodes are first defined in the MCU's DTS file with the `status` property set to `"disabled"`:

`zephyr/dts/arm/nordic/nrf52840.dtsi`

```dts
/ {
  soc {
    gpio0: gpio@50000000 { /* --snip-- */ status = "disabled"; port = <0>; };
    gpio1: gpio@50000300 { /* --snip-- */ status = "disabled"; port = <1>; };
  };
};
```

In the nRF52840 development kit's DTS file, the `status` property is overwritten with `"okay"`:

`zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts`

```dts
&gpio0 { status = "okay"; };
&gpio1 { status = "okay"; };
```

Zephyr's introduction to Devicetree explains the `status` property in a dedicated section for [important Devicetree properties](https://docs.zephyrproject.org/2.7.5/guides/dts/intro.html#important-properties) as follows:

> "A node is considered enabled if its `status` property is either `"okay"` or not defined (i.e., does not exist in the Devicetree source). Nodes with `status` `"disabled"` are explicitly disabled. [...] Devicetree nodes which correspond to physical devices must be enabled for the corresponding `struct` device in the Zephyr driver model to be allocated and initialized."

This matches with what we've seen for the `DT_INST_FOREACH_STATUS_OKAY` macro in `gpio_nrfx.c`: Instances are only created for each node with the `status` set to `"okay"`. We can disable the node by setting the `status` to `"disabled"` in our application's board overlay file:

```dts
&gpio0 {
    status = "disabled";
};
```

Trying to recompile the project leads to the linker error that we've seen in the documentation of `DEVICE_NAME_GET`:

```
/opt/nordic/ncs/v2.4.0/zephyr/include/zephyr/device.h:84:41: error:
'__device_dts_ord_11' undeclared here (not in a function);
did you mean '__device_dts_ord_15'?
   84 | #define DEVICE_NAME_GET(dev_id) _CONCAT(__device_, dev_id)
      |                                         ^~~~~~~~~
```

The matching device object has not been allocated in `gpio_nrfx.c` and thus the linker can't resolve the symbol `__device_dts_ord_11`. For demonstration purposes (this is not typically done in an application) we _could_ transform this into a compiler error by using the `devicetree.h` API as follows:

```c#
#if DT_NODE_HAS_STATUS(DT_PHANDLE(LED_NODE, gpios), okay)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
#else
#error "The status of the GPIO referenced by the LED node is not 'okay'"
#endif
```

Discard the changes to the board's overlay and remove the conditional compilation in the sources. The documentation also mentions, that the `status` property is implicitly added with the value `"okay"` for nodes that do not define the property in the Devicetree. As we can see in the merged `zephyr.dts` file in the build folder, our `/leds/led_0` node doesn't have the `status` property:

`build/zephyr/zephyr.dts`

```dts
/ {
  leds {
    compatible = "gpio-leds";
    led0: led_0 {
      gpios = < &gpio0 0xd 0x1 >;
      label = "Green LED 0";
    };
  };
};
```

Instead of looking into `devicetree_generated.h`, we can also check that the node is indeed implicitly assigned the `status` with the value `"okay"` using the following compile time switch and macros from `devicetree.h`:

```c
#if !DT_NODE_HAS_STATUS(LED_NODE, okay)
#error "LED node status is not okay." /* `status` is added implicitly. */
#endif
```

Rebuilding the application works without any warnings or errors and the LED continues blinking happily. You'll find such conditional compilation switches in many of Zephyr's source files. _How_ the `status` property is used varies between the corresponding subsystems.

What's the use of the `status`, then? You can and should use `status` to _disable_ all nodes that you don't need. This typically leads to a (slight) reduction of code size but is especially important for low-power applications to reduce power consumption by disabling unused peripherals.

> **Note:** In case you're wondering what the difference between the `status` property and the `is_ready_dt` function call is - you're not alone, so let's clarify this briefly. The `status` property is used to _remove_ instances altogether, whereas the `is_ready_dt` ensures that the driver is ready to be used. You can't call `is_ready_dt` with a specification for a disabled node - as we've seen, the compilation or linking fails entirely for disabled nodes.


### Intermezzo: Power profiling

Let's apply what we've just learned and observe the MCU's current consumption with and without disabling unused nodes. To do that, I'll be using Nordic's [Power Profiler Kit](https://www.nordicsemi.com/Products/Development-hardware/Power-Profiler-Kit-2). You can, of course, use any clamp meter or other measurement hardware to verify the current consumption.

![]({% img_url practical-zephyr/kit-ppk.jpg %})

<!-- > **Disclaimer:** Nope, I'm not affiliated with Nordic in any way. I'm just a big fan of their MCUs, hardware, and software. You can, of course, use any clamp meter or other measurement hardware to verify the current consumption. -->

Without disabling unused nodes, I'm observing a current consumption in the application's sleep cycles of around *550 uA*. The screenshot below shows the measurement for an interval of 3 seconds, where the y-axis measures the current consumption truncated to a range of *0 .. 5 mA*. You should still be able to make out the three peaks in the current consumption where the MCU wakes up to toggle the LED.

![]({% img_url practical-zephyr/ppk-status-enabled.png %})

How can we find out which nodes to disable?

Zephyr's board DTS files typically enable plenty of nodes, mostly for you to be able to run the samples without having to use an overlay to enable the required nodes. For a custom board, this might not be the case, meaning nodes could be _disabled_ by default and you might have to enable nodes before using them. In the end, you should be well aware of the peripherals and thus nodes that you need, and should thus _simply know_ which nodes can be disabled.

In case of doubt another good location to look for a node's `status` is the `zephyr.dts` in the build directory. I've picked the following nodes from there and disabled them in the board's overlay:

`boards/nrf52840dk_nrf52840.overlay`

```dts
&adc {status = "disabled"; };
&i2c0 {status = "disabled"; };
&i2c1 {status = "disabled"; };
&pwm0 {status = "disabled"; };
&spi0 {status = "disabled"; };
&spi1 {status = "disabled"; };
&spi3 {status = "disabled"; };
&usbd {status = "disabled"; };
&nfct {status = "disabled"; };
&temp {status = "disabled"; };
&radio {status = "disabled"; };
&uart0 {status = "disabled"; };
&uart1 {status = "disabled"; };
&gpiote {status = "disabled"; };
```

> **Note:** We're also disabling `uart0` and therefore the `/chosen` node for our console output - the boot banner `*** Booting Zephyr OS build v3.3.99-ncs1 ***` will no longer be output.

After rebuilding and flashing the application, I can perform the same measurement again and I indeed got a very different result, as you can see in the below screenshot:

![]({% img_url practical-zephyr/ppk-status-disabled.png %})

Notice that the scale of the y-axis changed from *0 .. 5 mA* to  *0 .. 20 uA*, that's a factor 250 smaller! This is also visible in the MCU's current consumption in the sleep phase, which dropped from *550 uA* to an average of *7 uA*.

It is quite convenient and easy enough to disable unused peripherals. If you think that power management is Zephyr is as simple as that, however, I'll have to disappoint you: This is really just a way to disable peripherals that are _not used at all_. If we'd, e.g., need the UART while the device is not in sleep mode, we're looking at an entirely different mechanism. We'll only scratch the surface of power management in Zephyr when we have a look at [pin control](#peripheral-pin-control-in-zephyr), but for details, you'll need to dive into the [official documentation](https://docs.zephyrproject.org/latest/services/pm/index.html).

Comparing the memory usage of the build with and without optimized node status, we can also see a slight gain in the flash memory consumption:

```
# west build with all nodes "okay"
Memory region     Used Size  Region Size  %age Used
       FLASH:       27808 B         1 MB      2.65%
         RAM:        7552 B       256 KB      2.88%
    IDT_LIST:          0 GB         2 KB      0.00%
```

```
# west build with optimized node status
Memory region     Used Size  Region Size  %age Used
       FLASH:       25352 B         1 MB      2.42%
         RAM:        7552 B       256 KB      2.88%
    IDT_LIST:          0 GB         2 KB      0.00%
```



## Device driver function calls

Even though we still won't write an instance-based device driver in this article series it is worth quickly (and this time I mean _really quickly_) reviewing how Zephyr's API function calls map to the function tables provided by the device drivers.

Let's first have a look at an overly simplified Devicetree driver function call tree for `gpio_pin_configure_dt`, which looks approximately as follows:

```
gpio_pin_configure_dt(
  │     spec, extra_flags)
  └── gpio_pin_configure(
        │     spec->port, spec->pin, spec->dt_flags | extra_flags)
        └── z_impl_gpio_pin_configure(
              │     spec->port, spec->pin, spec->dt_flags | extra_flags)
              └── spec->port->api->pin_configure(
                        spec->port, spec->pin, flags)
```

A call to `gpio_pin_configure_dt` thus maps to the function call `spec->port->api->pin_configure`. When we dissected the GPIO pin information type, we've already seen that `spec->port` is assigned the [device object `__device_dts_ord_<N>`](#macrobatics-resolving-device-objects-with-device_dt_get). Knowing already that our device object is created in `zephyr/drivers/gpio/gpio_nrfx.c`, we can immediately see that the API function table is the last parameter passed to the macro `DEVICE_DT_INST_DEFINE`:

`zephyr/drivers/gpio/gpio_nrfx.c`

```c
static const struct gpio_driver_api gpio_nrfx_drv_api_funcs = {
  .pin_configure = gpio_nrfx_pin_configure,
  // --snip--
};

// --snip--
#define GPIO_NRF_DEVICE(id)                 \
  /* --snip-- */                            \
  DEVICE_DT_INST_DEFINE(id, gpio_nrfx_init, \
      /* --snip-- */                        \
      &gpio_nrfx_drv_api_funcs);

DT_INST_FOREACH_STATUS_OKAY(GPIO_NRF_DEVICE)
```

That's it, as promised I was quick about that one. That's the magic behind the device driver abstractions in Zephyr. With that, you'll have an easy time diving into the [device driver model](https://docs.zephyrproject.org/latest/kernel/drivers/index.html) and [instance-based APIs](https://docs.zephyrproject.org/latest/build/dts/api/api.html#devicetree-inst-apis).



## Dissecting UART peripheral pin multiplexing

With `&led0` we've seen that it's quite straightforward to use GPIOs in Zephyr. Changing the GPIO pin, e.g., of an LED node, can be as easy as changing the corresponding phandle's pin specifier. We'll now look at how pin assignments work for _peripherals_. For that, we'll use our old friend _UART_.

We've learned that UART is used for the console output and thus for `printk`, so let's use some `printk` calls to make use of it. We'll output `"tick"` each time the LED is turned on, and `"tock"` each time it is turned off:

```c
void main(void)
{
    int err   = 0;
    bool tick = true;

    if (!gpio_is_ready_dt(&led))
    {
        printk("Error: LED pin is not available.\n");
        return;
    }

    err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (err != 0)
    {
        printk("Error %d: failed to configure LED pin.\n", err);
        return;
    }

    while (1)
    {
        (void) gpio_pin_toggle_dt(&led);
        k_msleep(SLEEP_TIME_MS);

        if (tick != false) { printk("Tick\n"); }
        else { printk("Tock\n"); }
        tick = !tick;
    }
}
```

> **Note:** If you've been following along, make sure to set the UART node's status back to `"okay"` in case you've disabled it in the last section.


### Peripheral pin control in Zephyr

To see how pins are assigned to our UART peripheral, we need to look into our board's DTS file. There, we find `pinctrl-<x>` properties for the `&uart0` node:

`zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts`

```dts
&uart0 {
  compatible = "nordic,nrf-uarte";
  status = "okay";
  current-speed = <115200>;
  pinctrl-0 = <&uart0_default>;
  pinctrl-1 = <&uart0_sleep>;
  pinctrl-names = "default", "sleep";
};
```

Before we look at how `pinctrl` works, you might ask yourself why we need another mechanism when we have `gpios`, and what the difference between the two is. Couldn't we just use `gpios` and, e.g., pass _phandles_ for all the pins we need for our UART interface?

For some MCUs, like Nordic's nRF series, this might work since pin multiplexing for such MCUs is not restricted: On Nordic's MCUs it is possible to assign any functionality to any pin. Other MCUs (e.g., STM32) typically restrict this and clearly define possible alternate pin functions for all pins.

MCU manufacturers can be quite creative when it comes to pin multiplexing, and before Zephyr 3.0, pin multiplexing was entirely vendor-specific. Zephyr 3.0 adopted the [Linux `pinctl`](https://www.kernel.org/doc/html/v4.13/driver-api/pinctl.html) concept as a _standardized_ way of assigning peripheral functions to pins in Devicetree - and called it `pinctrl`.

There is, of course, still an overlap between `gpios` and `pinctrl` since both associate nodes with pins and their parameters (e.g., pull resistors). As a rule of thumb, `pinctrl` is generally used if the pin goes to a **peripheral** in the `/soc` node, whereas `gpios` are used if the pin is used directly by the application.

#### Pin control concepts

[Zephyr's official documentation](https://docs.zephyrproject.org/latest/hardware/pinctrl/index.html) distinguishes between two general concepts: _Distributed_ and _centralized_ pin control. The difference between the two concepts is essentially defined by which device decides about possible pin assignments, as we've seen before:

**Distributed pin control**

Some MCUs, such as the nRF series from Nordic, do not restrict pin assignments, and the pin assignment is defined by each _peripheral_. E.g., the UART node selects whichever pins it uses for RXD, TXD, CTS, and RTS. The pin control is thus _distributed_ across all peripherals; there is no centralized pin multiplexer.

**Centralized pin control**

Other MCUs, such as the STM32, do not allow an arbitrary pin assignment. Instead, pins support alternative functions, managed by a centralized pin multiplexer which assigns the pin to a certain _peripheral_.

**Devicetree approach**

Whether an MCU uses centralized or distributed pin control does not necessarily have an impact on how the pin control is reflected in the Devicetree. Zephyr supports two approaches in _Devicetree_ for pin multiplexing: **Grouped** and **node** approach.

- In the **node approach** the vendor provides a DTS file containing dedicated nodes for **all** pins and the supported alternative functions. The provided nodes are referenced **unmodified** by the `pinctrl` properties of the peripheral. This is mostly used for MCUs with centralized pin control and fixed alternative functions and we'll see a practical example when browsing the [STM32 Devicetree source files](#node-approach-with-the-stm32).

- In the **grouped approach** the vendor's Devicetree sources do not provide nodes for all possible combinations. Instead, nodes containing the pin configuration are created either for the board or by the application, and pins are grouped by their configuration (e.g., pull resistors), thus the name "grouped". The pin multiplexing may or may not be restricted and thus this approach is for both, distributed and centralized pin multiplexing. We'll see this when we browse the [Nordic Devicetree source files in the next section](#node-approach-with-the-stm32).

Neither concept or approach is "better or worse", it is just what's implemented by hardware. As mentioned, MCU manufacturers can be very, very creative, and thus pin control will probably always remain vendor-specific.

### Basics and grouped pin control with the nRF52840

Let's bring up the `&uart0` node in the nRF52840 development kit's DTS file to save you some scrolling:

#### Pin control basics

`zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts`

```dts
&uart0 {
  compatible = "nordic,nrf-uarte";
  status = "okay";
  current-speed = <115200>;
  pinctrl-0 = <&uart0_default>;
  pinctrl-1 = <&uart0_sleep>;
  pinctrl-names = "default", "sleep";
};
```

The first thing we notice are the two properties `pinctrl-0` and `pinctrl-1`. The property names don't really give a hint about their semantics. You could, of course, look at the matching binding `zephyr/dts/bindings/pinctrl/pinctrl-device.yaml`, but in this case, the meaning of these properties is better described in [Zephyr's official documentation about pin control](https://docs.zephyrproject.org/latest/hardware/pinctrl/index.html):

Each device can have multiple _pin controller_ **states**. The names of the supported states are listed in the `pinctrl-names` property, in this case `"default"` and `"sleep"`. Both states are standard states defined by Zephyr for when the device is operational and in its sleep mode (if supported).

Whichever states are supported by a device is defined by its _driver_. Therefore, you can't, e.g., simply add another `pinctrl-2` and add a new name to `pinctrl-names` in an overlay and expect your application to compile or work. The _device driver_ must support multiple states for you to be able to use them.

> **Note:** We won't go into detail about how a device driver switches modes or _dynamic pin control_ in this article. Also, the `"sleep"` mode is described in more detail in Zephyr's official documentation about the [power management OS service](https://docs.zephyrproject.org/latest/services/pm/index.html). E.g., the `"sleep"` mode can be disabled using the _Kconfig_ symbol [`PM_DEVICE`](https://docs.zephyrproject.org/latest/kconfig.html#CONFIG_PM_DEVICE). For now, it is enough to know that such states exist and how we can configure them.

As the names of the _phandles_ in the `pinctrl-<n>` suggest, `pinctrl-<n>` is the pin control configuration for device state with index `n` in the property `pinctrl-names`: `pinctrl-0` is used for the operational state `"default"`, and `pinctrl-1` is used for the sleep state `"sleep"`.

Except for a list of pre-defined properties that we'll see in just a bit, this is it for the standardized part of pin control! The structure of the referenced nodes is entirely vendor-specific, though vendors typically at least either follow the _grouping_ or _node approaches_ that we've mentioned before.

#### Grouped pin control

We can find the referenced nodes `&uart0_default` and `&uart0_sleep` in the matching `-pinctrl.dtsi` DTS include file of the board:

`zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840-pinctrl.dtsi`

```dts
&pinctrl {
  uart0_default: uart0_default {
    group1 {
      psels = <NRF_PSEL(UART_TX, 0, 6)>, <NRF_PSEL(UART_RTS, 0, 5)>;
    };
    group2 {
      psels = <NRF_PSEL(UART_RX, 0, 8)>, <NRF_PSEL(UART_CTS, 0, 7)>;
      bias-pull-up;
    };
  };
  uart0_sleep: uart0_sleep {
    group1 {
      psels =
        <NRF_PSEL(UART_TX, 0, 6)>,  <NRF_PSEL(UART_RX, 0, 8)>,
        <NRF_PSEL(UART_RTS, 0, 5)>, <NRF_PSEL(UART_CTS, 0, 7)>;
      low-power-enable;
    };
  };
};
```

The above is an example of the _grouping approach_ for pin control:

Pins with common _properties_ are _grouped_ into child nodes with the name `group<m>`. E.g., the pins configured in `/pinctrl/uart0_default/group2` all use the `bias-pull-up` property to indicate that pull-up resistors are enabled for all pins in `psels`. Notice that the index `m` has nothing to do with the corresponding `pinctrl-<n>` properties of the peripheral node; it is simply used as _group_ index. All groups _combined_ contain the complete pin configuration for the state where they're referenced, e.g., _"default"_ or _"sleep"_.

> **Note:** Naming child nodes of `pinctrl` using the format `group<m>` is only a convention for the _grouped approach_. In fact, for most drivers that follow this _grouped approach_ the actual node names within the `pinctrl` node is irrelevant: The drivers simply iterate over all child nodes regardless of their names.

Within each group, Nordic uses a vendor-specific format and thus also vendor-specific binding. We can find the referenced node `&pinctrl` in the included `nrf_common.dtsi` file, where the compatible binding is defined:

`zephyr/dts/arm/nordic/nrf_common.dtsi`

```dts
/ {
  pinctrl: pin-controller {
    compatible = "nordic,nrf-pinctrl";
  };
};
```

Within the `nordic,nrf-pinctrl.yaml` binding, we find a `child-binding` property to define the child nodes of `nordic,nrf-pinctrl`:

`zephyr/dts/bindings/pinctrl/nordic,nrf-pinctrl.yaml`

```yaml
compatible: "nordic,nrf-pinctrl"
include: base.yaml

child-binding:
  child-binding:
    include:
      - name: pincfg-node.yaml
        property-allowlist:
          - bias-disable
          - bias-pull-down
          - bias-pull-up
          - low-power-enable

    properties:
      psels:
        required: true
        type: array
        description: |
          An array of pins sharing the same group properties. The pins should
          be defined using the NRF_PSEL utility macro that encodes the port,
          pin and function. NRF_PSEL_DISCONNECTED is also available to explicitly
          disconnect a pin.

      # --snip-- more nordic-specific properties
```

The property `psels` is specific to Nordic MCUs and - as documented - you're supposed to use the `NRF_PSEL` macro to create an entry in the `psels` array for all pins that you're using. This macro (defined in `zephyr/include/zephyr/dt-bindings/pinctrl/nrf-pinctrl.h`) takes the pin _function_, _port_, and _pin_ as a parameter.

Other vendors use an entirely different set of properties and macros for their Devicetree nodes and values. E.g., _Espressif_ uses the property `pinmux` instead of `psels` for the ESP32, as specified by `espressif,esp32-pinctrl.yaml`:

`zephyr/boards/xtensa/esp_wrover_kit/esp_wrover_kit-pinctrl.dtsi`

```dts
&pinctrl {
  uart0_default: uart0_default {
    group1 {
      pinmux = <UART0_TX_GPIO1>;
    };
    group2 {
      pinmux = <UART0_RX_GPIO3>;
      bias-pull-up;
    };
  };
};
```

ESP32 provides a fixed [set of macros for their bindings](https://github.com/zephyrproject-rtos/hal_espressif/blob/zephyr/include/dt-bindings/pinctrl) as thoroughly documented in their [`pinctrl` index page](https://docs.zephyrproject.org/latest/build/dts/api/bindings/pinctrl/espressif%2Cesp32-pinctrl.html#dtbinding-espressif-esp32-pinctrl): On the ESP32 a particular I/O pin does not necessarily have a certain pin function. Instead of providing a parameterized macro, the fixed macros `UART0_TX_GPIO1` and `UART0_RX_GPIO3` are used to configure the GPIO and function for the corresponding pins.

But let's get back to our `&uart0` node on the nRF52840 development kit. We've now seen `psels`, but what is this `pincfg-node.yaml` include?

#### Standard pin control properties

The binding `pincfg-node.yaml` contains **standardized** pin properties that _should_, in turn, be used by a vendor's `pinctrl`. E.g., properties for the pull resistor configuration, low-power modes, slew rates, etc.

`zephyr/dts/bindings/pinctrl/pincfg-node.yaml`

```yaml
properties:
  bias-disable:
    type: boolean
    description: disable any pin bias
  # --snip--
  bias-pull-up:
    type: boolean
    description: enable pull-up resistor
  bias-pull-down:
    type: boolean
    description: enable pull-down resistor
  # --snip--
  low-power-enable:
    type: boolean
    description: enable - mode
  # --snip--
```

Each vendor can, in turn, restrict which properties are supported by MCU pins using the `property-allowlist`. The following is the matching snippet from the nRF52840 DTS file that we've seen before:

```yaml
child-binding:
  child-binding:
    include:
      - name: pincfg-node.yaml
        property-allowlist:
          - bias-disable
          - bias-pull-down
          - bias-pull-up
          - low-power-enable
```


### Node approach with the STM32

Having seen the _grouped approach_, let's see how the _node approach_ is applied for the STM32 by looking at the
[STM32 Nucleo-64 development board](https://www.st.com/en/evaluation-tools/nucleo-c031c6.html)'s DTS file:

`zephyr/boards/arm/nucleo_c031c6/nucleo_c031c6.dts`

```dts
&usart2 {
  pinctrl-0 = <&usart2_tx_pa2 &usart2_rx_pa3>;
  pinctrl-names = "default";
  current-speed = <115200>;
  status = "okay";
};
```

Since the `pinctrl-<x>` properties are predefined by Zephyr, the format of the above snippet looks very familiar: The `pinctrl-names` only contains the operational mode `"default"` - it thus seems like the `"sleep"` mode is not supported (yet) by the STM32 drivers. Therefore, only `pinctrl-0` is provided, which now contains _two_ references instead of one.

Aside from the fact that we're using **two** _phandles_, the node names already indicate that this is a node-based approach with fixed pin multiplexing. This is especially clear when comparing the node names to the _alternate function mapping_ table in the datasheet:

- Pin _PA2_ has the alternative function `AF1=USART2_TX`, matching `usart2_tx_pa2`
- Pin _PA3_ has the alternative function `AF1=USART2_RX`, matching `usart2_rx_pa3`

The content in the matching DTS file in the [STM32 HAL](https://github.com/zephyrproject-rtos/hal_stm32) specifies just that:

`hal_stm32/dts/st/c0/stm32c031c(4-6)tx-pinctrl.dtsi`

```dts
/ {
  soc {
    pinctrl: pin-controller@50000000 {
      /omit-if-no-ref/ usart2_tx_pa2: usart2_tx_pa2 {
        pinmux = <STM32_PINMUX('A', 2, AF1)>;
        bias-pull-up;
      };
      /omit-if-no-ref/ usart2_rx_pa3: usart2_rx_pa3 {
        pinmux = <STM32_PINMUX('A', 3, AF1)>;
      };
    };
  };
};
```

> **Note:** If you're also using the Nordic toolchain installer to install Zephyr, you won't find the `stm32c031c(4-6)tx-pinctrl.dtsi` locally, since `hal_stm32` is not included in the installation. Check the [STM32 HAL on GitHub](https://github.com/zephyrproject-rtos/hal_stm32) until we solve this problem in the next article.

> **Note:** `/omit-if-no-ref/` is used to tell the Devicetree generator that no code should be generated for the node in case it is not referenced in the Devicetree. This avoids adding unnecessary content to the Devicetree since `pinctrl` nodes are only used when referenced.

On the STM32, the RX and TX functionality for `USART2` can only be selected via a pin's alternative function and is therefore not available on all pins. Instead of providing macros for all combinations, in the node approach, you'll find predefined _nodes_ for all pins and functions, e.g., for `USART2_TX` the following nodes exist in `stm32c031c(4-6)tx-pinctrl.dtsi`.

- `usart2_tx_pa2`
- `usart2_tx_pa4`
- `usart2_tx_pa8`
- `usart2_tx_pa14`

The `USART2_TX` functionality is therefore only available on the pins _PA2_, _PA4_, _PA8_ and _PA14_, as specified in the datasheet. The order in which the pins are assigned to `pinctrl-<n>` is again irrelevant since the pin's configuration determines its function. E.g., selecting `AF1` for the pin _PA2_ means that this pin is used for UART-TX of the USART2 peripheral.

As stated in [Zephyr's official documentation about pin control](https://docs.zephyrproject.org/latest/hardware/pinctrl/index.html), the _node approach_ method is discouraged if autogeneration is not an option. Therefore, you should really only encounter this pattern for MCUs where predefined nodes exist for all possible pin/function combinations.

This also means, that for MCUs using the _node approach_ you should **never** attempt to change the properties of the `pinctrl` nodes, e.g., in your overlay files. When mapping a peripheral to a different set of pins, you should instead _change_ the node **references** in the `pinctrl-<n>` property.


### Summary

Zephyr streamlines pin multiplexing by requiring the use of `pinctrl` properties for SoC peripherals and by providing a standardized set of pin configuration properties.

The nodes referenced in the `pinctrl-<n>` phandle arrays, however, are entirely vendor-specific and will probably remain so in the foreseeable future. Two patterns for dealing with `pinctrl` nodes exist: The **grouping** and the **node** approach.

- Pin control with the **grouping approach** is the recommended approach and uses macros for the pin assignment. MCUs with restricted pin multiplexing typically provide macros for supported pins and functions, whereas MCUs with unrestricted multiplexing such as Nordics nRF series use parameterized macros.

- Pin control with the **node approach** is only used for MCUs with restricted pin multiplexing: For each supported combination of pins and functions, the vendor provides a DTS file with generated nodes. Pins are assigned to a peripheral by referring to different (generated) nodes.

You can explore the pin control for your vendor of choice by looking it up in [Zephyr's bindings index](https://docs.zephyrproject.org/latest/build/dts/api/bindings.html).



## Devicetree and `vscode`

If you like `vscode`, you can also have a look at Nordic's [nRF DeviceTree extension](https://marketplace.visualstudio.com/items?itemName=nordic-semiconductor.nrf-devicetree). It includes Devicetree language support and even a visual Devicetree editor in case you're using [Nordic](https://www.nordicsemi.com/)'s MCUs.

![]({% img_url practical-zephyr/nrf-vscode-dt.png %})

While I haven't used the visual editor, it is also able to visualize multiple assignments for the same GPIO within the Devicetree. The extension is very useful when browsing Devicetree source files.



## Rerouting UART

With what we've learned about [grouped pin control with the nRF52840](#basics-and-grouped-pin-control-with-the-nrf52840), let's reroute our `&uart0` instance to pins that are available on the development kit's pin rows instead of using the pins that lead to the USB interface.

E.g., let's use the pins _P1.6_ and _P1.8_ instead. There are two ways to do this: Either we define our own nodes and assign them to the `pinctrl-<n>` properties of the `&uart0` node, or we overwrite the `psels` of the existing `&uart0_default` and `&uart0_default` nodes. For MCUs using the grouping approach, there's really no difference at all, and since it is easier, we'll just overwrite the existing nodes in our overlay as follows:

`04_practice/boards/nrf52840dk_nrf52840.overlay`

```dts
&uart0_default {
  group1 {
    psels = <NRF_PSEL(UART_TX, 1, 6)>, <NRF_PSEL_DISCONNECTED(UART_RTS)>;
  };
  group2 {
    psels = <NRF_PSEL(UART_RX, 1, 8)>, <NRF_PSEL_DISCONNECTED(UART_CTS)>;
    bias-pull-up;
  };
};
&uart0_sleep {
  group1 {
    psels =
      <NRF_PSEL(UART_TX, 1, 6)>,  <NRF_PSEL(UART_RX, 1, 8)>,
      <NRF_PSEL_DISCONNECTED(UART_RTS)>, <NRF_PSEL_DISCONNECTED(UART_CTS)>;
    low-power-enable;
  };
};
```

Notice that I've also disabled the RTS and CTS since we're not really using it. Since I don't have a working [FDTI](https://ftdichip.com/) cable, I'll be using my completely overqualified but trusted [Saleae](https://www.saleae.com) and connect it to the pins _P1.6_ and _P1.8_.

![]({% img_url practical-zephyr/kit-saleae.jpg %})

And it works!

![]({% img_url practical-zephyr/saleae-uart.png %})

Also, the output for the J-Link device remains empty, meaning that we really rerouted our UART node. A bit more intricate than reassigning GPIOs, but just as flexible.


### Switching to the STM32 Nucleo-64 development board

Since I have a spare [STM32 Nucleo-64 development board](https://www.st.com/en/evaluation-tools/nucleo-c031c6.html) at hand, it'd be nice to repeat the same steps and reroute the UART output for an MCU that uses the _pin approach_. Before we try to reroute anything, let's try to build the project for the matching board `nucleo_c031c6`:

```bash
$ rm -rf ../build
$ west build --board nucleo_c031c6 --build-dir ../build
```

```
In file included from <command-line>:
/opt/nordic/ncs/v2.4.0/zephyr/boards/arm/nucleo_c031c6/nucleo_c031c6.dts:9:10: fatal error: st/c0/stm32c031c(4-6)tx-pinctrl.dtsi: No such file or directory
    9 | #include <st/c0/stm32c031c(4-6)tx-pinctrl.dtsi>
      |          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
compilation terminated.
```

Well, that's embarrassing. Something's wrong with our setup, we don't seem to have all the files we need! At least this is what happens if you've followed my advice to use Nordic's toolchain installer in the very first article of this series. What's the reason for this error?

The reason is quite simple: Obviously, Nordic's toolchain installer assumes that you're only using their MCUs and the _nRF Connect SDK_, and not "vanilla Zephyr". It therefore doesn't include HALs and files from other vendors that are not required.

When using multiple MCUs it is therefore much better to stop using installed sources and use a _west workspace_ application instead. But that's a topic for the next and final article of this series. See? Embedded articles can have cliffhangers too!



## Conclusion

In this article, we made use of what we've learned about the Devicetree and its bindings and had a detailed look at advanced Devicetree concepts in Zephyr. I really hope that this article made up for all the theory that I've put you through!

Some of you might have expected a bit more of a "practice" article, but looking at APIs and drivers in Zephyr, I see no point in writing an article when there are so many [samples and demos](https://docs.zephyrproject.org/latest/samples/index.html) that do a much better job - and are maintained by the awesome Zephyr contributors.

Instead, we had a look at _some_ of Zephyr's APIs and device drivers to learn that _Devicetree_ is much more than just simple macro pasting for obtaining property values. The section on _pin control_ showed us a more advanced concept in Devicetree that you'll for sure need for even simple applications. At the same time, it also showed us that _Devicetree_ concepts in Zephyr are still evolving, since the `pinctrl` feature was not available prior to Zephyr 3.0 - and that's very good news!

> **Note:** The full example application including all the Devicetree files that we've seen throughout this article is available in the [`04_practice` folder of the accompanying GitHub repository](https://github.com/lmapii/practical-zephyr/tree/main/04_practice).



## Further reading

The following are great resources when it comes to Zephyr and are worth a read _or watch_:

- When trying to find out how a driver works, definitely have a look at Zephyr's [samples and demos](https://docs.zephyrproject.org/latest/samples/index.html). They're usually the best resource.
- If you want to know more about pin control, have a look at [Zephyr's official documentation](https://docs.zephyrproject.org/latest/hardware/pinctrl/index.html) and definitely watch the presentation [Deep Dive into Pin Control in Zephyr](https://www.youtube.com/watch?v=bcTekbGN-Pk) by Gerard Marull Paretas from the Zephyr Development Summit 2022.
- A more concise presentation about [How to use Zephyr Pin Control (pinctrl) for pin multiplexing and configuration](https://www.zephyrproject.org/how-to-use-zephyr-pin-control-pinctrl-for-pin-multiplexing-and-configuration/) is available on the Zephyr blog.
- If you want to know more about driver development, watch the [Tutorial: Mastering Zephyr Driver Development](https://www.youtube.com/watch?v=o-f2qCd2AXo) by Gerard Marull Paretas from the Zephyr Development Summit 2022.
- There's also a blog post on this Interrupt blog about [building drivers on Zephyr](https://interrupt.memfault.com/blog/building-drivers-on-zephyr).

Finally, have a look at the files in the [accompanying GitHub repository](https://github.com/lmapii/practical-zephyr/tree/main/04_practice) and I hope I'll see you again in the next article!


<!-- References

[nordicsemi]: https://www.nordicsemi.com/
[nordicsemi-dev-academy]: https://academy.nordicsemi.com/
[nordicsemi-nrf52840-dk]: https://www.nordicsemi.com/Products/Development-hardware/nrf52840-dk
[nordicsemi-ppk]: https://www.nordicsemi.com/Products/Development-hardware/Power-Profiler-Kit-2

[stm-nucleo]: https://www.st.com/en/evaluation-tools/nucleo-c031c6.html
[esp32-dt-bindings]: https://github.com/zephyrproject-rtos/hal_espressif/blob/zephyr/include/dt-bindings/pinctrl
[linux-pinctl]: https://www.kernel.org/doc/html/v4.13/driver-api/pinctl.html

[zephyr-samples-and-demos]: https://docs.zephyrproject.org/latest/samples/index.html
[zephyr-samples-blinky]: https://docs.zephyrproject.org/latest/samples/basic/blinky/README.html
[zephyr-samples-blinky-main]: https://github.com/zephyrproject-rtos/zephyr/blob/main/samples/basic/blinky/src/main.c

[zephyr-api-gpio]: https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html#gpio-api
[zephyr-blog-howto-pinctrl]: https://www.zephyrproject.org/how-to-use-zephyr-pin-control-pinctrl-for-pin-multiplexing-and-configuration/
[zephyr-hal-stm32]: https://github.com/zephyrproject-rtos/hal_stm32

[zephyr-dts-api-instance]: https://docs.zephyrproject.org/latest/build/dts/api/api.html#devicetree-inst-apis
[zephyr-dts-important-properties]: https://docs.zephyrproject.org/2.7.5/guides/dts/intro.html#important-properties
[zephyr-dts-bindings-index]: https://docs.zephyrproject.org/latest/build/dts/api/bindings.html

[zephyr-app-repository]: https://docs.zephyrproject.org/latest/develop/application/index.html#zephyr-repo-app
[zephyr-app-freestanding]: https://docs.zephyrproject.org/latest/develop/application/index.html#zephyr-freestanding-app
[zephyr-app-workspace]: https://docs.zephyrproject.org/latest/develop/application/index.html#zephyr-workspace-app

[zephyr-pinctrl]: https://docs.zephyrproject.org/latest/hardware/pinctrl/index.html
[zephyr-os-services]: https://docs.zephyrproject.org/latest/services/index.html
[zephyr-peripherals]: https://docs.zephyrproject.org/latest/hardware/peripherals/index.html
[zephyr-drivers]: https://docs.zephyrproject.org/latest/kernel/drivers/index.html
[zephyr-pm]: https://docs.zephyrproject.org/latest/services/pm/index.html
[zephyr-kconfig-pm]: https://docs.zephyrproject.org/latest/kconfig.html#CONFIG_PM_DEVICE

[zephyr-ds-2022-driver-dev]: https://www.youtube.com/watch?v=o-f2qCd2AXo
[zephyr-ds-2022-pinctrl]: https://www.youtube.com/watch?v=bcTekbGN-Pk

[interrupt-drivers-on-zephyr]: https://interrupt.memfault.com/blog/building-drivers-on-zephyr

-->
