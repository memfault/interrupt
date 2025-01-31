---
title: Publishing the Memfault SDK as an ESP-IDF Component
description: How we shipped our SDK as an ESP-IDF component
author: noah
tags: [ci, memfault, esp32, esp-idf, github-actions, interrupt-live]
image: /img/memfault-esp-component/esp-registry-page.png
---

<!-- excerpt start -->

In this very Memfault-centric post, I'll be talking about how we shipped our SDK
as an ESP-IDF component. This is a continuation of our efforts to make it easier
to integrate Memfault into your projects, specifically targeting ESP32-based
projects.

<!-- excerpt end -->

> 🎬
> _[Listen to Noah on Interrupt Live](https://www.youtube.com/watch?v=UPFhUrQa1bs)
> talk about the content and motivations behind writing this article._

{% include newsletter.html %}

{% include toc.html %}

## What are we talking about here

Ok, so full disclosure- your author is employed by Memfault, and this post is
going to be talking a LOT about our
[Memfault Firmware SDK](https://github.com/memfault/memfault-firmware-sdk)! In
spite of that, I hope it's an interesting walkthrough of this specific problem
we recently tackled.

### ESP Component Registry

A little over 2 years ago, in 2022, Espressif introduced the
[ESP Component Registry](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-component-manager.html),
to streamline sharing and reusing public components.

Initially, it was used for breaking up the ESP-IDF itself into smaller
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

<video autoplay loop src="/img/esp-component/zissou.webm" alt="Steve Zissou: let me tell you about my boat GIF"></video>

ESP-IDF's build system is based on CMake and has a LOT of features and options.

The manual is here if you're interested in going on a deep dive:

- [ESP-IDF CMake Build System](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html)

From our perspective, there are essentially 4 steps that happen when you build
an ESP-IDF-based project[^1] (names are my own):

![Picture of 4 steps of esp-idf build system](/img/esp-component/idf.py-build-system.excalidraw.svg)

1. Scan and load all selected components- each component's `CMakeLists.txt` file
   is processed in script mode
1. `Kconfig` files are loaded from ESP-IDF and all registered components.
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
and a `Kconfig` file that define the SDK's build and configuration settings.
Take a look here if you're curious:

- [CMakeLists.txt](https://github.com/memfault/memfault-firmware-sdk/blob/master/ports/esp_idf/memfault/CMakeLists.txt)
- [Kconfig](https://github.com/memfault/memfault-firmware-sdk/blob/master/ports/esp_idf/memfault/Kconfig)

### What we need to do to make this a component

ESP Component Registry-enabled components have the following properties:

- Root-level `idf_component.yml` manifest
- Root-level `CMakeLists.txt` and `Kconfig` files
- The enclosing folder name must _exactly_ match the component name

We have essentially 2 options when it comes to making our SDK an ESP-IDF
component:

1. **Reorganize our SDK to put the ESP port at the root and the core files under
   `src`, etc**:
   - This is the simplest option, but it requires changing the Memfault SDK's
     folder structure
1. **Use a `CMakeLists.txt` file in the SDK's root folder to include the SDK
   folder**:
   - This is more complex, but it allows us to keep the SDK's folder structure
     as-is

We'll go with option #2 because it's simpler to keep the ESP Component version
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

  Note: we leave off the `version:` field, which is inserted when the component
  is packed for upload (see
  [reference here](https://docs.espressif.com/projects/idf-component-manager/en/latest/reference/manifest_file.html#version))

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
  # If the repo is being used as an ESP-IDF component, bring in the ESP-IDF-specific
  # Kconfig file. Otherwise, this should be unused.
  if IDF_TARGET != ""
  rsource "ports/esp_idf/memfault/Kconfig"
  endif
  ```

There were a few minor tweaks needed to the existing `CMakeLists.txt` (at
`ports/esp_idf/memfault/CMakeLists.txt`) to make it work when included in this
way, but nothing notable- just a few path adjustments.

### Testing the change!

The ESP component tooling permits selecting a local path for a component, like
so:

```yaml
dependencies:
  # Define local dependency with relative path
  some_local_component:
    path: ../../projects/component
```

One nuance here- the component name specified must exactly match the enclosing
folder name or the build will fail.

Memfault uses a monorepo for our development, where the Memfault Firmware SDK is
located in a subfolder at `sdk/embedded`. To make it easy to test the component
change, we'll create a bind mount with the correct name
(`memfault-firmware-sdk`), so we can work out of our monorepo and test the
component build without needing to copy files around:

```bash
mkdir /tmp/memfault-firmware-sdk && \
sudo mount --bind /path/to/memfault-monorepo/sdk/embedded /tmp/memfault-firmware-sdk
```

Now, we can update our sample ESP32 app to use the Memfault SDK from the
filesystem as a component:

```yaml
dependencies:
  memfault-firmware-sdk:
    path: /tmp/memfault-firmware-sdk
```

And build the project:

```bash
idf.py build
```

The ESP-IDF build system will print out the loaded components by default, so we
can check the build output (it's pretty busy, truncated for brevity below):

```plaintext
-- Components: ... memfault-firmware-sdk ...
-- Component paths: ... /tmp/memfault-firmware-sdk ...
```

This makes iterating on the component changes much easier. We've now confirmed
our changes work as expected 🎉!

## Final steps

Let's use the ESP component tooling to package up our component for upload to
the ESP Component Registry. Modern ESP-IDF development environments come with
the `compote` Python tool pre-installed. Otherwise, it can be installed with
`pip install idf-component-manager` to a Python environment.

```bash
# Running from the root folder of our component
❯ compote component pack --name memfault-firmware-sdk --version 1.11.4-rc1
Saving archive to "dist/memfault-firmware-sdk_1.11.4-rc1.tgz"
```

That spits out an archive that can be uploaded to the ESP Component Registry. We
can also unpack the archive and confirm the contents look sane- it's a copy of
the files with the `idf_component.yml` manifest stamped with the version number.

We can test the component end-to-end now by uploading it to the component
registry:

```bash
❯ compote component upload \
  --archive dist/memfault-firmware-sdk_1.11.4-rc1.tgz \
  --namespace memfault \
  --name memfault-firmware-sdk
```

> Note: the example command above is uploading the component to the `memfault`
> namespace. To register a new namespace, you'll need to create an account on
> the ESP Component Registry website- the namespace will be set to the user
> account name. Espressif does support custom namespaces, but they need to be
> manually created by
> [contacting Espressif](https://docs.espressif.com/projects/idf-component-manager/en/latest/guides/packaging_components.html#authentication).

### Auto-publishing the component

We can automate the component publishing process by adding a step to our
existing Memfault SDK publishing automation. A simplified example is shown
below, using GitHub Actions:

```yaml
name: Publish ESP Component

on:
  push:
    tags:
      - "v*"

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4

      - name: Set up Python
        uses: actions/setup-python@v4
        with:
          python-version: "3.x"

      - name: Install compote
        run: pip install idf-component-manager

      - name: Build and publish component
        env:
          # This token is used to authenticate with the ESP Component Registry
          IDF_COMPONENT_API_TOKEN: ${{ secrets.IDF_COMPONENT_API_TOKEN }}
        run: |
          compote component pack --name memfault-firmware-sdk --version ${{ github.ref_name }}
          compote component upload --archive dist/memfault-firmware-sdk_${{ github.ref_name }}.tgz --namespace memfault --name memfault-firmware-sdk
```

### Continuous verification of the component

Finally, we also want to make sure the component doesn't break in the future
(maybe we change some paths or files in the SDK that break the component).

We take a couple of strategies to ensure this:

- Confirm the component builds every time we push a change to the internal
  development repo for the SDK
- A nightly check that the latest published component on the registry builds
  successfully
- A second nightly check that confirms the component builds successfully with
  the latest ESP-IDF master branch, in case there's upcoming tooling or build
  changes that we need to be aware of

## Summary

It's a pretty dry topic, but it is always interesting to learn about a new
software packaging system!

Some of the techniques apply to other build systems, especially the overall
methodology:

1. Identify and make the changes
1. Test (focus on an efficient test strategy!)
1. Automate the verification and distribution process

It was a fun project to work on, and I'm excited to see the Memfault SDK
available as an ESP-IDF component; we love making it simpler to integrate
Memfault into your projects, and this removes a few manual steps from the
process 🥳.

The final component is now available here:

<https://components.espressif.com/components/memfault/memfault-firmware-sdk>

One last thing- the ESP Component system permits including example apps. In our
case, it's the same example application we provide with the non-component
SDK[^2]. You can find the example here in the ESP Component Registry page:

<https://components.espressif.com/components/memfault/memfault-firmware-sdk/versions/1.15.0/examples/esp32/apps/memfault_demo_app>

And the ESP tooling makes it very easy to try out:

```bash
❯ idf.py create-project-from-example "memfault/memfault-firmware-sdk:esp32/apps/memfault_demo_app"
Executing action: create-project-from-example
Example "esp32/apps/memfault_demo_app" successfully downloaded to /home/noah/dev/memfault/interrupt/memfault_demo_app
Done

❯ cd memfault_demo_app

# Add a Memfault Project Key:
❯ echo "CONFIG_MEMFAULT_PROJECT_KEY=\"dummy\"\n" >> sdkconfig.defaults

# Build for the default ESP32 target, flash, and start the serial port monitor:
❯ idf.py build flash monitor
```

Thanks for reading!

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->

[^1]: This is a simplification, but it's close enough for our purposes. See [full explanation here](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/build-system.html#build-process).
[^2]: The example app is a simple ESP32 app that demonstrates the Memfault SDK's features, and can be [found here](https://github.com/memfault/memfault-firmware-sdk/tree/master/examples/esp32)

<!-- prettier-ignore-end -->
