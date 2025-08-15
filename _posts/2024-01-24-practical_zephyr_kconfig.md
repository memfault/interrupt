---
title: Practical Zephyr - Kconfig (Part 2)
description: Article series "Practical Zephyr", the second part covering the kernel configuration system Kconfig.
author: lampacher
tags: [zephyr, kconfig, practical-zephyr-series, nordic]
---

<!-- excerpt start -->

In this second article of the "Practical Zephyr" series, we'll explore the [kernel configuration system Kconfig](https://docs.zephyrproject.org/latest/build/kconfig/index.html#configuration-system-kconfig) by looking at the `printk` logging option in Zephyr. We won't explore the logging service as such in detail but instead use it as an excuse to dive deep into [Kconfig](https://docs.zephyrproject.org/latest/build/kconfig/index.html#configuration-system-kconfig). Finally, we'll create our own little application-specific _Kconfig_ configuration.

<!-- excerpt end -->

👉 Find the other parts of the *Practical Zephyr* series [here](/tags#practical-zephyr-series).

> 🎬
> [Listen to Martin on Interrupt Live](https://www.youtube.com/watch?v=ls_Y45WsTiA?feature=share)
> talk about the content and motivations behind writing this series.

{% include newsletter.html %}

{% include toc.html %}

## Prerequisites

This article is part of an article _series_. In case you haven't read the [previous article]({% post_url 2024-01-10-practical_zephyr_basics %}), please go ahead and have a look since it explains the required installation steps to follow along. If you're already familiar with Zephyr and have a working installation handy, it should be easy to follow using your own setup.

> **Note:** A full example application including all the configuration files that we'll see throughout this article is available in the [`01_kconfig` folder of the accompanying GitHub repository](https://github.com/lmapii/practical-zephyr/tree/main/01_kconfig).

## Warm up with `printk`

Go ahead and create a new, empty freestanding application, e.g., using the application from the [previous article]({% post_url 2024-01-10-practical_zephyr_basics %}).

```bash
$ tree --charset=utf-8 --dirsfirst
.
├── src
│   └── main.c
├── CMakeLists.txt
└── prj.conf
```

The simplest way to log text in Zephyr is the `printk` function. `printk` can take multiple arguments and format strings, but that's not what we want to show here, so we'll just modify the application's `main` function to output the string _"Message in a bottle."_ each time it is called - and thus after each reset:

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

But where does our output end up?

As mentioned in the previous article, we're using a development kit from [Nordic](https://www.nordicsemi.com/); in my case, the [development kit for the nRF52840](https://www.nordicsemi.com/Products/Development-hardware/nrf52840-dk). For most development kits, the logging output is available via its UART interface and the following settings:

* Baud rate: _115200_ bits/s
* Data bits: _8_
* No parity, one stop bit

The connection settings for the UART interface are configured using [devicetree](https://docs.zephyrproject.org/latest/build/dts/index.html) - which we'll explore in a later article. Since these default settings are all we need for now, let's focus on Kconfig. We build and flash the project as follows:

```bash
$ west build --board nrf52840dk_nrf52840 --build-dir ../build
$ west flash --build-dir ../build
```

When connecting the development kit to my computer, it shows up as TTY device `/dev/tty.usbmodem<some-serial-number>`. The format may vary depending on your development kit and operating system, e.g., on Windows, it'll show up as a `COM` port.

Open your serial monitor of choice, e.g., using the `screen` command, by launching [PuTTY](https://www.putty.org/) or - in case you're using _macOS_ - maybe you're also a fan of [Serial](https://www.decisivetactics.com/products/serial/), and reboot or reflash your device. You should see the following output each time your device starts:

```
$ screen /dev/tty.usbmodem<some-serial-number> 115200
*** Booting Zephyr OS build v3.3.99-ncs1 ***
Message in a bottle.

$ # use CTRL+A+k to quit the screen command.
```

Great! But what if we'd like to get rid of this feature entirely without changing our code? We all know that string processing is expensive and slow on an embedded system. Let's assume that in a production build, we want to get rid of any logging output to reduce our image size and speed up our application. How do we achieve this? This is where the [kernel configuration system _"Kconfig"_](https://docs.zephyrproject.org/latest/build/kconfig/index.html#configuration-system-kconfig) comes into play.

## Kconfig

We've seen that Zephyr uses [CMake](https://docs.zephyrproject.org/latest/build/cmake/index.html) as one of its [build and configuration systems](https://docs.zephyrproject.org/latest/build/index.html). On top of CMake - just like the Linux kernel - Zephyr uses the [kernel configuration system "Kconfig"](https://docs.zephyrproject.org/latest/build/kconfig/index.html#configuration-system-kconfig). We've already encountered Kconfig in the previous section, where we had to provide an empty _application configuration file_ [prj.conf](https://github.com/lmapii/practical-zephyr/blob/main/00_basics/prj.conf).

In this article, we'll explore the _practical_ aspects of Kconfig. As always, much more detailed information about Kconfig in Zephyr can be found in the [official documentation](https://docs.zephyrproject.org/latest/build/kconfig/index.html#configuration-system-kconfig). So, what is Kconfig?

In simple words, Kconfig is a configuration system that uses a hierarchy of configuration _files_ (with their own syntax), which in turn define a hierarchy of configuration _options_, also called _symbols_. The _build system uses these symbols_ to include or exclude files from the build process and as configuration options in the _source code_ itself. All in all, this allows you to modify your application without modifying the source code.

### Configuring symbols

For the time being, let's ignore _how_ the entire _Kconfig_ hierarchy is built or merged, and let's just have a look at how the configuration system works in general.

The debugging subsystem of Zephyr defines the _Kconfig_ symbol `PRINTK` that allows you to enable or disable the `printk` output. This `PRINTK` _symbol_ is defined in the `Kconfig` file `zephyr/subsys/debug/Kconfig`. We won't go into detail about the syntax here. It is quite self-explanatory for this example.

> For details about _Kconfig_'s syntax, you can have a look at the _Kconfig_ language specification in the [Linux kernel documentation](https://www.kernel.org/doc/html/latest/kbuild/kconfig-language.html) or Zephyr's [Kconfig tips and best practices](https://docs.zephyrproject.org/latest/build/kconfig/tips.html#kconfig-tips-and-best-practices).

`zephyr/subsys/debug/Kconfig`

```conf
config PRINTK
	bool "Send printk() to console"
	default y
```

As you can see from the above `Kconfig` file, unless configured otherwise, `PRINTK` is *enabled* by default. We can configure symbols to our needs in the _application configuration file_ [prj.conf](https://github.com/lmapii/practical-zephyr/blob/main/01_kconfig/prj.conf) that we've encountered in the previous article. Symbols are assigned their values using the following syntax:

```conf
CONFIG_<symbol name>=<value>
```

> **Note:**  At the time of writing, no spaces are allowed before or after the `=` operator.

Our `PRINTK` is a symbol of type `bool` and can be assigned the values `y` and `n` (have a look at the [official documentation](https://docs.zephyrproject.org/latest/build/kconfig/setting.html#setting-kconfig-configuration-values) for details). To disable `PRINTK`, we can therefore add the following to our application configuration file [prj.conf](https://github.com/lmapii/practical-zephyr/blob/main/01_kconfig/prj.conf):

```conf
CONFIG_PRINTK=n
```

What's the effect of changing a symbol? As mentioned before, this _can_ affect the build options and _can_ also be used by some subsystem's implementation. Whatever a symbol does, however, is specific for each symbol, meaning there is no convention for symbol names that always have the same effect. Thus, in the end, it is up to you to dig into the sources and documentation to see which options are available and what effect they have (there's a [Kconfig search](#kconfig-search) that we'll see later).

Let's have a short look at our `PRINTK` symbol. A quick search for `CONFIG_PRINTK` in our local Zephyr code reveals how the symbol is used in the implementation of the `printk` debugging subsystem. Notice that the below snippets are simplified versions of the real code:

`zephyr/lib/os/printk.h`

```c
#ifdef CONFIG_PRINTK
extern void printk(const char *fmt, ...);
#else
static inline void printk(const char *fmt, ...)
{
    ARG_UNUSED(fmt);
}
```

`zephyr/lib/os/printk.c`

```c
#ifdef CONFIG_PRINTK
void printk(const char *fmt, ...)
{
    // ...
}
#endif /* defined(CONFIG_PRINTK) */
```

> **Notice:** In our `prj.conf` file, we **set** `CONFIG_PRINTK=n`, but in the implementation, this translates to a missing definition, not a definition with a value of `n`. This is simply what a `bool` symbol translates to in the implementation. For details, I'll once again have to direct you to the [official documentation](https://docs.zephyrproject.org/latest/build/kconfig/setting.html#setting-kconfig-configuration-values).

In the header file `zephyr/lib/os/printk.h` we can see that `printk` is replaced by a dummy function in case `PRINTK` is disabled. Thus, all the calls to `printk` will effectively be removed by the compiler. In the `zephyr/lib/os/printk.c` source file, we can see that the `#if` directive is used to conditionally compile the relevant source code only in case `PRINTK` is enabled. Later in this section, we'll also see an example of how files can be excluded entirely by the build system if an option is not enabled.

With `printk` disabled in our application configuration file, let's try to rerun our application to verify that the output is indeed disabled:

```bash
$ west build --board nrf52840dk_nrf52840 --build-dir ../build --pristine
$ west flash --build-dir ../build
```

> **Notice:** In most cases when changing your _Kconfig_ symbols, you can simply run `west build` and the build system will rebuild the appropriate source. But in some cases, it is required to use the `--pristine` option to enforce a complete rebuild of your application. Typically, this is needed when adding new build or configuration files. If you encounter a build error or are missing a configuration change, try rebuilding with `--pristine`. This in turn also requires specifying the correct `--board` in case you didn't configure it using `west config`.

```
$ screen /dev/tty.usbmodem<some-serial-number> 115200
*** Booting Zephyr OS build v3.3.99-ncs1 ***
Message in a bottle.
```

Huh. Even though we've disabled `PRINTK` using `CONFIG_PRINTK=n` in our `prj.conf` file, the output is still not disabled. The build passed, and there were no warnings - did we miss something?

## Navigating Kconfig

As mentioned before, _Kconfig_ uses an entire _hierarchy_ of `Kconfig` files. The initial configuration is merged from several `Kconfig` files, including the `Kconfig` file of the specified board. The exact procedure used by Zephyr for merging the configuration file is explained in great detail in a [dedicated section of the official documentation](https://docs.zephyrproject.org/latest/build/kconfig/setting.html#the-initial-configuration) but is not worth repeating here.

> **Notice:** The command line output of `west build` also shows the order in which _Kconfig_ merges the configuration files. We'll see this in the section about [build types and alternative Kconfig files](#build-types-and-alternative-kconfig-files).

When debugging _Kconfig_ settings, it can be helpful to have a look at the generated output. All _Kconfig_ symbols are merged into a single `zephyr/.config` file located in the build directory, in our case, `build/zephyr/.config`. There, we find the following setting:

`build/zephyr/.config`

```conf
#
# Debugging Options
#
# CONFIG_DEBUG is not set
# CONFIG_STACK_USAGE is not set
# CONFIG_STACK_SENTINEL is not set
CONFIG_PRINTK=y
```

It seems that our `CONFIG_PRINTK` setting was not accepted. Why?

The reason is that `Kconfig` files can have _dependencies_: Enabling one option can automatically also enable other options. Sadly, at the time of writing, _Kconfig_ does not seem to warn about conflicting values for symbols, e.g., from the application configuration file: It'd be great if Kconfig would tell me that some other option overwrites my `CONFIG_PRINTK=n` setting and has no effect!

Zephyr contains _hundreds_ of so-called Kconfig _fragments_. It is therefore almost impossible to navigate these files without tool support. Thankfully, _West_ integrates two graphical tools to explore available _Kconfig_ options and their current setting: `menuconfig` and `guiconfig`.

### Loading `menuconfig` and `guiconfig`

For the `build` command, _West_ has some builtin _targets_, two of which are used for _Kconfig_:

```bash
$ west build --build-dir ../build -t usage
-- west build: running target usage
...
Kconfig targets:
  menuconfig - Update .config using a console-based interface
  guiconfig  - Update .config using a graphical interface
...
```

The targets `menuconfig` and `guiconfig` can be used to start graphical _Kconfig_ tools which allow modifying the `.config` file in your build directory. Both tools are essentially identical. The only difference is that `menuconfig` is a text-based graphical interface that opens directly in your terminal, whereas `guiconfig` is a "clickable" user interface running in its own window. The screenshots below show the tools launched by the following commands:

```bash
$ west build --build-dir ../build -t menuconfig
$ west build --build-dir ../build -t guiconfig
```

> **Notice:** If you did not build your project or the build system cannot build the required targets due to CMake/Kconfig processing issues, the tool will not load since the `zephyr/.config` file is missing.

![]({% img_url practical-zephyr/kconfig-menuconfig.png %})
![]({% img_url practical-zephyr/kconfig-guiconfig.png %})

For the sake of simplicity, we'll use `menuconfig` in the remainder of this article. In both tools, you can search for configuration symbols. In `menuconfig` - similar to `vim` - you can use the forward slash `/` to start searching, e.g., for our `PRINTK` symbol. Several results match, but only one of them is the plain symbol `PRINTK` which also shows the same description that we've seen in the `Kconfig` file:

```
PRINTK(=y) "Send printk() to console"
```

Navigate to the symbol using the search or the path _Subsystems and OS Services > Debugging Options_. You should see the following screen:

![]({% img_url practical-zephyr/kconfig-menuconfig-debug.png %})

Typically, you can toggle `bool` symbols using `Space` or `Enter`, but this does not seem to be possible for `PRINTK`. You can see that a symbol cannot be changed - which is indicated by the marker `*` being enclosed by dashes `-*-` instead of square brackets `[*]`. Using `?`, we can display the symbol's information:

```
Name: PRINTK
Prompt: Send printk() to console
Type: bool
Value: y

Help:

  This option directs printk() debugging output to the supported
  console device, rather than suppressing the generation
  of printk() output entirely. Output is sent immediately, without
  any mutual exclusion or buffering.

Default:
  - y

Symbols currently y-selecting this symbol:
  - BOOT_BANNER
...
```

We found the reason why we can't disable the symbol at the very bottom of the `PRINTK`'s symbol information! The symbol `BOOT_BANNER` is `y-selecting` the `PRINTK` symbol, so we can't disable it, and the application's configuration has no effect. This simple example should also make you realize that it is hard, if not impossible, to figure out dependencies by navigating `Kconfig` fragments file by file.

> **Notice:** If you can't find a Kconfig symbol in `menuconfig`, you can toggle the _Show all_ mode using `Shift+A`. This shows hidden Kconfig symbols - your symbol might be one of those!

Let's fix our configuration:

* Navigate to the `BOOT_BANNER` symbol and disable it.
* Going back to `PRINTK` it should now be possible to disable it.

### Persisting `menuconfig` and `guiconfig` changes

`menuconfig` and `guiconfig` can be used to test and explore configuration options. Typically, you'd use the _Save_ operation to store the new configuration to the `zephyr/.config` file in your build directory. You can then also launch `menuconfig` multiple times and change the configuration to your needs, and the changes will always be reflected in the next build.

The downside of saving the configuration to `zephyr/.config` in the build directory is that **all changes will be lost** once you perform a clean build, e.g., using `west build --pristine`. To persist your changes, you need to place them into your [configuration files](https://docs.zephyrproject.org/latest/build/kconfig/setting.html#setting-kconfig-configuration-values).

**Using `diff`**

The people at [Golioth](https://golioth.io/) published a [short article](https://blog.golioth.io/zephyr-quick-tip-show-what-menuconfig-changed-and-make-changes-persistent/) that shows how to leverage the `diff` command to find out which configuration options have changed when using the normal _Save_ operation: Before writing the changes to the `zephyr/.config` file, Kconfig stores the old configuration in `build/zephyr/.config.old`. Thus, all changes that you make between a _Save_ operation are reflected by the differences between `zephyr/.config` and `build/zephyr/.config.old`:

```bash
$ west build --board nrf52840dk_nrf52840 -d ../build --pristine
$ west build -d ../build -t menuconfig
# Within menuconfig:
# - Disable BOOT_BANNER and PRINTK.
# - Save the changes to ../build/zephyr/.config using [S].

$ diff ../build/zephyr/.config ../build/zephyr/.config.old
1097c1097
< # CONFIG_BOOT_BANNER is not set
---
> CONFIG_BOOT_BANNER=y
1462c1462
< # CONFIG_PRINTK is not set
---
> CONFIG_PRINTK=y
```

> **Notice:** The `# CONFIG_<symbol> is not set` comment syntax is used by `zephyr/.config` instead of `CONFIG_<symbol>=n` for historical reasons, as also mentioned in the [official documentation](https://docs.zephyrproject.org/latest/build/kconfig/setting.html#setting-kconfig-configuration-values). This allows parsing configuration files directly as `makefiles`.

You can now simply take the changes from the `diff` and persist them, e.g., by writing them into your application configuration file. Keep in mind that this only works for changes performed between **one** _Save_ operation. If you _Save_ multiple times, `.config.old` is always replaced by the current `.config`. The `diff` operation must therefore be performed after each _Save_.

This approach, however, is not always feasible. E.g., try to enable the logging subsystem _Subsystems and OS Services > Logging_: You'll notice that the `diff` is huge even though you've only changed one option. This is due to the large number of dependencies of the `CONFIG_LOG=y` setting.

**Save minimal configuration**

Both `menuconfig` and `guiconfig` have the _Save minimal config_ option. As the name implies, this option exports a minimal `Kconfig` file containing _all_ symbols that differ from their default values. This does, however, also include symbols that differ from their default values due to _other_ `Kconfig` fragments than your application's configuration. E.g., the symbols in the `Kconfig` files of the chosen board are saved in this minimal configuration file.

Let's try this out with our settings for the `BOOT_BANNER` and `PRINTK` symbols.

```bash
$ west build --board nrf52840dk_nrf52840 -d ../build --pristine
$ west build -d ../build -t menuconfig
# Within menuconfig:
# - Disable BOOT_BANNER and PRINTK.
# - Use [D] "Save minimal config" and write the changes to tmp.conf.
$ cat tmp.conf
```

```conf
CONFIG_GPIO=y
CONFIG_PINCTRL=y
CONFIG_SERIAL=y
CONFIG_NRFX_GPIOTE_NUM_OF_EVT_HANDLERS=1
CONFIG_USE_SEGGER_RTT=y
CONFIG_SOC_SERIES_NRF52X=y
CONFIG_SOC_NRF52840_QIAA=y
CONFIG_ARM_MPU=y
CONFIG_HW_STACK_PROTECTION=y
# CONFIG_BOOT_BANNER is not set
CONFIG_CONSOLE=y
CONFIG_UART_CONSOLE=y
# CONFIG_PRINTK is not set
CONFIG_EARLY_CONSOLE=y
```

As you can see, this minimal configuration contains a lot more options than we changed within `menuconfig`. Some of these options come from the selected board, in my case `zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840_defconfig`, but are still listed since they do not use default values. However, the two changes to `PRINTK` and `BOOT_BANNER` are still visible. We can just take those options and write them to our application configuration file:

```bash
$ cat prj.conf
CONFIG_PRINTK=n
CONFIG_BOOT_BANNER=n
```

Now, after rebuilding, the output on our serial interface indeed remains empty. Also, if you look at the output of your build should notice that the _flash_ usage decreased by around 2 KB.

### Kconfig search

In case you don't like `menuconfig` or `guiconfig`, or just want to browse available configuration options, Zephyr's documentation also includes a dedicated [Kconfig search](https://docs.zephyrproject.org/latest/kconfig.html). E.g., when [searching for our `PRINTK` configuration](https://docs.zephyrproject.org/latest/kconfig.html#CONFIG_PRINTK) we'll be presented with the following output:

![]({% img_url practical-zephyr/kconfig-search.png %})

While this search does point out that there is a dependency on the `BOOT_BANNER` symbol, it cannot know our current configuration and therefore can't tell us that disabling `PRINTK` won't have any effect since `BOOT_BANNER` is also still enabled by default.

### Nordic's Kconfig extension for VS Code

Another graphical user interface for _Kconfig_ is the [nRF Kconfig](https://marketplace.visualstudio.com/items?itemName=nordic-semiconductor.nrf-kconfig) extension for VS Code. This is an extension tailored for use with the [nRF Connect SDK](https://www.nordicsemi.com/Products/Development-software/nrf-connect-sdk) and with Nordic's [complete VS Code extension pack](https://nrfconnect.github.io/vscode-nrf-connect/), but to some extent, this extension can also be used for a generic Zephyr project and might therefore also be useful if you're not developing for a target from [Nordic](https://www.nordicsemi.com/):

![]({% img_url practical-zephyr/kconfig-nrf-vscode.png %})

At the time of writing and for this repository, the following configuration options were necessary to use the extension in a workspace that does not rely on the entire extension set:

```json
{
  "nrf-connect.toolchain.path": "${nrf-connect.toolchain:2.4.0}",
  "nrf-connect.topdir": "${nrf-connect.sdk:2.4.0}",
  "kconfig.zephyr.base": "${nrf-connect.sdk:2.4.0}",
  "nrf-connect.applications": ["${workspaceFolder}/path/to/app"]
}
```

> **Note:** Let's hope that the good people at Nordic find the time to change this extension so that it is easily usable for any Zephyr project!

However, as visible in the above screenshot, at the time of writing the extension fails to inform the user about the dependency to `BOOT_BANNER`, and in contrast to `menuconfig` and `guiconfig` it is therefore not visible why the configuration option cannot be disabled. Aside from that, this extension has two major benefits:

* The _Save minimal config_ option seems to recognize options coming from a `_defconfig` file and therefore really only exports the configuration options set within the extension.
* The extension adds auto-completion to your `.conf` files.

With this last tool to explore _Kconfig_, let's have a look at a couple of more options.

## Using different configuration files

Until now, we've only used the _application configuration file_ [prj.conf](https://github.com/lmapii/practical-zephyr/blob/main/00_basics/prj.conf) for setting _Kconfig_ symbols. In addition to this configuration file, Zephyr's build system automatically picks up additional _Kconfig_ _fragments_, if provided, and also allows explicitly specifying additional or alternative fragments.

We'll quickly glance through the most common practices here.

### Build types and alternative Kconfig files

Zephyr doesn't use pre-defined build types such as _Debug_ or _Release_ builds. It is up to the application to decide the optimization options and build types. Different build types in Zephyr are supported using [alternative `Kconfig` files](https://docs.zephyrproject.org/latest/develop/application/index.html#basics), specified using the [`CONF_FILE`](https://docs.zephyrproject.org/latest/develop/application/index.html#important-build-system-variables) build system variable.

Let's freshen up on the _build system variables_: These variables are _not_ direct parameters for the build and configuration systems used by Zephyr, e.g., the `--build-dir` option for `west build`, but are available globally. As described in the [official documentation on important build system variables](https://docs.zephyrproject.org/latest/develop/application/index.html#important-build-system-variables), there are multiple ways to pass such variables to the build system. We'll repeat two of the available options here:

1. A build system variable may exist as an environment variable.
2. A build system variable may be passed to `west build` as [a one-time CMake argument](https://docs.zephyrproject.org/latest/develop/west/build-flash-debug.html#one-time-cmake-arguments) after a double dash `--`.

We'll stick to the second option and use a [one-time CMake argument](https://docs.zephyrproject.org/latest/develop/west/build-flash-debug.html#one-time-cmake-arguments), as shown in the [application development basics in the official documentation](https://docs.zephyrproject.org/latest/develop/application/index.html#basics). First, let's go ahead and create a [`prj_release.conf`](https://github.com/lmapii/practical-zephyr/blob/main/01_kconfig/prj_release.conf), which defines our symbols for the `release` build type:

```
$ tree --charset=utf-8 --dirsfirst
.
├── src
│   └── main.c
├── CMakeLists.txt
├── prj.conf
└── prj_release.conf

$ cat prj_release.conf
```

```conf
CONFIG_LOG=n
CONFIG_PRINTK=n
CONFIG_BOOT_BANNER=n
CONFIG_EARLY_CONSOLE=n
CONFIG_OVERRIDE_FRAME_POINTER_DEFAULT=y
CONFIG_USE_SEGGER_RTT=n
CONFIG_BUILD_OUTPUT_STRIPPED=y
CONFIG_FAULT_DUMP=0
CONFIG_STACK_SENTINEL=y

# Optimize build for size
CONFIG_SIZE_OPTIMIZATIONS=y
```

For our release build, we set the symbol `CONFIG_SIZE_OPTIMIZATIONS` to `y` and thereby optimize our release build for _size_: [Build optimizations](https://docs.zephyrproject.org/latest/kconfig.html#CONFIG_COMPILER_OPTIMIZATIONS) are not set up in your _CMake_ configuration, but instead using _Kconfig_. Don't worry about all the other symbols in this file for now. We'll catch up on them in the section about [Kconfig hardening](#kconfig-hardening).

We can now instruct _West_ to use this `prj_release.conf` _instead_ of our `prj.conf` for the build by using the following command:

```bash
$ rm -rf ../build
$ west build --board nrf52840dk_nrf52840 -d ../build -- -DCONF_FILE=prj_release.conf
```

If you scroll through the output of the `west build` command, you'll notice that _Kconfig_ merged `prj_release.conf` into our final configuration:

```
Parsing /opt/nordic/ncs/v2.4.0/zephyr/Kconfig
Loaded configuration '/opt/nordic/ncs/v2.4.0/zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840_defconfig'
Merged configuration '/path/to/01_kconfig/prj_release.conf'
Configuration saved to '/path/to/zephyr_practical/build/zephyr/.config'
```

Thus, in short, Zephyr accepts build types by specifying an alternative `Kconfig` file that typically uses the file name format `prj_<build>.conf`, using the `CONF_FILE` build system variable. The name of the build type is entirely application-specific and does not imply any, e.g., compiler optimizations.

> **Notice:** Zephyr uses _Kconfig_ to specify build types and also optimizations. Thus, _CMake_ options such as [`CMAKE_BUILD_TYPE`](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html) are typically **not** used directly in Zephyr projects. This includes compiler optimization flags such as `-Os` or `-O2`, which are set using the [`CONFIG_COMPILER_OPTIMIZATIONS`](https://docs.zephyrproject.org/latest/kconfig.html#CONFIG_COMPILER_OPTIMIZATIONS) in Kconfig instead.

### Board-specific Kconfig fragments

Aside from the application configuration file [prj.conf](https://github.com/lmapii/practical-zephyr/blob/main/01_kconfig/prj.conf), Zephyr also automatically picks up board-specific Kconfig _fragments_. Board fragments are placed in the `boards` directory in the project root (next to the `CMakeLists.txt` file) and use the `<board>.conf` name format.

E.g., throughout this guide, we're using the nRF52840 development kit, which has the board name `nrf52840dk_nrf52840`. Thus, the file `boards/nrf52840dk_nrf52840.conf` is automatically merged into the `Kconfig` configuration during the build, if present.

Let's try this by disabling UART as console output using a new fragment `boards/nrf52840dk_nrf52840.conf`. For the nRF52840 development kit, this symbol is enabled by default. You can go ahead and verify this by checking the default configuration `zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840_defconfig` - or take my word for it:

```bash
tree --charset=utf-8 --dirsfirst
.
├── boards
│   └── nrf52840dk_nrf52840.conf
├── src
│   └── main.c
├── CMakeLists.txt
├── prj.conf
└── prj_release.conf

$ cat boards/nrf52840dk_nrf52840.conf
```

```conf
CONFIG_UART_CONSOLE=n
```

Then, we perform a _pristine_ build of the project. Notice that a `--pristine` build is required at this point because otherwise, the build system does not pick up our newly added `.conf` file. This is one of the very few occasions where a pristine build is actually required.

```bash
$ west build --board nrf52840dk_nrf52840 -d ../build --pristine
```

In the output of `west build` shown below you can see that the new file `boards/nrf52840dk_nrf52840.conf` is indeed merged into the `Kconfig` configuration. You can also see a warning that the Zephyr library `drivers__console` is excluded from the build since it now has no `SOURCES`:

```
Parsing /opt/nordic/ncs/v2.4.0/zephyr/Kconfig
Loaded configuration '/opt/nordic/ncs/v2.4.0/zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840_defconfig'
Merged configuration '/path/to/01_kconfig/prj.conf'
Merged configuration '/path/to/01_kconfig/boards/nrf52840dk_nrf52840.conf'
Configuration saved to '/path/to/build/zephyr/.config'
...
CMake Warning at /opt/nordic/ncs/v2.4.0/zephyr/CMakeLists.txt:838 (message):
  No SOURCES given to Zephyr library: drivers__console
  Excluding target from build.
```

You can of course verify that the symbol is indeed disabled by checking the `zephyr/.config` file in the `build` directory.

> **Notice:** Previous versions of Zephyr used a `prj_<board>.conf` fragment in the project root directory instead of board fragments in the `boards` directory. At the time of writing, the build system still merges any such file into the configuration. However, this `prj_<board>.conf` fragment is deprecated, and `boards/<board>.conf` should be used instead.

How are board fragments used by our previously created `release` build type? Let's try it out and have a look at the build output:

```bash
$ rm -rf ../build
$ west build --board nrf52840dk_nrf52840 -d ../build -- -DCONF_FILE=prj_release.conf
```

```
Parsing /opt/nordic/ncs/v2.4.0/zephyr/Kconfig
Loaded configuration '/opt/nordic/ncs/v2.4.0/zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840_defconfig'
Merged configuration '/path/to/01_kconfig/prj_release.conf'
Configuration saved to '/path/to/build/zephyr/.config'
```

Well, this is awkward: Even though we provided a board-specific `Kconfig` fragment, it is ignored as soon as we switch to our alternative `prj_release.conf` configuration. This is, however, [working as intended](https://docs.zephyrproject.org/latest/build/kconfig/setting.html#the-initial-configuration): When using alternative `Kconfig` files in the format `prj_<build>.conf`, Zephyr looks for a board fragment matching the file name `boards/<board>_<build>.conf`.

In my case, I need to create the file `boards/nrf52840dk_nrf52840_release.conf`. Let's get rid of the warning for the Zephyr library `drivers__console` by disabling the `CONSOLE` entirely in our board's release configuration:

```bash
tree --charset=utf-8 --dirsfirst
.
├── boards
│   ├── nrf52840dk_nrf52840.conf
│   └── nrf52840dk_nrf52840_release.conf
├── src
│   └── main.c
├── CMakeLists.txt
├── prj.conf
└── prj_release.conf

$ cat boards/nrf52840dk_nrf52840_release.conf
```

```conf
CONFIG_CONSOLE=n
CONFIG_UART_CONSOLE=n
```

Now, when we rebuild the project, we see that our board fragment is merged into the `Kconfig` configuration, and no warning is issued by `west build`:

```bash
$ rm -rf ../build
$ west build --board nrf52840dk_nrf52840 -d ../build -- -DCONF_FILE=prj_release.conf
```

```
Parsing /opt/nordic/ncs/v2.4.0/zephyr/Kconfig
Loaded configuration '/opt/nordic/ncs/v2.4.0/zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840_defconfig'
Merged configuration '/path/to/01_kconfig/prj_release.conf'
Merged configuration '/path/to/01_kconfig/boards/nrf52840dk_nrf52840_release.conf'
Configuration saved to '/path/to/build/zephyr/.config'
```

### Extra configuration files

There is one more option to modify the _Kconfig_, listed among the [important build system variables in Zephyr's official documentation](https://docs.zephyrproject.org/latest/develop/application/index.html#important-build-system-variables): `EXTRA_CONF_FILE`. This build system variable accepts one or more additional _Kconfig_ fragments. This option can be useful, e.g., to specify additional configuration options used by multiple build types (normal builds, "release" builds, "debug" builds) in a separate fragment. An example could be adding TLS configs with a `tls.conf` or adding external NOR configs with `qspi_nor.conf`

> **Note:** Since [Zephyr 3.4.0](https://docs.zephyrproject.org/latest/releases/release-notes-3.4.html) the `EXTRA_CONF_FILE` build system variable replaces the deprecated variable `OVERLAY_CONFIG`.

Let's add two _Kconfig_ fragments `extra0.conf` and `extra1.conf`:

```bash
$ tree --charset=utf-8 --dirsfirst
.
├── boards
│   ├── nrf52840dk_nrf52840.conf
│   └── nrf52840dk_nrf52840_release.conf
├── src
│   └── main.c
├── CMakeLists.txt
├── extra0.conf
├── extra1.conf
├── prj.conf
└── prj_release.conf

$ cat extra0.conf
CONFIG_DEBUG=n

$ cat extra1.conf
CONFIG_GPIO=n
```

> **Notice:** In an actual project, you'll need to pick better names for the extra configuration files, e.g., `no-debug.conf` and `no-gpio.conf`. I'm using `extra0.conf` and `extra1.conf` to explicitly show their use and to make the order in which the files are applied visible, as you'll see just now.

We can now pass the two extra configuration fragments to the build system using the `EXTRA_CONF_FILE` variable. The paths are relative to the project root and can either be separated using semicolons or spaces:

```bash
$ rm -rf ../build
$ west build --board nrf52840dk_nrf52840 -d ../build -- \
  -DEXTRA_CONF_FILE="extra0.conf;extra1.conf"
```

```
Parsing /opt/nordic/ncs/v2.4.0/zephyr/Kconfig
Loaded configuration '/opt/nordic/ncs/v2.4.0/zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840_defconfig'
Merged configuration '/path/to/01_kconfig/prj.conf'
Merged configuration '/path/to/01_kconfig/boards/nrf52840dk_nrf52840.conf'
Merged configuration '/path/to/01_kconfig/extra0.conf'
Merged configuration '/path/to/01_kconfig/extra1.conf'
Configuration saved to '/path/to/build/zephyr/.config'
```

The fragments specified using the `EXTRA_CONF_FILE` variable are merged into the final configuration in the given order. E.g., we can create a `release` build and reverse the order of the fragments:

```bash
$ rm -rf ../build
$ west build --board nrf52840dk_nrf52840 -d ../build -- \
  -DCONF_FILE="prj_release.conf" \
  -DEXTRA_CONF_FILE="extra1.conf;extra0.conf"
```

```
Parsing /opt/nordic/ncs/v2.4.0/zephyr/Kconfig
Loaded configuration '/opt/nordic/ncs/v2.4.0/zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840_defconfig'
Merged configuration '/path/to/01_kconfig/prj_release.conf'
Merged configuration '/path/to/01_kconfig/boards/nrf52840dk_nrf52840_release.conf'
Merged configuration '/path/to/01_kconfig/extra1.conf'
Merged configuration '/path/to/01_kconfig/extra0.conf'
Configuration saved to '/path/to/build/zephyr/.config'
```

## Kconfig hardening

Another tool worth mentioning when talking about _Kconfig_ is the [hardening tool](https://docs.zephyrproject.org/latest/security/hardening-tool.html). Just like `menuconfig` and `guiconfig`, this tool is a target for `west build`. It is executed using the `-t hardenconfig` target in the call to `west build`. The hardening tool checks the _Kconfig_ symbols against a set of known configuration options that should be used for a secure Zephyr application. It then lists all differences found in the application.

We can use this tool to check our normal and _release_ configurations:

```bash
$ # test normal build
$ west build --board nrf52840dk_nrf52840 \
  -d ../build \
  --pristine \
  -t hardenconfig

$ # test release build
$ west build --board nrf52840dk_nrf52840 \
  -d ../build \
  --pristine \
  -t hardenconfig \
  -- -DCONF_FILE=prj_release.conf
```

For the normal build using `prj.conf`, the hardening tool displays a table of all symbols whose current values do not match the recommended, secure configuration value. E.g., at the time of writing, this is the output for the normal build in my demo application:

```
-- west build: running target hardenconfig
[0/1] cd /path/to/zephyr/kconfig/hardenconfig.py /opt/nordic/ncs/v2.4.0/zephyr/Kconfig
                 name                 | current | recommended || check result
==============================================================================
CONFIG_OVERRIDE_FRAME_POINTER_DEFAULT |    n    |      y      ||     FAIL
CONFIG_USE_SEGGER_RTT                 |    y    |      n      ||     FAIL
CONFIG_BUILD_OUTPUT_STRIPPED          |    n    |      y      ||     FAIL
CONFIG_FAULT_DUMP                     |    2    |      0      ||     FAIL
CONFIG_STACK_SENTINEL                 |    n    |      y      ||     FAIL
```

The symbols in `prj_release.conf` have been chosen such that at the time of writing all hardening options are fulfilled and the above table is empty.

## A custom Kconfig symbol

In the previous sections, we examined _Kconfig_ in detail. To wrap up this article, we'll create and use a custom, application-specific symbol in our application _and_ build process.

> **Note:** This section borrows from the [nRF Connect SDK Fundamentals lesson on configuration files](https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/lessons/lesson-3-elements-of-an-nrf-connect-sdk-application/topic/configuration/) that is freely available in the [Nordic Developer Academy](https://academy.nordicsemi.com/). If you're looking for additional challenges, check out the available courses!

### Adding a custom configuration

[CMake automatically detects a `Kconfig` file](https://docs.zephyrproject.org/latest/develop/application/index.html#application-cmakelists-txt) if it is placed in the same directory of the application's `CMakeLists.txt`, and that is what we'll use for our own configuration file. If you want to place the `Kconfig` file somewhere else, you can customize this behavior using an absolute path for the `KCONFIG_ROOT` build system variable.

Similar to the file `zephyr/subsys/debug/Kconfig` of Zephyr's _debug_ subsystem, the `Kconfig` file specifies all available configuration options and their dependencies. We'll only specify a simple boolean symbol in the `Kconfig` file in our application's root directory. Please refer to the [official documentation](https://docs.zephyrproject.org/latest/build/kconfig/setting.html#setting-kconfig-configuration-values) for details about the `Kconfig` syntax:

```bash
$ tree --charset=utf-8 --dirsfirst -L 1
.
├── boards
├── src
├── CMakeLists.txt
├── Kconfig
├── extra0.conf
├── extra1.conf
├── prj.conf
└── prj_release.conf

$ cat Kconfig
```

```conf
mainmenu "Customized Menu Name"
source "Kconfig.zephyr"

menu "Application Options"
config USR_FUN
	bool "Enable usr_fun"
	default n
	help
	  Enables the usr_fun function.
endmenu
```

The skeleton of this `Kconfig` file is well explained in [step 4 in the "Application CMakeLists.txt" section in the official documentation](https://docs.zephyrproject.org/latest/develop/application/index.html#application-cmakelists-txt):

The statement _"mainmenu"_ defines a custom text that is used as our `Kconfig` main menu. It is shown, e.g., as a title in the build target `menuconfig`. We'll see this in a moment when we'll build our target and launch `menuconfig`.

The _"source"_ statement essentially includes the top-level Zephyr Kconfig file `zephyr/Kconfig.zephyr` and all of its symbols (all _"source"_ statements are relative to Zephyr's root directory). This is necessary since we're effectively replacing the `zephyr/Kconfig` file of the Zephyr base that is usually _parsed_ as the first file by _Kconfig_. We'll see this below when we look at the build output. The contents of the default root `Kconfig` file are quite similar to what we're doing right now:

```bash
$ cat $ZEPHYR_BASE/Kconfig
# -- hidden comments --
mainmenu "Zephyr Kernel Configuration"
source "Kconfig.zephyr"
```

By sourcing the `Kconfig.zephr` file, we're loading all _Kconfig_ menus and symbols provided with Zephr. Next, we declare our own menu between the _"menu"_ and _"endmenu"_ statements to group our application symbols. Within this menu, we declare our `USR_FUN` symbol, which we'll use to enable a function `usr_fun`.

Let's rebuild our application without configuring `USR_FUN` and have a look at the build output. A `--pristine` build is required to pick up the new `Kconfig` file:

```bash
$ west build --board nrf52840dk_nrf52840 -d ../build --pristine
```

```
Parsing /path/to/01_kconfig/Kconfig
Loaded configuration '/opt/nordic/ncs/v2.4.0/zephyr/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840_defconfig'
Merged configuration '/path/to/01_kconfig/prj.conf'
Merged configuration '/path/to/01_kconfig/boards/nrf52840dk_nrf52840.conf'
Configuration saved to '/path/to/build/zephyr/.config'
Kconfig
```

Have a good look at the very first line of the _Kconfig_-related output:
* In our previous builds, this line indicated the use of Zephyr's default `Kconfig` file as follows:
  `Parsing /opt/nordic/ncs/v2.4.0/zephyr/Kconfig`,
* Whereas now it uses our newly created `Kconfig` file:
  `Parsing /path/to/01_kconfig/Kconfig`.

The remainder of the output should look familiar. When we look at the generated `zephyr/.config` file in our build directory, we see that the `USR_FUN` symbol is indeed set according to its default value `n`:

```bash
$ cat ../build/zephyr/.config
# Application Options
#
# CONFIG_USR_FUN is not set
# end of Application Options
```

The new, custom main menu name _"Customized Menu Name"_ is only visible in the interactive _Kconfig_ tools, such as `menuconfig`, and is not used within the generated `zephyr/.config` file. The interactive tools now also show our newly created menu and symbol.

As you can see in the below screenshot, the main menu has the name _"Customized Menu Name"_ and a new menu _"Application Options"_ is now available as the last entry of options:

```bash
$ west build -d ../build -t menuconfig
```

![]({% img_url practical-zephyr/kconfig-custom.png %})
![]({% img_url practical-zephyr/kconfig-application.png %})

> **Notice:** It is also possible to source `Kconfig.zephyr` _after_ defining the application symbols and menus. This would have the effect that your options will be listed _before_ the Zephyr symbols and menus. In this section, we've sourced `Kconfig.zephr` before our own options.

### Configuring the application build using _Kconfig_

Now that we have our own `Kconfig` symbol, we can use it in the application's build process. Create two new files `usr_fun.c` and `usr_fun.h` in the `src` directory:

```bash
$ tree --charset=utf-8 --dirsfirst
.
├── boards
│   ├── nrf52840dk_nrf52840.conf
│   └── nrf52840dk_nrf52840_release.conf
├── src
│   ├── main.c
│   ├── usr_fun.c
│   └── usr_fun.h
├── CMakeLists.txt
├── Kconfig
├── extra0.conf
├── extra1.conf
├── prj.conf
└── prj_release.conf
```

We're defining a simple function `usr_fun` that prints an additional message on boot using the newly created files:

```c
// Content of usr_fun.h
#pragma once

void usr_fun(void);
```

```c
// Content of usr_fun.c
#include "usr_fun.h"
#include <zephyr/kernel.h>

void usr_fun(void)
{
    printk("Message in a user function.\n");
}
```

Now, all that's left is to tell _CMake_ about our new files. Instead of always including these files when building our application, we can use our _Kconfig_ symbol `USR_FUN` to only include the files in the build if the symbol is enabled. Add the adding the following to `CMakeLists.txt`:

```cmake
target_sources_ifdef(
    CONFIG_USR_FUN
    app
    PRIVATE
    src/usr_fun.c
)
```

`target_sources_ifdef` is another _CMake_ extension from `zephyr/cmake/modules/extensions.cmake`: It conditionally includes our files in the build in case `CONFIG_USR_FUN` is defined. Since the symbol `USR_FUN` is _disabled_ by default, our build currently does not include `usr_fun`.

### Using the application's `Kconfig` symbol

Let's first extend our `main.c` file to use `usr_fun` regardless of any configuration options:

```c
#include <zephyr/kernel.h>
#include "usr_fun.h"

#define SLEEP_TIME_MS 100U

void main(void)
{
    printk("Message in a bottle.\n");
    usr_fun();

    while (1)
    {
        k_msleep(SLEEP_TIME_MS);
    }
}
```

Unsurprisingly, building this application fails since `usr_fun.c` is not included in the build:

```bash
$ west build --board nrf52840dk_nrf52840 -d ../build --pristine
```

```
[158/168] Linking C executable zephyr/zephyr_pre0.elf
FAILED: zephyr/zephyr_pre0.elf zephyr/zephyr_pre0.map
--snip--
/path/to/ld.bfd: app/libapp.a(main.c.obj): in function `main':
/path/to/main.c:9: undefined reference to `usr_fun'
collect2: error: ld returned 1 exit status
ninja: build stopped: subcommand failed.
FATAL ERROR: command exited with status 1: /path/to/cmake --build /path/to/build
```

We have two possibilities to fix our build: The first possibility is to enable `USR_FUN` in our kernel configuration, e.g., in our `prj.conf` file:

```conf
# --snip--
CONFIG_USR_FUN=y
```

This, however, defeats the purpose of having a configurable build: The build would still fail in case we're not enabling `USR_FUN`. Instead, we can also use the `USR_FUN` symbol within our code to only conditionally include parts of our application:

```c
#include <zephyr/kernel.h>

#ifdef CONFIG_USR_FUN
#include "usr_fun.h"
#endif

#define SLEEP_TIME_MS 100U

void main(void)
{
    printk("Message in a bottle.\n");

#ifdef CONFIG_USR_FUN
    usr_fun();
#endif

    while (1)
    {
        k_msleep(SLEEP_TIME_MS);
    }
}
```

Now, our build succeeds for either configuration of the `USR_FUN` symbol. Clearly, we could also use the same approach as `printk` and provide an empty function or definition for `usr_fun` instead of adding conditional statements to our application.

> **Note:** The application builds and links, but you won't see any output on the serial console in case you've followed along; The _Kconfig_ symbols responsible for the serial output are still disabled within `prj.conf` and the board-specific fragment. Feel free to update your configuration, e.g., by enabling the debug output for normal builds while disabling it for the "release" build!

To understand why we can use the `CONFIG_USR_FUN` definition, we can have a look at the compiler commands used to compile `main.c`. Conveniently (as mentioned in the previous article), by default, Zephyr enables the _CMake_ variable [`CMAKE_EXPORT_COMPILE_COMMANDS`](https://cmake.org/cmake/help/latest/variable/CMAKE_EXPORT_COMPILE_COMMANDS.html). The compiler command for `main.c` is thus captured by the `compile_commands.json` in our build directory:

```json
{
  "directory": "/path/to/build",
  "command": "/path/to/bin/arm-zephyr-eabi-gcc --SNIP-- -o CMakeFiles/app.dir/src/main.c.obj -c /path/to/main.c",
  "file": "/path/to/main.c"
},
```

Within the large list of parameters passed to the compiler, there is also the `-imacros` option specifying the `autoconf.h` Kconfig header file:

```
-imacros /path/to/build/zephyr/include/generated/autoconf.h
```

This header file contains the configured value of the `USR_FUN` symbol as a macro:

```c
// --snip---
#define CONFIG_USR_FUN 1
```

Looking at the [official documentation of the `-imacros` option for `gcc`](https://gcc.gnu.org/onlinedocs/gcc/Preprocessor-Options.html), you'll find that this option acquires all the macros of the specified header without also processing its declarations. Thus, all macros within the `autoconf.h` files are also available at compile time.

## Conclusion

In this article, we've explored the _kernel configuration system_ _Kconfig_ in great detail. Even if you've never used _Kconfig_ before, you should now be able to at least use _Kconfig_ with Zephyr, and - in case you're stuck - you should be able to easily navigate the official documentation. We've seen:

- how to find and change existing _Kconfig_ symbols,
- how to analyze dependencies between different symbols,
- files that are automatically picked up by the build system,
- files generated by _Kconfig_,
- how to define your own build types and board-specific fragments,
- how to define application-specific _Kconfig_ symbols, and
- how the build system and the application use _Kconfig_ symbols.

> **Note:** A full example application including all the configuration files that we've created throughout this article is available in the [`01_kconfig` folder of the accompanying GitHub repository](https://github.com/lmapii/practical-zephyr/tree/main/01_kconfig).

If you've followed along, you may also have noticed that `Kconfig` files are not something you'd like to explore file by file. While writing this article, I wanted to include an entire include graph showing this, but in the end, it is just too many files. I'd recommend sticking to the available tools!

In your future Zephyr projects, _Kconfig_ will become especially important once you're starting to build your own device drivers. With this article, I hope I can give you an easy start.

## Further reading

The following are great resources when it comes to Zephyr and are worth a read _or watch_:

- Zephyr's own [official documentation on Kconfig](https://docs.zephyrproject.org/latest/build/kconfig/index.html#configuration-system-kconfig) is of course the first go-to reference.
- Zephyr's official docs also contain a great collection of [Kconfig tips and best practices](https://docs.zephyrproject.org/latest/build/kconfig/tips.html#kconfig-tips-and-best-practices), including  [what not to turn into Kconfig options](https://docs.zephyrproject.org/latest/build/kconfig/tips.html#what-not-to-turn-into-kconfig-options) in case you're thinking about creating your own symbols.
- If you want to see _Kconfig_ in action in a device driver, I can highly recommend watching the [Tutorial: Mastering Zephyr Driver Development](https://www.youtube.com/watch?v=o-f2qCd2AXo) by Gerard Marull Paretas from the Zephyr Development Summit 2022.
- The Linux Kernel documentation contains a great section about the [Kconfig Language](https://www.kernel.org/doc/html/latest/kbuild/kconfig-language.html).
- In case you haven't done it yet, the [nRF Connect SDK Fundamentals](https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/) course in Nordic's [DevAcademy](https://academy.nordicsemi.com/) also covers _Kconfig_.
- [Golioth](https://golioth.io/) also has a large number of [blog posts about Zephyr](https://blog.golioth.io/category/zephyr/), make sure to check them out!

Just like in the previous article, I can always warmly recommend browsing through the videos from the _Zephyr Development Summit_, e.g., the playlists from the [2022](https://www.youtube.com/watch?v=o-f2qCd2AXo&list=PLzRQULb6-ipFDwFONbHu-Qb305hJR7ICe) and [2023](https://www.youtube.com/watch?v=PY64voxdhAU&list=PLzRQULb6-ipERkFrHaBh8tuSnK923ZUjY) Developers Summits.

Finally, have a look at the files in the [accompanying GitHub repository](https://github.com/lmapii/practical-zephyr) and I hope you'll follow along with the future articles of this series!

<!-- References

[practical-zephyr]: https://github.com/lmapii/practical-zephyr

[zephyr-ds-2022-playlist]: https://www.youtube.com/watch?v=o-f2qCd2AXo&list=PLzRQULb6-ipFDwFONbHu-Qb305hJR7ICe
[zephyr-ds-2023-playlist]: https://www.youtube.com/watch?v=PY64voxdhAU&list=PLzRQULb6-ipERkFrHaBh8tuSnK923ZUjY
[zephyr-ds-2022-driver-dev]: https://www.youtube.com/watch?v=o-f2qCd2AXo

[nordicsemi]: https://www.nordicsemi.com/
[nordicsemi-dev-academy]: https://academy.nordicsemi.com/
[nordicsemi-academy-kconfig]: https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/lessons/lesson-3-elements-of-an-nrf-connect-sdk-application/topic/configuration/
[nordicsemi-nrf52840-dk]: https://www.nordicsemi.com/Products/Development-hardware/nrf52840-dk

[nrf-connect-sdk-fundamentals]: https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/
[nrf-vscode-kconfig]: https://marketplace.visualstudio.com/items?itemName=nordic-semiconductor.nrf-kconfig
[nrf-connect-sdk]: https://www.nordicsemi.com/Products/Development-software/nrf-connect-sdk
[nrf-connect-vscode]: https://nrfconnect.github.io/vscode-nrf-connect/

[zephyr-build]: https://docs.zephyrproject.org/latest/build/index.html
[zephyr-cmake]: https://docs.zephyrproject.org/latest/build/cmake/index.html
[zephyr-printk]: https://docs.zephyrproject.org/latest/services/logging/index.html#printk
[zephyr-logging]: https://docs.zephyrproject.org/latest/services/logging/index.html
[zephyr-kconfig]: https://docs.zephyrproject.org/latest/build/kconfig/index.html#configuration-system-kconfig
[zephyr-kconfig-merge]: https://docs.zephyrproject.org/latest/build/kconfig/setting.html#the-initial-configuration
[zephyr-kconfig-extra]: https://docs.zephyrproject.org/latest/develop/application/index.html#important-build-system-variables
[zephyr-kconfig-dont]: https://docs.zephyrproject.org/latest/build/kconfig/tips.html#what-not-to-turn-into-kconfig-options
[zephyr-kconfig-search]: https://docs.zephyrproject.org/latest/kconfig.html
[zephyr-kconfig-tips]: https://docs.zephyrproject.org/latest/build/kconfig/tips.html#kconfig-tips-and-best-practices
[zephyr-kconfig-syntax]: https://docs.zephyrproject.org/latest/build/kconfig/setting.html#setting-kconfig-configuration-values
[zephyr-kconfig-cmake]: https://docs.zephyrproject.org/latest/develop/application/index.html#application-cmakelists-txt
[zephyr-hardening]: https://docs.zephyrproject.org/latest/security/hardening-tool.html
[zephyr-dts]: https://docs.zephyrproject.org/latest/build/dts/index.html
[zephyr-west-dashdash]: https://docs.zephyrproject.org/latest/develop/west/build-flash-debug.html#one-time-cmake-arguments
[zephyr-rn-3.4.0]: https://docs.zephyrproject.org/latest/releases/release-notes-3.4.html
[zephyr-dconf]: https://docs.zephyrproject.org/latest/develop/application/index.html#basics

[golioth]: https://golioth.io/
[golioth-kconfig-diff]: https://blog.golioth.io/zephyr-quick-tip-show-what-menuconfig-changed-and-make-changes-persistent/
[golioth-blog-zephyr]: https://blog.golioth.io/category/zephyr/

[serial-dt]: https://www.decisivetactics.com/products/serial/
[serial-putty]: https://www.putty.org/

[kernel-config]: https://www.kernel.org/doc/html/latest/kbuild/kconfig-language.html
[cmake-build-type]: https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html
[cmake-compile-commands]: https://cmake.org/cmake/help/latest/variable/CMAKE_EXPORT_COMPILE_COMMANDS.html
[gcc-imacros]: https://gcc.gnu.org/onlinedocs/gcc/Preprocessor-Options.html

-->
