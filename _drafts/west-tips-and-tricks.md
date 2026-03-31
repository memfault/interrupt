---
title: "Zephyr West tips and tricks"
description:
  Some useful west tricks for managing Zephyr workspaces more effectively.
author: noah
tags: [zephyr, west]
---

<!-- excerpt start -->

The Zephyr `west` tool is a powerful workspace manager, but it is admittedly
pretty feature-rich: there's a lot of functionality buried inside that tool!

I thought it would be interesting to share a few tips and tricks that I use with
`west` to make my Zephyr development workflow a little smoother.

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
first-class citizen in your workflow rather than a separate script (it's
especially nice for discoverability, since everything is available from
`west --help`).

It's useful when the commands you need don't fit into the normal
build/flash/debug/attach workflow provided by the Zephyr SDK's own extension
commands, and aliases are insufficient.

A quick example: we add the following files/snippets to our manifest repo:

**`west.yml`:**

```yaml
manifest:
  self:
    # register this repo's west extensions
    west-commands: scripts/west-commands.yml
```

**`scripts/west-commands.yml`:**

```yaml
west-commands:
  - file: scripts/west_extensions.py
    commands:
      - name: project-hello
        class: ProjectHello
        help: sample Project west extension command
```

**`scripts/west_extensions.py`:**

```python
from textwrap import dedent

from west.commands import WestCommand


class ProjectHello(WestCommand):
    def __init__(self):
        super().__init__(
            "project-hello",  # gets stored as self.name
            "",  # ignored self.help, will not be required by future west versions
            description=dedent(
                """
            Sample Project west extension command.
            """
            ),
        )

    def do_add_parser(self, parser_adder):
        parser = parser_adder.add_parser(self.name, description=self.description)

        levels = ["dbg", "inf", "wrn", "err"]
        parser.add_argument(
            "-l",
            "--level",
            default="inf",
            choices=levels,
            help="log level for the message (default: %(default)s)",
        )
        parser.add_argument("message", help="message to print")

        return parser  # gets stored as self.parser

    def do_run(self, args, unknown):
        log_fn = {
            "dbg": self.dbg,
            "inf": self.inf,
            "wrn": self.wrn,
            "err": self.err,
        }[args.level]

        self.inf(f"[{self.name}] log level: {args.level}")
        log_fn(f"[{self.name}] message: {args.message}")
```

Now the new extension shows up in `west --help`:

```plaintext
extension commands from project manifest (path: blahblah):
  project-hello:        sample Project west extension command
```

And we can run it:

```bash
❯ west project-hello --level err 'Hello!'
[project-hello] log level: err
ERROR: [project-hello] message: Hello!
```

The Zephyr reference goes through the details of how to do this:
<https://docs.zephyrproject.org/latest/develop/west/extensions.html#west-extensions>

## Reducing what `west update` fetches

For day-to-day development you often don't need the full history of every
dependency. You can speed up `west update` significantly with a couple of flags:

```bash
west update --narrow --fetch-opt=--depth=1
```

- `--narrow` tells west to only fetch the specific revision referenced in the
  manifest, rather than fetching all refs from the remote.
- `--fetch-opt=--depth=1` passes `--depth=1` to the underlying `git fetch`,
  giving you a shallow clone with just the tip commit.

Together these can dramatically cut down network traffic and disk usage,
especially on first initialization of a large workspace.

Use `git fetch --unshallow` on a module to convert a shallow clone to a full one
later- this is useful if you need the full history (eg bisecting) or want to
make a contribution upstream!

## Resolving the manifest to exact SHAs

West manifests typically reference branches or tags, which can drift over time.
To capture a fully-resolved snapshot with exact commit SHAs — useful for
reproducible builds or auditing — use:

```bash
west manifest --resolve
```

The output is a valid west manifest with every project pinned to a specific
revision:

```yaml
manifest:
  group-filter:
    - -babblesim
    - -optional
    - -testing
  projects:
    - name: canopennode
      url: https://github.com/zephyrproject-rtos/canopennode
      revision: dec12fa3f0d790cafa8414a4c2930ea71ab72ffd
      path: modules/lib/canopennode
      groups:
        - optional
    - name: chre
      url: https://github.com/zephyrproject-rtos/chre
      revision: c4c2f49fdcaa2fed49eb1db027696a5734a010d2
      path: modules/lib/chre
      groups:
        - optional
  # ...
```

You can redirect this into a `west.lock.yml` (or similar) and commit it
alongside your manifest so that anyone cloning the workspace gets exactly the
same tree.

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
[^west-update]: [West update command](https://docs.zephyrproject.org/latest/develop/west/basics.html#west-init-and-west-update)
[^west-manifest-resolve]: [West manifest --resolve](https://docs.zephyrproject.org/latest/develop/west/manifest.html#resolving-manifests)
<!-- prettier-ignore-end -->
