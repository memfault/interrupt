---
title: "gdbundle - GDB's Missing Plugin Manager"
description: ""
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
  invocation]({% post_url 2019-08-27-building-a-cli-for-firmware-projects %}#a-project-cli
  ) and loading GDB Python scripts automatically so every developer on the team
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
user-interface improvements, and utilities with the broader public. This would
enable building upon each other's work and debugging techniques, progressing the
entire community and preventing developers from reinventing the wheel time and
time again. GDB (and LLDB) needs a plugin manager, and I'd like to introduce to
you `gdbundle`.

<!-- excerpt end -->

_Like Interrupt? [Subscribe](http://eepurl.com/gpRedv) to get our latest posts
straight to your mailbox_

{:.no_toc}

## Table of Contents

<!-- prettier-ignore -->
* auto-gen TOC:
{:toc}

## Consequences of GDB's Lack of Plugin Manager

What has happened in the GDB ecosystem due to not having a plugin manager?

### No Out-Of-Box Debugging of Popular Software

Within this past year, I've worked with four different microcontroller stacks, 7
different Real-Time Operating Systems (RTOS's), and a handful of common
low-level software libraries including Mbed TLS, the WICED Bluetooth Stack, and
many mediocre vendor SDK's. Tens of thousands of developers use each one of
these stacks/libraries, and every single one of them has to manually debug each
module by hand by using GDB's print functionality or write their own scripts.

This is why embedded developers often choose to use thousand dollar or mediocre
IDE debuggers over using GDB. They have these debugging utilities built in or
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

### Monolithic, Invasive Projects

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
3. I to pull in the Mbed TLS library to be able to connect to secure servers.
4. I install the Mbed TLS GDB plugin so that I again can look at the libraries
   global state to help my debugging experience.
5. I find a bug in the Zephyr GBD plugin due to a global variable name change. I
   fork the plugin, fix the bug, and the maintainer publishes a new version of
   the plugin and now everyone can update.

The only thing we need to allow the above to happen is a standard way to write,
package, publish, and install these plugins. I'm constantly amazed that our
IDE's, GUI based text editors, and even Vim have plugin managers, and I feel
that GDB desperately needs one too.

## GDB's Current Extensibility Hooks

There are essentially three "built-in" ways to automatically load GDB scripts
upon launch.

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
> `show auto-load scripts-directory` or from thee current working directory.

For this example, we'll copy our previous file.

```
$ cp hello.gdb myexe.out-gdb.gdb
```

We can now launch GDB without nay extra arguments other than the executable and
verify that the command is loaded.

```
$ gdb myexe.out
(gdb) hello
Hello World from .gdb
```

It also works with GDB Python scripts, however the filename must end with
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

## It Doesn't Have To Be Awful

Let's take Vim for example. Vim's plugins are, at their core, a collection of
`.vim` files written in Vimscript packaged up. These "plugins" are distributed
and installed on a user's computer with one of the many Vim plugin managers,
such as [vim-plug](https://github.com/junegunn/vim-plug) or
[Vundle](https://github.com/VundleVim/Vundle.vim).

These Vim plugins range in functionality from new language syntax support,
integrations with common tools like git and grep, and even alterations to the
user-interface to make things colored, formatted, and more efficient.

What I'm trying to say is...**GDB should have a similar plugin architecture to
Vim**

| Functionality                    | Vim                                                   | GDB                                       |
| -------------------------------- | ----------------------------------------------------- | ----------------------------------------- |
| Scripts loaded on startup        | `~/.vimrc`<br>`/etc/vimrc`                            | ~/.gdbinit<br>/usr/local/share/auto-load/ |
| Scripting Language               | Vimscript                                             | GDB Commands, Python, or Guile            |
| How to manage plugins            | Plugin Managers, like vim-plug and Vundle             | gdbundle                                  |
| How to initialize plugin manager | Add snippet of code to `~/.vimrc`                     | Add snippet of code to `~/.gdbinit`       |
| How to install plugins           | Define plugins in `~/.vimrc` and run `:PluginInstall` | `pip install gdbundle-<plugin-name>`      |
| Where are plugins stored         | Downloaded and cached in user's home directory        | Python virtualenv's site-packages/        |

## Life with a Package Manager

- You've written

## `gdbundle` - GDB's Plugin Manager

gdbundle is short for GDB bundle and is a plugin manager for GDB.

To quickly summarize, gdbundle plugins:

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

In order and by default, gdbundle:

1. Searches for installed Python packages that begin with `gdbundle_`.
2. Calls the `gdbundle_load()` function of each plugin module.
3. In each plugin's `gdbundle_load()` function, it will source, import, and
   configure the GDB scripts that it has bundled inside (or from another package
   by way of a dependency).

And that is all.

### Requirements

gdbundle has a couple requirements.

#### Python-compatible GDB

gdbundle requires a GDB compiled with Python. You can check if your GDB is
configured correctly by the following quick test:

```
$ gdb
(gdb) python-interactive
>>> print("TEST")
"TEST"
```

If "TEST" is printed, everything should work! If not, there are a few things to
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

As I discussed in a previous post,
[Using PyPi Packages with GDB](https://interrupt.memfault.com/blog/using-pypi-packages-with-GDB#3-append-syspath-to-gdbs-python),
the first part of this **is required** to be able to use Python packages
installed in a virtual environment, Conda environment, or non-default Python
installation.

The second part, `gdbundle.init()`, is what is new and will load installed
gdbundle plugins.

## gdbundle Example Plugin

I've created a "Hello World" plugin for gdbundle called
[`gdbundle-example`](https://github.com/memfault/gdbundle-example). Let's dig in
and see how it's built.

### Structure

Below is the directory structure of our plugin.

```
├── README.md
├── gdbundle_example
│   ├── __init__.py
│   └── scripts
│       ├── example.gdb
│       └── example.py
└── pyproject.toml
```

Simple, right? I've opted to use `pyproject.toml` instead of `setup.py` because
I wanted to embrace the future of standardized packaging, but note that these
packages will work with Python 2 and 3, so it shouldn't matter.

### How It Works

Let's look at the code in `gdbundle_example/__init__.py`

```python
import gdb
import os

ROOT_DIR = os.path.dirname(__file__)

SCRIPT_PATHS = [
    [ROOT_DIR, 'scripts', 'example.gdb'],
    [ROOT_DIR, 'scripts', 'example.py']
]

def _abs_path(path):
    return os.path.abspath(os.path.join(*path))

def gdbundle_load():
    for script_path in SCRIPT_PATHS:
        gdb.execute("source {}".format(_abs_path(script_path)))
```

Notice the module-level `gdbundle_load` function which `gdbundle` will call
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

## The Future

{:.no_toc}

## Closing

_All the code used in this blog post is available on
[Github](https://github.com/memfault/interrupt/tree/master/example/faster-compilation/).
See anything you'd like to change? Submit a pull request!_

{:.no_toc}

## Neat GDB Script Repositories

I came across some neat repositories of GDB scripts while I was searching the
Internet. I've included them below for anyone looking to learn more about how
people use GDB Python.

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
<!-- prettier-ignore-end -->
