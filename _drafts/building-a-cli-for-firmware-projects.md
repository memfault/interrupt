---
title: Building a CLI for Firmware Projects using Invoke
description:
  "A walkthrough on how to build a command line interface (CLI) for a firmware
  project using the Python Invoke package."
author: tyler
tags: [python]
---

<!-- excerpt start -->

Building a small (or large) command line interface (CLI) for a project is a
great way to get an entire team to build, test, debug, and work with a project
in the same way using the same set of tools. This post goes into detail about
how to think about a project's CLI and implementing one using the
[Invoke](http://www.pyinvoke.org/) Python package.

<!-- excerpt end -->

## Why Build A Project CLI?

By building a CLI for a project, it becomes a single way for any developer,
script, or continuous integration system to perform the most common operations
are performed everyday.

For example, it helps convert launching JLinkGDBServer with the appropriate
settings from a Wiki copy-paste of:

```
$ JLinkGDBServer -device Cortex-M4 -if SWD -speed 4000 -RTOSPlugin_FreeRTOS
```

to

```
invoke jlink
```

and flashing using GDB from:

```
$ cd products/blinky/gcc
$ arm-none-eabi-gdb-py -x gdbinit.jlink ../build/product.elf -x ../../common_gdb_scripts.gdbinit
```

to

```
invoke flash
```

A web service's REST API needs to be stable, easy to use, and self documenting.
Your project's CLI should meet the same requirements.

## When to Build Project CLI

If you or your teammates have experienced any of the following more than once,
it may be time to write a CLI for your project.

- You **copy/paste** commands from a Wiki page or an old Slack message
- You found the command you were looking for but it's now **out-of-date**
- It has become common for teammates to ask in Slack **"What is that
  command..."**
- Scripts have **undocumented** arguments and environment variable overrides
- Two or more teammates have **written similar scripts** for themselves but a
  common shared version doesn't exist, or is broken.
- You check out a 3 month-old revision of your project and there is no longer
  documentation about how to run (factory builds anyone?).

I could go on and on, but I hope the idea is clear. Building a CLI using Invoke
helps with all of the above issues and more.

## Getting started with Invoke

Invoke is a "task execution tool & library" that works with Python 2.7 and 3.4+,
and be installed using `pip`. We'll be using it today to build our project's CLI
tool.

We'll be using a virtual environment, and I highly recommend them.
[This guide](https://docs.python-guide.org/dev/virtualenvs/#lower-level-virtualenv)
is a good starting point. I will be using Python 3.6, which has the `virtualenv`
command available already.

```
$ virtualenv venv
$ source venv/bin/activate
(venv)
$ pip install invoke
```

We can quickly test if it was properly installed by running `invoke` in our
shell.

```
(venv)
$ invoke --list
Can't find any collection named 'tasks'!
```

We've confirmed it's installed. To start adding tasks to Invoke, one needs to
add a `tasks.py` file to the root of a project. No matter where the user is
within the project, Invoke will search upwards until it finds a `tasks.py` file.

## Using Invoke with the nRF5 SDK

> For purposes of this example, we are going to use the Nordic nRF5 SDK version
> 15.2.0, which can be downloaded
> [here](https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/), and
> within that, the Blinky app located at
> `nrf5_sdk/examples/peripheral/blinky/pca10056/blank/armgcc/`.

The following is the most basic, but still reasonable, `tasks.py` to allow us to
easily build our Blinky project:

```python
# /nrf5_sdk/tasks.py

import os

from invoke import Collection, task

ROOT_DIR = os.path.dirname(__file__)
BLINKY_DIR = os.path.join("nrf5_sdk", "examples", "peripheral",
                          "blinky", "pca10056", "blank", "armgcc")

@task
def build(ctx):
    """Build the project"""
    with ctx.cd(PCA10056_BLINKY_ARMGCC_DIR):
        ctx.run("make build")
```

This sets up the proper paths, `cd`'s into the directory, and runs `make`.

```
$ inv --echo build
cd /Users/tyler/dev/memfault/interrupt/example/invoke-basic/nrf5_sdk/examples/peripheral/blinky/pca10056/blank/armgcc && make -j4
mkdir _build
cd _build && mkdir nrf52840_xxaa
Assembling file: gcc_startup_nrf52840.S
Compiling file: main.c
Compiling file: nrf_log_frontend.c
Compiling file: nrf_log_str_formatter.c
```

Some of you might immediately think this wrapper or abstraction is unnecessary,
and maybe it is in the simplest case, but let's dive a bit into the future of
this build command.

### Adding Parallel Builds

A few weeks pass on the project and our build time slips from a few seconds to
30 seconds. We now want everyone working on the project to use Make's `-j`
option to enable parallel builds. To do so using our Invoke command, we'd only
have to add `-j4` to our build command.

```python
@task
def build(ctx):
    """Build the project"""
    with ctx.cd(PCA10056_BLINKY_ARMGCC_DIR):
        ctx.run("make build -j4")
```

Now everone running `invoke build` will automatically use parallel make.

### Adding `ccache`

A few more weeks, and our build time slowed down again. This time, we've thought
to enable `ccache`, and we want every developer to use this. It appears that the
nRF5 SDK somewhat enables this feature, but it depends on the existence of a
`/usr/bin/ccache` binary. On my macOS machine, `ccache` is installed to the path
`/usr/local/bin/ccache`, so we'd rather have this variable be automatically set!

We can change the SDK from `:=` to `?=` to allow the `CCACHE` variable to be
overridden by our environment, and now we can define this variable ourselves
from our Invoke task!

```
# ⁨nRF5_SDK_15.2.0/components⁩/toolchain⁩/gcc⁩/Makefile.common

CCACHE ?= $(if $(filter Windows%,$(OS)),, \
               $(if $(wildcard /usr/bin/ccache),ccache))
```

```python
from shutil import which

@task
def build(ctx, ccache=True):
    """Build the project"""
    env = {}
    if ccache:
      env = {'CCACHE': which(ccache)}

    with ctx.cd(PCA10056_BLINKY_ARMGCC_DIR):
        ctx.run("make build -j4", env=env)
```

A few things are happening above now.

1. We've imported the `which` Python function to help us find where a binary in
   our `$PATH` is located.
2. We create an `env` variable and pass that to our Invoke task. Invoke will
   take the variables defined and `export` them into the environment while
   running that task.
3. We've added a `ccache` argument, which our CI system can use to disable
   `ccache` in CI since we want to guarantee a fresh build. Our CI system would
   use this as `invoke build --no-ccache`

What's neat about everything here is that even though developers keep running
`invoke build`, new functionality gets added without them knowing.

### Checking for `ccache`

A few more weeks later, we realize some developers are not using `ccache`
because it isn't installed on their machines. One could go around from computer
to computer checking that each developer installs it and keeps it installed, but
an Invoke 'pre' task is a better option.

```python
@task
def check_ccache(ctx):
    ccache = which(ccache)
    if not ccache:
        msg = (
            "Couldn't find `ccache`.\n"
            "Please install it using the following command:\n\n"
            ">  `brew install ccache` OR `apt install ccache`"
        )
        raise Exit(msg)


@task(pre=[check_ccache])
def build(ctx, ccache=True):
    """Build the project"""
    ...
```

Using these `check_` style routines along with `@task(pre=[...])` is an easy way
to ensure that a developer's local environment is set up and remains set up
correctly even as project requirements and versions change. I would argue it's
very much required when a project upgrades `gcc` versions!

## Don't use Make as your CLI

I've seen Make used as a CLI many times. I cringe most times.

Make is great at dependency checking and simple recipes, but it shouldn't be
used as a CLI. Let's give an example.

In the nRF5 SDK, this is the recipe for `flash`.

```
# Flash the program
flash: default
    @echo Flashing: $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex
    nrfjprog -f nrf52 --program $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex --sectorerase
    nrfjprog -f nrf52 --reset
```

This uses the `nrfjprog` to flash and reset the device with the hard coded
`.hex` file. This is great if that's **all** we wanted to do, but in the
future...

- What if we want to flash a different `.hex` file?
- What if we have two devices plugged in and want to select a certain one?
- What if we allowed the developer to choose between flashing with GDB or
  `nrfjprog`?

As you can see, this would require a few global variables that can be overridden
using `make flash ELF=<path/to/hex> METHOD=gdb`, and a developer would never
know about these unless a Wiki page is written and kept up-to-date or the source
code is read frequently.

Let's go through some other reasons why I advice against using Make as a CLI.

### Writing Make recipes is hard

Make recipes are essentially shell commands. If you have a CLI command that does
argument parsing, logic, environment checking, or spawns external processes and
threads, then it's going to be incredibly difficult to do in a make recipe and
shell command. It's also incredibly difficult to write these in a cross-platform
manner.

Invoke has
[argument parsing](http://docs.pyinvoke.org/concepts/invoking-tasks.html#task-command-line-arguments)
logic built-in, a
[configuration management system](http://docs.pyinvoke.org/en/1.3/concepts/configuration.html#configuration-files),
and for all the complex logic, it's just Python!

### Make is hard to debug

Debugging a CLI is mostly about debugging the variables that get passed down
into the shell commands or into the later Python scripts that get called.

To debug an Invoke task, there are two common methods: Printing out the `ctx`
object or setting a breakpoint.

```python
@task
def env(ctx):
    """Print out the `ctx` variable"""
    # Print out the Context, which contains most information you need
    print(ctx)

    # Or set a breakpoint to get a debugger
    import pdb
    pdb.set_trace()
```

To debug a Make task, there is really only one easy way, and that is using
`$(info ...)` statements

```
env:
    $(info $(ARG_ELF_FILE))
    # ...
```

### Make commands aren't documented

When working in a team environment, documentation is **critical**. When using
Make, the Makefile and other source code is all the documentation that is
provided.

With Invoke, documentation is built in. We can print all commands available as
well as drill down into each to find out more information. This makes it easy
for developers to discover new commands and find what they are looking for.

```
$ inv --list
Available tasks:

  build       Build the project
  console     Start an RTT console session
  debug       Spawn GDB and attach to the device without flashing
  flash       Spawn GDB and flash application & softdevice
  gdbserver   Start JLinkGDBServer
```

```
$ inv flash --help
Usage: inv[oke] [--core-opts] flash [--options] [other tasks here ...]

Docstring:
  Spawn GDB and flash application & softdevice

Options:
  -e STRING, --elf=STRING   The elf to flash
  -p INT, --port=INT        The GDB port to attach to
```

### Make requires at least one expert

Make is a great build system, and it has an impressive number of features, but
trying to do too much with it will result in, to most developers other than Make
experts or to the original author, an unintelligible mess of `include <>.mk`
calls and variable overrides with `?=`.

## A full Invoke example for Blinky

Here, I provide a more production ready example for the Blinky app to give a
better idea of how Invoke should be used for a project. The contents here can be
placed in a `tasks.py` file in the project root and Invoke will automatically
pick up the tasks!

> This source code can be found on
> [Github](https://github.com/memfault/interrupt/tree/master/example/invoke-basic/)

```python
import os, shutil, sys
from invoke import Collection, Config, Exit, task
from shutil import which

# Constants
ROOT_DIR = os.path.dirname(__file__)
NRF_SDK_DIR = os.path.join(ROOT_DIR, "nrf5_sdk")
BLINKY_DIR = os.path.join(NRF_SDK_DIR, "examples", "peripheral", "blinky",
                          "pca10056", "blank", "armgcc")
BLINKY_ELF = os.path.join(BLINKY_DIR, "_build", "nrf52840_xxaa.out")

GDB_EXE = "arm-none-eabi-gdb-py"
GCC_EXE = "arm-none-eabi-gcc"

JLINK_GDB_PORT = 2331
JLINK_TELNET_PORT = 19021


def _check_exe(exe, instructions_url):
    exe_path = which(exe)
    if not exe_path:
        msg = (
            "Couldn't find `{}`.\n"
            "This tool can be found here:\n\n"
            ">  {}".format(exe, instructions_url)
        )
        raise Exit(msg)

@task
def check_toolchain(ctx):
    """Run as a `pre` task to check for the presence of the ARM toolchain"""
    url = "https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads"
    _check_exe(GCC_EXE, url)
    _check_exe(GDB_EXE, url)


@task
def check_segger_tools(ctx):
    """Run as a `pre` task to check for the presence of the Segger tools"""
    url = "https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack"
    _check_exe("JLinkGDBServer", url)


@task(pre=[check_toolchain])
def build(ctx, ccache=True):
    """Build the project"""
    with ctx.cd(BLINKY_DIR):
        # To get nrf52 SDK to pick up GCC in our $PATH
        env = {"GNU_INSTALL_ROOT": ""}
        if ccache:
            env.update({"CCACHE": which("ccache")})
        ctx.run("make -j4", env=env)


@task(pre=[check_toolchain], help={
    "elf": "The elf to flash",
    "port": "The GDB port to attach to"
})
def flash(ctx, elf=BLINKY_ELF, port=JLINK_GDB_PORT):
    """Spawn GDB and flash application & softdevice"""
    cmd = f'{GDB_EXE} --eval-command="target remote localhost:{JLINK_GDB_PORT}"' \
          f' --ex="mon reset" --ex="load" --ex="mon reset" --se={BLINKY_ELF}'
    ctx.run(cmd)


@task(pre=[check_toolchain], help={
        "elf": "The ELF file to pull symbols from",
        "port": "The GDB port to attach to"
})
def debug(ctx, elf=BLINKY_ELF, port=JLINK_GDB_PORT):
    """Spawn GDB and attach to the device without flashing"""
    cmd = f'{GDB_EXE} --eval-command="target remote localhost:{JLINK_GDB_PORT}"' \
          f' --se={BLINKY_ELF}'
    ctx.run(cmd)


@task(pre=[check_segger_tools], help={
    "gdb": "The GDB port to publish to",
    "telnet": "The Telnet port to publish to"
})
def gdbserver(ctx, gdb=JLINK_GDB_PORT, telnet=JLINK_TELNET_PORT):
    """Start JLinkGDBServer"""
    ctx.run(f"JLinkGDBServer -if swd -device nRF52840_xxAA -speed auto "
            f"-port {gdb} -RTTTelnetPort {telnet}")


@task(pre=[check_segger_tools])
def console(ctx, telnet=JLINK_TELNET_PORT):
    """Start an RTT console session"""
    ctx.run(f"JLinkRTTClient -LocalEcho Off -RTTTelnetPort {telnet}")


# Add all tasks to the namespace
ns = Collection(build, console, debug, flash, gdbserver)
# Configure every task to act as a shell command (will print colors, allow interactive CLI)
# Add our extra configuration file for the project
config = Config(defaults={"run": {"pty": True}})
ns.configure(config)
```

Given the full (but incredibly minimal) example above, we have the following
tasks and features:

- We can run: - `invoke build` to build the project with `ccache` and
  parallelized - `invoke console` will connect to the device's serial console -
  `invoke gdbserver` will spawn `JLinkGDBServer` with the correct
  configuration - `invoke flash` will flash the binary through GDB and give us a
  prompt to the device - `invoke debug` will attach to a running device using
  GDB without flashing
- For each command, we will check that the proper binaries/packages are
  installed using `pre` tasks
- We can run `inv --list` and `inv <command> --help` for help menus.

## Final Thoughts

I've thoroughly enjoyed using Invoke in my previous and current job. In
Memfault's codebase, we have around 100 tasks, most of which are namespaced
under general top-level modules. This provides our team with a centralized place
for all of the common tasks, such as building the firmware SDK, publishing
documentation, running automated tests on our firmware devices, pushing new
versions of our service, performing database migrations...everything. The
self-documenting nature of the commands is indispensable.

I encourage the exploration of the
[Invoke documentation](http://docs.pyinvoke.org/) to learn more about all the
features.

For a production example of Invoke tasks, look no further than the
[Invoke source code](https://github.com/pyinvoke/invoke/blob/master/tasks.py).
It provides a good example of how to import tasks from other modules to create a
centralized tasks list.

## Further Reading

- I like to use a combination of Invoke and
  [Click](https://click.palletsprojects.com/en/7.x/utils/#printing-to-stdout)
  for better pretty prining and colors.
- Invoke includes
  [tab-completion support](http://docs.pyinvoke.org/en/1.3/invoke.html#shell-tab-completion).

_All the code used in this blog post is available on
[Github](https://github.com/memfault/interrupt/tree/master/example/invoke-basic/).
See anything you'd like to change? Submit a pull request!_
