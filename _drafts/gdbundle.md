---
title: "gdbundle - GDB's Missing Plugin Manager"
description: "Introducing gdbundle, a plugin manager for GDB and LLDB, which enables developers to easily install plugins through PyPi and pip."
author: tyler
image:
---

I started with embedded development at Pebble, the smart watch maker, where we
were building a modern operating system on a device with 128 KB of RAM. It's
also where I got up-close and personal with GDB and had my first encounter with
GDB's Python API.

One one of our bi-weekly hack-days, one of the firmware engineers wrote a script
to pretty-print a summary of the heap for a device connected to GDB using its
Python API ([similar to
this]({% post_url 2019-07-02-automate-debugging-with-gdb-python-api %}#adding-a-custom-gdb-command-with-gdb-python)).
It was **awesome**. But, using this script with GDB wasn't without problems. We
slowly grew it into a collection of scripts and improved our workflows. A few
notable improvements were:

- Add more scripts for each of our critical modules in the firmware, such as
  state machines, Bluetooth state, etc.
- Instead of just printing the heap allocations, expose the heap through a
  Python Class. e.g.
  `heap = Heap(g_heap); total_size = sum([b.size] for b in heap.iter_blocks()])`.
- [Wrapping the GDB
  invocation]({% post_url 2019-08-27-building-a-cli-for-firmware-projects %})
  and loading GDB Python scripts automatically so every developer on the team
  had them loaded and ready to use at all times.

This was all back in 2015 or so. As I've moved around to other companies and
talked to many other embedded software developers, I've discovered that many of
them have never thought to extend or automate GDB using scripts, and even more
had not even heard of GDB's Python API. I'm saddened by this.

I have a personal goal (and it's one of Memfault's missions) to empower embedded
software developers with the tools they need to get their job done, and I'm
excited to share with you something I've been thinking about and building
towards for over 3 years.

<!-- excerpt start -->

GDB desperately needs a better way for developers to share scripts,
user-interface improvements, and utilities with their peers. This would
enable building upon each other's work and debugging techniques, progressing the
entire community and preventing developers from reinventing the wheel. GDB (and LLDB) needs a plugin manager, and I'd like to introduce to
you [gdbundle](https://github.com/memfault/gdbundle).

<!-- excerpt end -->

> This article speaks primarily to GDB, but gdbundle works perfectly
> with LLDB as well.

_Like Interrupt? [Subscribe](http://eepurl.com/gpRedv) to get our latest posts
straight to your mailbox_

{:.no_toc}

## Table of Contents

<!-- prettier-ignore -->
* auto-gen TOC:
{:toc}

## Why Does GDB Need a Plugin Manager

Anyone who has used a modern language will tell you: package managers are fantastic! The GDB ecosystem stands to gain much from adopting one.

### No Out-Of-Box Debugging of Popular Software

Within the past year, I've worked with four different microcontroller stacks, seven
different Real-Time Operating Systems (RTOS's), and a handful of common
low-level software libraries including Mbed TLS, the WICED Bluetooth Stack, and
many vendor SDK's. Tens of thousands of developers use each one of
these stacks/libraries, and every single one of them has to manually debug each
module by hand by using GDB's print functionality or write their own scripts.

This is why embedded developers often choose to use proprietary debuggers over, despite their cost and clunkiness. They have these debugging utilities built in or
allow extensions to be integrated and sold[^code_confidence], even though the
software backing them isn't all that complex.

### Poor Installation and Non-Discoverable of Extensions

In the age of modern developer tools, easy installation of extensions is a
critical facet that needs to be taken seriously. Let's take the new wave of text
editors as an example. The ones that have gained massive popularity over the
last 5 years, Sublime Text, VSCode, and Atom, all have a built-in package
manager which allows searching for and installing extensions with only a few
keystrokes. **This is what GDB should have**.

As for discoverability, I love websites like
[VimAwesome](https://vimawesome.com/) and the
[VSCode Marketplace](https://marketplace.visualstudio.com/vscode). I check them
regularly to see which pieces of my workflows have been automated for me.

If you believe this ease of extensibility is only for high-level applications
and desktop style software, check out
[PlatformIO's Libraries page](https://platformio.org/lib/search?query=) which
provides easy installation and discoverability of compiled C/C++ packages for
embedded systems and Arduino.

### Plugin development requires rebuilding the world

Since there is no trivial way for GDB scripts to build upon each other, all
ambitious projects, such as [hugsy/gef](https://github.com/hugsy/gef),
[pwndbg/pwndbg](https://github.com/pwndbg/pwndbg), and
[snare/voltron](https://github.com/snare/voltron), are required to rewrite or
copy-paste functionality that would normally be imported from popular Python
packages (e.g. termcolor). This is a barrier to entry for plugin developers.

On the other side of the pendulum, since packages can't be imported easily
through the Python ecosystem, users are required to run
[invasive](https://github.com/pwndbg/pwndbg/blob/dev/setup.sh)
[shell](https://github.com/snare/voltron/blob/master/install.sh) scripts that
use the system's package manager and install Python packages into the system's
Python installation.

Every developer should be using virtual environments and leaving their system
Python installation alone, and the Python environment within GDB is no
different.

### Lack of Customization

On one hand, some might see this as a nice feature of GDB. Once you get a GDB
shell, you know exactly how to accomplish what you want, assuming you know how
to use GDB to begin with. This is similar to how people think about Vi(m).

However, Vim is infinitely better today due to its customization options, plugin
managers, community, and ability to quickly change the initial historic settings
to modern and sane defaults.

In contrast, GDB and it's customization story has roughly remained the same for
the last 10 years for the majority of developers. I found it ironic that the
most popular GDB enhancement at my previous employer was
[vim-scripts/Conque-GDB](https://github.com/vim-scripts/Conque-GDB), a Vim
plugin which wraps GDB.

## Life with a GDB Plugin Manager

I'd like to now paint a picture that is possible if we take the time to clean
up, share, and install GDB scripts that we all have stored somewhere in our
local `~/.gdbinit` or stored away in some old repo.

1. I start a new project which uses the Zephyr RTOS as an operating system.
2. I install the Zephyr GDB plugin so I can view information about the threads,
   heaps, block pools, mutexes, and global state variables all in GDB.
3. I pull in the LittleFS[^littlefs] library so that I can persist logs to the filesystem.
4. I install the LittleFS GDB plugin which enables me to read and write files to the filesystem since I have a memory-mapped flash chip.
5. I find a bug in the LittleFS GBD plugin due to a global variable name change. I
   fork the plugin, push a fix, and the maintainer publishes a patch version update of
   the plugin, which every developer can immediately update to.

The only item gating this future is a standard and agreed upon way to write,
package, publish, and install these plugins. 

## GDB's Current Extensibility Hooks

Before covering what gdbundle is and how it works, I'd briefly like to talk about the extensibility that is already included with GDB. 

There are essentially three "built-in" ways to automatically load GDB scripts
upon launch.

If you'd like to follow along, you may download the Interrupt repo and navigate
to the example directory.

```
$ git clone git@github.com:memfault/interrupt.git
$ cd interrupt/example/gdbundle/
```

### Using `--command` Argument

GDB allows loading and executing scripts using the `--command <file>` and
`--eval-command <command>` arguments. This is the one I recommend everyone to
use and personally use within all of my projects.

For example, we can load a local file called `hello.gdb` which contains a GDB
command called `hello`.

```
# hello.gdb

define hello
echo "Hello World from .gdb\n"
end
```

To load this when we launch GDB, we can use `--command /path/to/hello.gdb`

```
$ gdb --command hello.gdb

(gdb) help user
User-defined commands.
The commands in this class are those defined by the user.
Use the "define" command to define a command.

List of commands:

hello -- User-defined

(gdb) hello
Hello World from .gdb
```

You can also load GDB Python scripts using this technique as well. Let's create
a `hello_py` command in a file `hello.py`.

```python
# hello.py

import gdb

class HelloPy(gdb.Command):
    def __init__(self):
        super(HelloPy, self).__init__('hello_py', gdb.COMMAND_USER)

    def invoke(self, _unicode_args, _from_tty):
        print("Hello World from .py")

HelloPy()
```

```
$ gdb --command hello.py
(gdb) hello_py
Hello World from .py
```

If you don't want to keep typing `--command ...` every time you launch GDB, or
trying to convince or remind your co-workers to do the same, I suggest [wrapping
the GBD invocation with a
CLI]({% post_url 2019-08-27-building-a-cli-for-firmware-projects %}).

### objfile-gdb.ext File

The next way that GDB allows you to auto-load extensions is by matching the
object name to a script. Let's run through an example of this as well. Let's
create a c file called `test.c`.

```c
// test.c

#include <stdio.h>

int main() {
  printf("%s\n", "Hello World");
}
```

Next, we'll compile it into a binary named `myexe.out`.

```
$ gcc -g test.c -o myexe.out
```

Now that we have an executable called `myexe.out`, we can create a script file
named `<object_file_name>-gdb.gdb`, or in our case `myexe.out-gdb.gdb`, and GDB
will load it for us automatically.

> NOTE: GDB will auto load scripts from the directories listed in the command
> `show auto-load scripts-directory` or from the current working directory.

For this example, we'll copy our previous file.

```
$ cp hello.gdb myexe.out-gdb.gdb
```

We can now launch GDB without any extra arguments other than the executable and
verify that the command is loaded.

```
$ gdb myexe.out
(gdb) hello
Hello World from .gdb
```

This also works with GDB Python scripts, however the filename must end with
`-gdb.py`.

### .debug_gdb_scripts Section

Upon loading an ELF file, GDB will also look for a section called
`.debug_gdb_scripts`. This section can contain either filenames (e.g.
`gdb_scripts.py`) or full scripts, like the `HelloPy` script we wrote above.

While I admit that this functionality is clever, it means that the information
is hard-coded in the ELF file and can't be easily changed at a later date.
Adding a filename or fixing a bug in an embedded script would require [creative
uses of GNU binutils]([Conda]({% post_url 2020-04-08-gnu-binutils %})) and some
work re-distributing the ELF files.

## `gdbundle` - GDB's Plugin Manager

gdbundle is short for GDB bundle and is a plugin manager for GDB.

To summarize, gdbundle plugins:

- can be automatically loaded by `gdbundle`.
- are Python 2 and 3 compatible.
- can be distributed through PyPi within Python packages.
- can be installed using `pip`, `poetry`, etc.
- can be installed into virtual environments to prevent version collisions.
- can wrap pre-existing GDB plugins/scripts, such as
  [cyrus-and/gdb-dashboard](https://github.com/cyrus-and/gdb-dashboard), so that
  modifying the original package is not necessary.

gdbundle tries to remain simple, un-opinionated, and delegates most of the work
of loading the plugin to the plugin itself. Let's go over how it works! It won't
take long.

By default, gdbundle:

1. Searches for installed Python packages that begin with the name `gdbundle_`.
2. Calls the `gdbundle_load()` function of each plugin module.
3. In each plugin's `gdbundle_load()` function, the plugin should source, import, and
   configure the GDB scripts that it has bundled inside (or from another package
   by way of a dependency).

And that is all.

> gdbundle should work with LLDB out of the box, as all that
> is required is Python and a way to run the setup procedure at launch. Please check
> out the [gdbundle README.md](https://github.com/memfault/gdbundle) for more information
> and examples. 

### Requirements

gdbundle has a couple requirements.

#### Python-compatible GDB

gdbundle requires a GDB compiled with Python. You can check if your GDB is
configured correctly by the following quick test:

```
$ gdb
(gdb) python-interactive
>>> import sys; sys.version
'3.6.7 | packaged by conda-forge'
```

If a Python version is printed, everything should work! If not, there are a few things to
try:

- Brew and Ubuntu's Apt both install a GDB with Python enabled.
- Your distribution might also include a `*-gdb-py` version with Python enabled.
- Clone [binutils-gdb](https://github.com/bminor/binutils-gdb) and try compiling
  it with the configuration flag `--with-python`.

#### Modification to `.gdbinit`

Just like Vim plugin managers require the user to edit their `~/.vimrc`,
gdbundle requires a snippet of code to be added to a loaded `.gdbinit` file.

```
# -- GDBUNDLE_EDITS_BEGIN
python

import os,subprocess,sys
# Execute a Python using the user's shell and pull out the sys.path (for site-packages)
paths = subprocess.check_output('python -c "import os,sys;print(os.linesep.join(sys.path).strip())"',shell=True).decode("utf-8").split()
# Extend GDB's Python's search path
sys.path.extend(paths)

import gdbundle
gdbundle.init()

end
# -- GDBUNDLE_EDITS_END
```

For more information about exactly what is happening above, feel free to read the short section in
[Using PyPi Packages with GDB](https://interrupt.memfault.com/blog/using-pypi-packages-with-GDB#setting-syspath-within-gdbinit). 

The first block **is required** to be able to use Python packages
installed in the activated virtual environment, Conda environment, or non-default Python
installation.

The second part, `gdbundle.init()`, is what is new and will load installed
gdbundle plugins.

> NOTE: The major (and ideally minor) version of Python linked with GDB should match
> the version that is activated at the time of launch, whether that is a virtual environment, Conda environment, `brew` Python installation, etc. 
> Mismatched major versions will cause packages and plugins to either break in mysterious ways or not show up at all. 
> More information can be found in the [gdbundle README.md](https://github.com/memfault/gdbundle)

## gdbundle Example Plugin

No plugin manager introduction would be complete without an example plugin. I've created a "Hello World" plugin for gdbundle called
[`gdbundle-example`](https://github.com/memfault/gdbundle-example). Let's dig in
and see how it's built.

### Structure

Below is the directory structure of our example plugin that works with GDB and LLDB.

```
├── README.md
├── gdbundle_example
│   ├── __init__.py
│   ├── gdb_loader.py
│   ├── lldb_loader.py
│   └── scripts
│       ├── example_gdb.gdb
│       ├── example_gdb.py
│       └── example_lldb.py
└── pyproject.toml
```

Simple, right? I've opted to use `pyproject.toml` instead of `setup.py` because
I wanted to embrace the future of standardized packaging, but note that these
packages will work with Python 2 and 3, so it shouldn't matter.

### How It Works

Let's look at the code in `gdbundle_example/gdb_loader.py`

```python
import gdb
import os

PACKAGE_DIR = os.path.dirname(__file__)

SCRIPT_PATHS = [
    [PACKAGE_DIR, 'scripts', 'example_gdb.gdb'],
    [PACKAGE_DIR, 'scripts', 'example_gdb.py']
]

def _abs_path(path):
    return os.path.abspath(os.path.join(*path))

def gdbundle_load():
    for script_path in SCRIPT_PATHS:
        gdb.execute("source {}".format(_abs_path(script_path)))
```

Notice the `gdbundle_load` function which `gdbundle` will call
directly. When this is called, it takes each script file located in `scripts/`
and tells GDB to `source` it. Notice that both GDB Python and GDB Command
scripts work!

By using `gdb.execute("source <script>")`, plugins can source GDB scripts that
were never designed with gdbundle in mind or designed to be distributed.

## A Few More Examples

Given the flexibility of gdbundle itself, it can allow developer to package up
any of the already existing GDB scripts and make installation easier for
everyone.

Here are a few more examples of plugins I've created that wrap popular packages
that I use frequently.

- [gdbundle-gdb-dashboard](https://github.com/memfault/gdbundle-gdb-dashboard) -
  gdbundle plugin for
  [cyrus-and/gdb-dashboard](https://github.com/cyrus-and/gdb-dashboard).
- [gdbundle-PyCortexMDebug](https://github.com/memfault/gdbundle-PyCortexMDebug) -
  gdbundle plugin for
  [bnahill/PyCortexMDebug](https://github.com/bnahill/PyCortexMDebug).
- [gdbundle-voltron](https://github.com/memfault/gdbundle-voltron) - gdbbundle plugin for [snare/voltron](https://github.com/snare/voltron). This example is my personal favorite because the install script is particularly sketchy but you can simply run `pip install gdbundle-voltron` and you're up and running!

## Summary of Benefits of using gdbundle

There are a handful of indisputable benefits of using gdbundle. 

1. Just `pip install gdbundle-<plugin-name>`. No more manually editing your `~/.gdbinit` in specific ways depending on the extension.
2. It **enables** developers to use virtual environments (and encourages it!) without the need for each plugin to [mangle `sys.path`](https://github.com/snare/voltron/blob/master/voltron/entry.py#L26-L34) in creative ways and install native packages using `apt` or `brew`. 
3. Personal projects and team projects can have project-specific `requirements.txt` and `.gdbinit` files. With these two in place, a new developer would just need to `pip install -r requirements.txt` and they'll have everything they need to start using the plugins.
4. Discoverability. Want to find out what new gdbundle packages exist? Just go to [PyPi and search](https://pypi.org/search/?q=gdbundle).
5. Dependency management and version tracking is now done automatically by Python's packaging infrastructure. No more telling users to download a new version of the script or writing your own `update.sh` script.

## The Future

So, where does gdbundle go from here? Honestly, it's up to everyone reading this. I've been using this internally for a few weeks, and I have been battle testing the `sys.path` hi-jacking approach for 3 years (along with \~100 developers at my previous company). I am confident in the approach taken, but just like I said about GDB, a plugin manager is nothing without the community and willing developers to hack something together.

_All the code used in this blog post is available on
[Github](https://github.com/memfault/interrupt/tree/master/example/faster-compilation/).
See anything you'd like to change? Submit a pull request!_

{:.no_toc}

## Neat GDB Script Repositories

I came across some neat repositories of GDB scripts while I was searching the
Internet. I've included them below for anyone looking to learn more about how
people use GDB Python.

> Sponsored by [grep.app](https://grep.app). Not really, but that app is seriously incredible. 
> [Try it out for yourself!](https://grep.app/search?q=%28import%20gdb%7Cgdb.COMMAND_USER%29&regexp=true&filter[lang][0]=Python)

### Debugging Utilities

<!-- prettier-ignore-start -->
- [Linux - scripts/gdb/](https://github.com/torvalds/linux/tree/master/scripts/gdb)
- [CPython - Tools/gdb/libpython.py](https://github.com/python/cpython/blob/master/Tools/gdb/libpython.py)
- [golang/Go - src/runtime/runtime-gdb.py](https://github.com/golang/go/blob/master/src/runtime/runtime-gdb.py)
- [MongoDB - buildscripts/gdb/mongo.py](https://github.com/mongodb/mongo/blob/master/buildscripts/gdb/mongo.py)
- [PX4/Firmware - platforms/nuttx/Debug/Nuttx.py](https://github.com/PX4/Firmware/blob/master/platforms/nuttx/Debug/Nuttx.py)
<!-- prettier-ignore-end -->

### Usability / UI Enhancements

<!-- prettier-ignore-start -->
- [cyrus/gdb-dashboard](https://github.com/cyrus-and/gdb-dashboard)
- [vim-scripts/Conque-GDB](https://github.com/vim-scripts/Conque-GDB)
- [longld/peda](https://github.com/longld/peda)
- [snare/voltron](https://github.com/snare/voltron)
- [hugsy/gef](https://github.com/hugsy/gef)
- [pwndbg/pwndbg](https://github.com/pwndbg/pwndbg)
- [cloudburst/libheap](https://github.com/cloudburst/libheap)
- [PlasmaHH/vdb](https://github.com/PlasmaHH/vdb)
<!-- prettier-ignore-end -->

## References

<!-- prettier-ignore-start -->
[^extending_gdb]: [GDB Documentation - Extending GDB](https://sourceware.org/gdb/onlinedocs/gdb/Extending-GDB.html#Extending-GDB)
[^gdb_startup]: [What GDB Does During Startup](http://sourceware.org/gdb/onlinedocs/gdb/Startup.html)
[^code_confidence]: [Code Confidence - FreeRTOS Eclipse Plugin](https://www.codeconfidence.com/freertos-tools.shtml)
[^littlefs]: [ARMmbed/littlefs](https://github.com/ARMmbed/littlefs)
[^gnu_arm_embedded_toolchain]: [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
<!-- prettier-ignore-end -->
