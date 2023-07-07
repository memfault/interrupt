---
title: "gdbundle - GDB and LLDB's Missing Plugin Manager"
description: "Introducing gdbundle, a minimalist plugin manager for GDB and LLDB, which enables developers to easily GDB and LLDB extensions, packages, and Python scripts from PyPi"
author: tyler
image: /img/gdbundle-plugin-manager/gdbundle.png
tags: [gdb, python, debugging]
---

I started with embedded development at Pebble, the smart watch maker, where we
were building a modern operating system on a device with 128 KB of RAM. It's
also where I got up-close and personal with GDB and had my first encounter with
GDB's Python API[^gdb_python_api].

On one of our bi-weekly hack-days, one of the firmware engineers wrote a script
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

This was back in 2015 or so. As I've moved to other companies, I've had the joy
to spread the love of GDB scripting to other embedded software developers. It's
amazing how much a few Python scripts can improve one's productivity in the
debugger.

I have a personal goal (and it's one of Memfault's missions) to empower embedded
software developers with the tools they need to get their job done, and I'm
excited to share with you something I've been thinking about and building
towards for over 3 years.

<!-- excerpt start -->

GDB and LLDB desperately need a way for developers to share scripts,
user-interface improvements, and utilities with their peers. This would enable
building upon existing work and debugging techniques, progressing the entire
community and preventing developers from reinventing the wheel. GDB and LLDB
need a plugin manager, and I'd like to introduce to you
[gdbundle](https://github.com/memfault/gdbundle).

<!-- excerpt end -->

<script id="asciicast-UEAFpeLDRxoN72NnrZpfL30Gk" src="https://asciinema.org/a/UEAFpeLDRxoN72NnrZpfL30Gk.js" async></script>

Here is a link to the
[gdbundle Github page](https://github.com/memfault/gdbundle).

Whether you are an embedded engineer using GDB directly or through VSCode and
Cortex-Debug, an iOS or Android developer using LLDB within XCode or Android
Studio, or a C++ and Rust developer building x86 software, gdbundle will work
for you.

> This article speaks primarily to GDB, but
> [gdbundle](https://github.com/memfault/gdbundle) works perfectly with LLDB as
> well.

{% include newsletter.html %}

{% include toc.html %}

## Why Does GDB Need a Plugin Manager?

Anyone who has used a modern language or code editor will tell you: package
managers are fantastic! The GDB ecosystem stands to gain much from adopting one.

### Out-Of-Box Debugging of Popular Software

Within the past year, I've worked with four different microcontroller stacks,
seven different Real-Time Operating Systems (RTOS's), and a handful of common
low-level software libraries including Mbed TLS, the NRF5 SDK, the WICED Wi-Fi
Stack, and many vendor SDK's. Tens of thousands of developers use each one of
these stacks/libraries, and every single one of them has to manually debug each
module by hand by using GDB's print functionality or writing their own scripts.

This is why embedded developers often choose to use proprietary debuggers over
GDB, despite their cost and clunkiness. They have these debugging utilities
built-in or allow extensions to be integrated and sold[^code_confidence], even
though the software backing them isn't all that complex.

I believe that most popular software libraries could and should have a
complimentary debugging plugin for GDB that provides a holistic view of the
state of the system without the developer needing to print linked-lists by hand.

### More Customization

GDB is a professional tool used tens of thousands of developers. No two people
use GDB the same way and they shouldn't be required to. Given these facts, GDB
should be able to adapt to each developer's use case and empower them to do
their job. The use cases for GDB span from reverse engineering binaries,
debugging embedded software running on hardware connected over USB, to debugging
super computers half-way across the world.

With this level of customization required, GDB needs to be able to easily tailor
itself to many different use cases. There is no better way to enable this than
having a package manager.

The Vim community has taken these ideals to heart. Vim began as a tiny text
editor and has grown into a 800 lb. gorilla that's still light on its toes. It
didn't get there alone though. It has a multitude of plugin managers, thousands
of easily installable plugins, a growing contributor base (and Neovim), and it's
all configured by a simple `~/.vimrc` configuration file.

### Package Managers Increase Adoption and Build Community

At their core, package managers help developers build on top of previously
existing work and manage dependencies between packages so that integrations go
more smoothly.

Package managers also lower the barrier to entry for users and contributors to
the platform, whether it's a programming language, code editor, IDE, or
operating system. For instance, a brand new user of Vim can install the latest
version using `brew install vim`, set up the vim-plug[^vim_plug] plugin manager,
define a list of plugins to install, and within 5 minutes have all the syntax
packages, key mappings, and visual extensions they need to be productive.

One extra benefit of having a package manager is it increases discoverability. I
frequently find myself browsing the
[VSCode Marketplace](https://marketplace.visualstudio.com/vscode) and
[VimAwesome](https://vimawesome.com/) every month or two to see what's new.

GDB, unfortunately, doesn't currently have these benefits. The authors of GDB
scripts can't rely on what is currently installed on the system, and there is no
central repository of GDB scripts. Because of this, each user only installs what
they stumble upon on the Internet or what they hear about from their peers. This
in turn keeps the community and adoption small, and new developers turn to
alternative solutions instead.

## GDB's Current Extensibility Hooks

Before covering what gdbundle is and how it works, I'd briefly like to talk
about the extensibility that is already included with GDB[^extending_gdb].

There are essentially three "built-in" ways to automatically load GDB scripts
upon launch.

If you'd like to follow along, you may download the Interrupt repo and navigate
to the example directory.

```
$ git clone git@github.com:memfault/interrupt.git
$ cd interrupt/example/gdbundle-plugin-manager/
```

### Using `--command` Argument

GDB allows loading and executing scripts using the `--command <file>` and
`--eval-command <command>` arguments. This is the one I recommend everyone to
use, and I use it within all of my projects.

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
the GDB invocation with a
CLI]({% post_url 2019-08-27-building-a-cli-for-firmware-projects %}).

### objfile-gdb.ext File

The next way that GDB allows you to auto-load extensions is by matching the
object name to a script[^gdb_objfile_gdb_ext]. Let's run through an example of
this as well. Let's create a c file called `test.c`.

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

> NOTE: GDB will auto-load scripts from the directories listed in the command
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

This also works with GDB Python scripts, however, the filename must end with
`-gdb.py`.

### .debug_gdb_scripts Section

Upon loading an ELF file, GDB will also look for a section called
`.debug_gdb_scripts`[^gdb_debug_gdb_scripts]. This section can contain either
filenames (e.g. `gdb_scripts.py`) or full scripts, like the `HelloPy` script we
wrote above.

While I admit that this functionality is clever, it means that the information
is hard-coded in the ELF file and cannot be easily changed at a later date.
Adding a filename or fixing a bug in an embedded script would require [creative
uses of GNU binutils]({% post_url 2020-04-08-gnu-binutils %}) and some work
re-distributing the ELF files.

## `gdbundle` - Plugin Manager for GDB/LLDB

[gdbundle](https://github.com/memfault/gdbundle) is short for GDB bundle and is
a plugin manager for GDB and LLDB.

To summarize, gdbundle plugins:

- can be automatically loaded by gdbundle.
- can be compatible with both GDB and LLDB.
- can be both Python 2 and 3 compatible.
- can be distributed through PyPi within Python packages.
- can be installed using `pip`, `poetry`, etc.
- can be installed into virtual environments to prevent version collisions.
- can wrap pre-existing GDB plugins/scripts, such as
  [cyrus-and/gdb-dashboard](https://github.com/cyrus-and/gdb-dashboard), so that
  modifying the original package is not necessary.

[gdbundle](https://github.com/memfault/gdbundle) tries to remain simple,
un-opinionated, and delegates most of the work of loading the plugin to the
plugin itself. Let's go over how it works! It won't take long.

By default, gdbundle:

1. Searches for installed Python packages that begin with the name `gdbundle_`.
2. Calls the `gdbundle_load()` function of each plugin module.
3. In each plugin's `gdbundle_load()` function, the plugin should source,
   import, and configure the GDB scripts that it has bundled inside (or from
   another package by way of a dependency).

And that is all.

> gdbundle should work with LLDB out of the box, as all that is required is
> Python and a way to run the setup procedure at launch. Please check out the
> [gdbundle README.md](https://github.com/memfault/gdbundle) for more
> information and examples.

### Requirements

gdbundle has a couple of requirements.

#### Python-compatible GDB

gdbundle requires a GDB compiled with Python. You can check if your GDB is
configured correctly by the following quick test:

```
$ gdb
(gdb) python-interactive
>>> import sys; sys.version
'3.6.7 | packaged by conda-forge'
```

If a Python version is printed, everything should work! If not, there are a few
things to try:

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

For more information about exactly what is happening above, feel free to read
the short section in
[Using PyPi Packages with GDB](https://interrupt.memfault.com/blog/using-pypi-packages-with-GDB#setting-syspath-within-gdbinit).

The first block **is required** to be able to use Python packages installed in
the activated virtual environment, Conda environment, or non-default Python
installation.

The second part, `gdbundle.init()`, is what is new and will load installed
gdbundle plugins.

> NOTE: The major (and ideally minor) version of Python linked with GDB should
> match the version that is activated at the time of launch, whether that is a
> virtual environment, Conda environment, `brew` Python installation, etc.
> Mismatched major versions will cause packages and plugins to either break in
> mysterious ways or not show up at all. More information can be found in the
> [gdbundle README.md](https://github.com/memfault/gdbundle)

## Creating a gdbundle Plugin

No plugin manager introduction would be complete without an example plugin. I've
created a "Hello World" plugin for gdbundle called
[gdbundle-example](https://github.com/memfault/gdbundle-example). Let's dig in
and see how it's built.

### Structure

Below is the directory structure of our example plugin that works with GDB and
LLDB.

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

Notice the `gdbundle_load` function which `gdbundle` will call directly. When
this is called, it takes each script file located in `scripts/` and tells GDB to
`source` it. Notice that both GDB Python and GDB Command scripts work!

By using `gdb.execute("source <script>")`, plugins can source GDB scripts that
were never designed with gdbundle in mind or designed to be distributed.

## A Few Real gdbundle Plugins

Given the flexibility of gdbundle itself, it can allow the developer to package
up any of the already existing GDB scripts and make installation easier for
everyone.

Here are a few more examples of plugins I've created that wrap popular packages
that I use frequently.

- [gdbundle-gdb-dashboard](https://github.com/memfault/gdbundle-gdb-dashboard) -
  gdbundle plugin for
  [cyrus-and/gdb-dashboard](https://github.com/cyrus-and/gdb-dashboard).
- [gdbundle-PyCortexMDebug](https://github.com/memfault/gdbundle-PyCortexMDebug) -
  gdbundle plugin for
  [bnahill/PyCortexMDebug](https://github.com/bnahill/PyCortexMDebug).
- [gdbundle-voltron](https://github.com/memfault/gdbundle-voltron) - gdbbundle
  plugin for [snare/voltron](https://github.com/snare/voltron). This example is
  my personal favorite because the install script is particularly sketchy but
  you can simply run `pip install gdbundle-voltron` and you're up and running!

## Summary of Benefits of using gdbundle

There are a handful of indisputable benefits of using gdbundle.

1. Just `pip install gdbundle-<plugin-name>`. No more manually editing your
   `~/.gdbinit` in specific ways depending on the extension.
2. It **enables** developers to use virtual environments (and encourages it!)
   without the need for each plugin to
   [mangle `sys.path`](https://github.com/snare/voltron/blob/master/voltron/entry.py#L26-L34)
   in creative ways and install native packages using `apt` or `brew`.
3. Personal projects and team projects can have project-specific
   `requirements.txt` and `.gdbinit` files. With these two in place, a new
   developer would just need to `pip install -r requirements.txt` and they'll
   have everything they need to start using the plugins.
4. Discoverability. Want to find out what new gdbundle packages exist? Just go
   to [PyPi and search](https://pypi.org/search/?q=gdbundle).
5. Dependency management and version tracking is now done automatically by
   Python's packaging infrastructure. No more telling users to download a new
   version of the script or writing your own `update.sh` script.

## The Future

So, where does [gdbundle](https://github.com/memfault/gdbundle) go from here?
Honestly, it's up to everyone reading this. I've been using this internally for
a few weeks, and I have been battle testing the `sys.path` hi-jacking approach
for 3 years (along with \~100 developers at my previous company). I am confident
in the approach taken, but just like I said about GDB, a plugin manager is
nothing without the community and willing developers to hack something together.

[gdbundle](https://github.com/memfault/gdbundle) is in its infancy, and I'm
looking forward to any feature requests or issues that you can think of!

_All the code used in this blog post is available on
[Github](https://github.com/memfault/interrupt/tree/master/example/gdbundle-plugin-manager/)._
{% include submit-pr.html %}

{:.no_toc}

## Neat GDB Script Repositories

I came across some neat repositories of GDB scripts while I was searching the
Internet. I've included them below for anyone looking to learn more about how
people use GDB Python.

> Sponsored by [grep.app](https://grep.app). Not really, but that app is
> seriously incredible.
> [Try it out for yourself!](https://grep.app/search?q=%28import%20gdb%7Cgdb.COMMAND_USER%29&regexp=true&filter[lang][0]=Python)

### Debugging Utilities

<!-- prettier-ignore-start -->
- [Linux - scripts/gdb/](https://github.com/torvalds/linux/tree/master/scripts/gdb)<br>
  The master set of Linux GDB scripts, which allow developers to pretty print nearly everything
  about the state of the kernel.
- [CPython - Tools/gdb/libpython.py](https://github.com/python/cpython/blob/master/Tools/gdb/libpython.py)<br>
  Coming full circle, a common way to debug CPython is to use a set of GDB Python scripts to more easily print data structures.
- [Facebook/hhvm](https://github.com/facebook/hhvm/tree/master/hphp/tools/gdb)<br>
  An extensive collection of pretty-printers, utilities, I particularly like the `gdbutils.py` and
  `pretty.py` files as they have a random collection of goodies.
- [MongoDB - buildscripts/gdb/mongo.py](https://github.com/mongodb/mongo/blob/master/buildscripts/gdb/mongo.py)<br>
  Debug utilities for MongoDB to print current threads, mutexes, stacks, and other state.
- [PX4/Firmware - platforms/nuttx/Debug/Nuttx.py](https://github.com/PX4/Firmware/blob/master/platforms/nuttx/Debug/Nuttx.py)<br>
  A wonderful collection of GDB scripts for the NuttX RTOS to print threads, mutexes, heap information, and data structures. The names of the commands could use some work.
<!-- prettier-ignore-end -->

### Usability / UI Enhancements

<!-- prettier-ignore-start -->
- [cyrus/gdb-dashboard](https://github.com/cyrus-and/gdb-dashboard)<br>
  If anyone has a GDB UI enhancement set up, it's probably this one. It's popular with embedded engineers I know and provides a full overview of the system state.
- [vim-scripts/Conque-GDB](https://github.com/vim-scripts/Conque-GDB)<br>
  Also popular with embedded engineers I know, it allows a GDB session to be used within Vim cleanly. If you're a Vim user, definitely check it out.
- [PlasmaHH/vdb](https://github.com/PlasmaHH/vdb)<br>
  I stumbled upon this one and it looks great! It only has 2 Github stars, but has some great visualizations to show jumps for assembly.
- [snare/voltron](https://github.com/snare/voltron)<br>
  Similar to gdb-dashboard and surprisingly compatible with GDB, LLDB, VDB, and WinDbg.
<!-- prettier-ignore-end -->


### Reverse Engineering Enhancements

I can't speak to much of these since I am not the target market, but these all look powerful and helpful.

<!-- prettier-ignore-start -->
- [longld/peda](https://github.com/longld/peda)
- [hugsy/gef](https://github.com/hugsy/gef)
- [pwndbg/pwndbg](https://github.com/pwndbg/pwndbg)
<!-- prettier-ignore-end -->

## References

<!-- prettier-ignore-start -->
[^gdb_python_api]: [GDB Python API](https://sourceware.org/gdb/onlinedocs/gdb/Python-API.html)
[^extending_gdb]: [GDB Documentation - Extending GDB](https://sourceware.org/gdb/onlinedocs/gdb/Extending-GDB.html#Extending-GDB)
[^gdb_objfile_gdb_ext]: [GDB Auto-Loading Extensions - The objfile-gdb.ext file](https://sourceware.org/gdb/onlinedocs/gdb/objfile_002dgdbdotext-file.html#objfile_002dgdbdotext-file)
[^gdb_debug_gdb_scripts]: [GDB Auto-Loading Extensions - The .debug_gdb_scripts section](https://sourceware.org/gdb/onlinedocs/gdb/dotdebug_005fgdb_005fscripts-section.html#dotdebug_005fgdb_005fscripts-section)
[^code_confidence]: [Code Confidence - FreeRTOS Eclipse Plugin](https://www.codeconfidence.com/freertos-tools.shtml)
[^vim_plug]: [junegunn/vim-plug](https://github.com/junegunn/vim-plug)

<!-- prettier-ignore-end -->
