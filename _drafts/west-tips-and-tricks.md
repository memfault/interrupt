---
title: "Zephyr West tips and tricks"
description:
  A handful of useful west tricks for managing Zephyr workspaces more
  effectively.
author: noah
tags: [zephyr, west]
---

<!-- excerpt start -->

A few handy `west` tips I've found useful that aren't always covered in the
getting-started guides.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Managing SDKs with `west sdk`

West has a built-in command for managing Zephyr SDKs:

```bash
# install a specific SDK version and target toolchain
west sdk install --version 0.17.4 -t arm-zephyr-eabi
```

You can install just the toolchain(s) you actually need, rather than pulling
down the full SDK bundle. Some other useful targets:

- `riscv64-zephyr-elf`
- `xtensa-espressif_esp32_zephyr-elf`

Run `west sdk list` to see what's available, and `west sdk install --help` for
the full set of options.

What's awesome is if you run this from a west workspace containing a `zephyr`
project, `west sdk install` will match the `zephyr/SDK_VERSION` value, so you'll
get a matching SDK version, ezpz!

```plaintext
  --version [SDK_VER]   version of the Zephyr SDK to install. If not specified,
                        the install version is detected from
                        ${ZEPHYR_BASE}/SDK_VERSION file.
```

This is covered in more depth in [Practical Zephyr - West workspaces (Part
6)]({% post_url 2024-05-16-practical_zephyr_west %}), definitely worth a read if
you haven't already.

## Limiting clone depth with `clone-depth`

If you're working with large repositories or frequently initializing new
workspaces, you can add `clone-depth` to projects in your west manifest to speed
things up considerably:

```yaml
# west.yml
manifest:
  projects:
    - name: some-large-repo
      url: https://github.com/example/some-large-repo
      clone-depth: 1
```

Full manifest reference:
<https://docs.zephyrproject.org/latest/develop/west/manifest.html#projects>

> **Note:** I'm not sure whether `clone-depth` is respected for projects that
> are pulled in transitively via `import`. Worth verifying for your particular
> workspace setup.

## Limiting what `west update` imports

By default, if you import a project's manifest (e.g., Zephyr's `west.yml`), west
will clone _all_ of its listed dependencies recursively. Zephyr has a lot of
them, and a naive `west update` can easily pull down gigabytes worth of repos
you don't need.

Two manifest options that help a lot here:

**`name-allowlist`** — Only import the named projects from the upstream
manifest:

```yaml
# west.yml
manifest:
  projects:
    - name: zephyr
      url: https://github.com/zephyrproject-rtos/zephyr
      revision: v4.3.0
      import:
        name-allowlist:
          - cmsis
          - hal_nordic
          - mbedtls
```

This is
[Option 3 in the manifest docs](https://docs.zephyrproject.org/latest/develop/west/manifest.html#option-3-mapping)
and is probably the most useful thing you can do to keep workspace
initialization fast.

**`path-prefix`** — Put all imported projects under a common subdirectory
instead of the workspace root. Keeps things tidy:

```yaml
import:
  path-prefix: deps
  name-allowlist:
    - cmsis
    - hal_nordic
```

With the above, imported repos land under `deps/` rather than scattered at the
top level. The [Practical Zephyr - West
workspaces]({% post_url 2024-05-16-practical_zephyr_west %}) article walks
through a full working example of both of these together.

## Caching west update fetches

If you clone west workspaces frequently (e.g., when testing unfamiliar
projects), git fetch traffic can add up quickly. West supports a reference cache
to share object storage across workspaces:

```bash
west config --global update.auto-cache ~/.cache/west
```

With `auto-cache`, west will automatically populate and use a cache directory,
so subsequent `west update` calls in new workspaces can resolve most objects
locally instead of hitting the network.

There are a few other useful `update.*` config options worth looking at in the
reference:
<https://docs.zephyrproject.org/latest/develop/west/config.html#built-in-configuration-options>

## Aliases

West supports user-defined command aliases, similar to `git` aliases. These are
great for shortening workflows you run constantly. From the docs:

```bash
west config --global alias.run "build --pristine=never --target run"
west config --global alias.menuconfig "build --pristine=never --target menuconfig"
```

After that, `west run` and `west menuconfig` just work. The alias docs have more
examples:
<https://docs.zephyrproject.org/latest/develop/west/alias.html#examples>

## West extensions

West extensions let projects (or your own local tooling) add custom subcommands
to west. This is useful when you want project-specific tooling to feel like a
first-class citizen in your workflow rather than a separate script.

One practical example: the `memfault-cli` package provides a west extension for
uploading symbol files, so you can do something like:

```bash
west memfault upload-mcu-symbols
```

...without any extra path wrangling.

The extension mechanism is straightforward to implement if you want to wrap your
own tooling. Reference:
<https://docs.zephyrproject.org/latest/develop/west/extensions.html#west-extensions>

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^west-sdk]: [Zephyr SDK - West SDK command](https://docs.zephyrproject.org/latest/develop/west/zephyr-cmds.html)
[^west-manifest]: [West manifest - projects](https://docs.zephyrproject.org/latest/develop/west/manifest.html#projects)
[^west-config]: [West built-in configuration options](https://docs.zephyrproject.org/latest/develop/west/config.html#built-in-configuration-options)
[^west-alias]: [West aliases](https://docs.zephyrproject.org/latest/develop/west/alias.html#examples)
[^west-extensions]: [West extensions](https://docs.zephyrproject.org/latest/develop/west/extensions.html#west-extensions)
<!-- prettier-ignore-end -->
