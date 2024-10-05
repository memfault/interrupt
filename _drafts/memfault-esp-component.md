---
title: Publishing the Memfault SDK as an ESP-IDF Component
description: How we shipped our SDK as an ESP-IDF component
author: noah
tags: [ci, memfault, esp32, esp-idf, github-actions]
---

<!-- excerpt start -->

In this very Memfault-centric post, I'll be talking about how we shipped our SDK
as an ESP-IDF component. This is a continuation of our efforts to make it easier
to integrate Memfault into your projects, specifically targeting ESP32-based
projects.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## What are we talking about here

Ok, so full disclosure- your author is employed by Memfault, and this post is
going to be talking a LOT about our
[Memfault Firmware SDK](https://github.com/memfault/memfault-firmware-sdk)! In
spite of that, I hope it's an iteresting walkthrough of this specific problem we
recently tackled.

### ESP Component Registry

A little over 2 years ago, in 2022, Espressif introduced the
[ESP Component Registry](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-component-manager.html),
to streamline sharing and reusing public components.

Initially it was used mostly for breaking up the ESP-IDF itself into smaller
components, but it has since grown to include a
[variety of third-party components](https://components.espressif.com). This is a
great way to share code and make it easier to integrate into your projects.

### Memfault Firmware SDK

Memfault's Firmware SDK is a platform-agnostic SDK that provides a set of
observability/telemetry features for embedded devices:

- Fault capture and analysis
- Log collection
- Metrics collection
- Reboot tracking

It's designed to be easy to integrate into your project, and we're constantly
working to make it even easier.

We've supported the ESP32 series of chips for a while now
([over 4 years!](https://github.com/memfault/memfault-firmware-sdk/blob/00ff70fb8b92581e179c31e4dace6d36da25fc8b/CHANGELOG.md#changes-between-memfault-sdk-024-and-sdk-023---march-10-2020)
😮 time flies), but our integration has always required manually adding the SDK
to your project:

1. Clone the SDK into your project folder (typically as a submodule)
2. Add the SDK to your project's `CMakeLists.txt`
3. Set up necessary Kconfig settings to enable the SDK

Compare that with our Zephyr integration, where you can just add the SDK as a
`west` module to a project's `west.yml` file like so:

```yaml
manifest:
  projects:
    - name: memfault-firmware-sdk
      url: https://github.com/memfault/memfault-firmware-sdk
      revision: 1.11.5
```

Add the `CONFIG_MEMFAULT=y` Kconfig setting, and the Memfault SDK is enabled in
your project.

This is great! and it's the user experience we'd love for all our supported
platforms (looking at you, Eclipse-based IDEs... 😁).

## How we go from manual mode to component

### Let me tell you about our existing integration

[Insert steve zissou gif]

ESP-IDF's build system is based on CMake, and has a LOT of features and options.

The manual is here if you're interested in going on a deep dive:

- [ESP-IDF CMake Build System](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html)

From our perspective, there's essentially 4 steps that happen when you build an
ESP-IDF based project[^1] (names are my own):

![Picture of 4 steps of esp-idf build system](/img/esp-component/idf.py-build-system.excalidraw.svg)

1. Scan and load all selected components- each component's `CMakeLists.txt` file
   is processed in script mode
1. `Kconfig` files are loaded from ESP-IDF and all registerd components.
   `Kconfig` settings are loaded (from `sdkconfig` or `sdkconfig.defaults` if
   `sdkconfig` is not already generated), and the final configuration is
   generated
1. Process the component `CMakeLists.txt` files by including the component
   directories. This loads all the component source files, etc.
1. Build the project, producing the output artifacts (`.elf` files, OTA images,
   etc).

Memfault's existing integration enables our component by adding the directory to
the `EXTRA_COMPONENT_DIRS` CMake variable, which causes the SDK folder to be
processed, and we call `register_component()` in our component `CMakeLists.txt`
to add the SDK to the build.

Note that we're using `register_component()` here, instead of the modern
`idf_component_register()` function (see
[documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/build-system.html#minimal-component-cmakelists)),
because we need to support older versions of ESP-IDF. More about that later!

Otherwise, our SDK component is pretty standard- we have a `CMakeLists.txt` file
and `Kconfig` file that define the SDK's build and configuration settings. Take
a look here if you're curious:

- [CMakeLists.txt](https://github.com/memfault/memfault-firmware-sdk/blob/master/ports/esp_idf/memfault/CMakeLists.txt)
- [Kconfig](https://github.com/memfault/memfault-firmware-sdk/blob/master/ports/esp_idf/memfault/Kconfig)

### What we need to do to make this a component

ESP Component Registry-enabled components have the follow properties:

- Root-level `idf_component.yml` manifest
- Root-level `CMakeLists.txt` and `Kconfig` files
- The enclosing folder name must _exactly_ match the component name

We have essentially 2 options when it comes to making our SDK an ESP-IDF
component:

1. **Move the SDK into a new folder that matches the component name**:
   - This is the simplest option, but it requires changing the SDK's folder
     structure
1. **Use a `CMakeLists.txt` file in the SDK's root folder to include the SDK
   folder**:
   - This is more complex, but it allows us to keep the SDK's folder structure
     as-is

We'll go with option #2, because it's simpler to keep the ESP Component version
of our SDK as close as possible to the upstream version (mostly, less confusing
for us, the maintainers!).

### Making the change

The first step is to create the required root-level files:

- `idf_component.yml`: this one's pretty straightforward:

  ```yml
  description:
    Memfault SDK for embedded systems. Observability, logging, crash reporting,
    and OTA all in one service
  url: https://github.com/memfault/memfault-firmware-sdk
  repository: https://github.com/memfault/memfault-firmware-sdk
  documentation: https://docs.memfault.com/
  issues: https://github.com/memfault/memfault-firmware-sdk/issues
  ```

- `CMakeLists.txt`: this will shim over to our existing component
  `CMakeLists.txt`, but only if we're building as an ESP-IDF component:

  ```cmake
  cmake_minimum_required(VERSION 3.12.4)

  if(ESP_PLATFORM)
    include(${CMAKE_CURRENT_LIST_DIR}/ports/esp_idf/memfault/CMakeLists.txt)
  else()
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/Memfault.cmake)
  endif()
  ```

- `Kconfig`: this will shim over to our existing component `Kconfig` file
  similarly to the `CMakeLists.txt`:

  ```configuration
  # If the repo is being used as an ESP-IDF component, bring in the ESP-IDF specific
  # Kconfig file. Otherwise this should be unused.
  if IDF_TARGET != ""
  rsource "ports/esp_idf/memfault/Kconfig"
  endif
  ```

There were a few minor tweaks needed to the existing `CMakeLists.txt` (at
`ports/esp_idf/memfault/CMakeLists.txt`) to make it work when included in this
way, but nothing notable- just a few path adjustments.

### Testing the change!

## Final steps

### Auto-publishing the component

### Continous verification of the component

## OUTRO NAME TBD

Thanks for reading!

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->

[^1]: This is a simplification, but it's close enough for our purposes. See [full explanation here](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/build-system.html#build-process).

<!-- prettier-ignore-end -->
