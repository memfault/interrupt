---
title: "Using Python PyPi Packages within GDB/LLDB"
description:
  "How to setup GDB/LLDB, virtualenv, and Python to use PyPi packages in GDB/LLDB
  scripts"
author: tyler
tags: [python, gdb]
---

> UPDATE: I've discovered that the approach in this post also works with LLDB,
> despite the
> [Python Caveat #3](https://lldb.llvm.org/resources/caveats.html#id1) mentioned
> in the LLDB documentation.

<!-- excerpt start -->

[In a previous
post]({% post_url 2019-07-02-automate-debugging-with-gdb-python-api %}), we
discussed how to automate some of the more tedious parts of debugging firmware
using
[Python in GDB Scripts](https://sourceware.org/gdb/onlinedocs/gdb/Python-API.html).
To make these commands more powerful, one could use third-party packages from
Python's [PyPi](https://pypi.org/) repository. In this post, we will discuss how
to properly setup GDB, Python, and optionally virtualenv and then modify the
`uuid_list_dump` command from the post mentioned above to make use of a third
party package installed through PyPi.

<!-- excerpt end -->

In the post [Automate Debugging with GDB Python
API]({% post_url 2019-07-02-automate-debugging-with-gdb-python-api %}), Chris
worked through an example Python GDB command which printed UUIDs stored in a
Linked List. In this post, we want to expand upon this `uuid_list_dump` command
and make the output more readable for humans using the
[PTable](https://pypi.org/project/PTable/) package from PyPi. We will print the
contents in an ASCII formatted table.

**Before:**

```
(gdb) uuid_list_dump s_list_head
Args Passed: s_list_head
Found a Linked List with 10 nodes:
0: Addr: 0x200070c0, uuid: 9d5d4440-8b5a-235e-a9ef-ec3f919c7a41
1: Addr: 0x200070a8, uuid: 8afe211b-197e-e475-8ec6-878558dd24e6
2: Addr: 0x20007088, uuid: 0f84cd49-d683-4217-e7a3-399373c6e6f1
3: Addr: 0x20007070, uuid: 23b05b1b-a32a-8288-4419-0e30f26ba3ae
4: Addr: 0x20007050, uuid: 3ca835e5-8f2a-cdc2-40c1-cf52716a7229
5: Addr: 0x20007038, uuid: 30d4349d-3a0d-0fbd-2fa1-f70fd968f4d9
6: Addr: 0x20007018, uuid: 437bbe90-1689-9d7e-77c6-2f269888f5b4
7: Addr: 0x20007000, uuid: 5e20fb38-a84e-a614-9325-562444df598d
8: Addr: 0x20006fe0, uuid: 7a96092c-a557-7464-c4af-1528a4e957db
9: Addr: 0x20006fc8, uuid: 2dcf4629-04b4-78d8-68a7-ff3f2bf1fcd9
```

**After:**

```
(gdb) uuid_list_dump s_list_head
Args Passed: s_list_head
Found a Linked List with 10 nodes:
+-----+-------------+--------------------------------------+
| Idx |     Addr    |                 UUID                 |
+-----+-------------+--------------------------------------+
|  0  | 0x1002002d0 | 008e7897-970e-3462-07d7-5e47a158298e |
|  1  | 0x1002002b0 | deb2f6b5-e24c-4fdd-9f88-67bdce1ff261 |
|  2  | 0x100200290 | 818fc33a-1e0f-c02c-4455-7ac8ab50c9b2 |
|  3  | 0x100200270 | 0ce10698-6d61-ff34-a11e-19fd3650e9b7 |
|  4  | 0x100200250 | d3feeda5-d5f3-d9e4-5bfa-6cc351e220ae |
|  5  | 0x100200230 | e9c5f1b0-c415-8ae5-9b4d-39f6f7e8a105 |
|  6  | 0x100200210 | 02353981-84b8-14a2-9cb4-5a672acae548 |
|  7  | 0x1002001f0 | 48336241-f30d-23e5-5f30-d1c8ed610c4b |
|  8  | 0x100200080 | 17119854-2f11-2d05-58f5-6bd688079992 |
|  9  | 0x100200060 | a7f1d92a-82c8-d8fe-434d-98558ce2b347 |
+-----+-------------+--------------------------------------+
```

This is incredibly easy to do using the
[PTable](https://pypi.org/project/PTable/) package:

```python
x = PrettyTable()
x.field_names = ["Field Name 1", ...]
x.add_row(["Value 1", ...])
...
print(x)
```

Let's go ahead and try it out!

{% include newsletter.html %}

## Setting up GDB's Python and PyPi

> If you want to know the solution right now and skip over our investigation,
> continue from [here](#setting-syspath-within-gdbinit).

### Environment

Throughout this tutorial, I will be using:

- macOS 10.14 using Python 2.7.15 installed using [Brew](https://brew.sh/) along
  with a virtualenv environment using
  [virtualenv](https://docs.python-guide.org/dev/virtualenvs/#lower-level-virtualenv).
- `arm-none-eabi-gdb-py` which is GDB 8.2 compiled against the macOS System
  Python 2.7.10.

Even though the tutorial is written on macOS, the steps mentioned apply directly
to Linux and likely Windows as well.

#### Virtual Environment

When installing Python packages, it's always best to use a virtual environment.
This is stressed in probably every Python tutorial, and I'll summarize their
advice here:

- Think twice before using `sudo pip`. The only place this may be appropriate is
  within a disposable environment, such as a virtual machine or a Docker image,
  or for installing the `virtualenv` package.
- If on a normal operating system installation, use a virtual environment.
  [This guide](https://docs.python-guide.org/dev/virtualenvs/#lower-level-virtualenv)
  is a good starting point.
- If using macOS, use
  [Brew to install Python](https://docs.brew.sh/Homebrew-and-Python) and don't
  touch your system Python at all (**never** use `sudo`)

In this post, we will be using a Python 2.7 virtual environment.

### Installing PTable and Initial Test

The first thing we need to do is install the package. Let's create and activate
our virtual environment and install PTable using `pip`.

```
$ virtualenv -p /usr/bin/python2.7 venv
$ source venv/bin/activate
(venv)
$ pip install PTable
Collecting ptable
  Downloading https://files.pythonhosted.org/packages/ab/b3/b54301811173ca94119eb474634f120a49cd370f257d1aae5a4abaf12729/PTable-0.9.2.tar.gz
Installing collected packages: ptable
  Running setup.py install for ptable ... done
Successfully installed ptable-0.9.2
```

Great! We double check that it's installed and everything checks out.

```
(venv)
$ python
Python 2.7.15 (default, Feb 12 2019, 11:00:12)
[GCC 4.2.1 Compatible Apple LLVM 10.0.0 (clang-1000.11.45.5)] on darwin
>>> import prettytable
>>> # It worked!
```

However, when we launch GDB and launch the Python shell, the package isn't
there, regardless of whether our virtual environment was activated or not when
we launched GDB.

```
(venv)
$ arm-none-eabi-gdb-py
GNU gdb (GNU Tools for Arm Embedded Processors 8-2018-q4-major) 8.2.50.20181213-git
...
(gdb) python-interactive
>>> import prettytable
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
ImportError: No module named prettytable
```

> NOTE: One can also use the GDB command `pi` , which is short for
> `python-interactive`

### Investigation into `sys.path`

Usually, when one experiences issues with Python package imports, the list of
paths within `sys.path` is invalid, has missing entries, or contains the wrong
package locations. This can be caused by two common reasons:

- Using the wrong Python executable, e.g. Mac's system Python at
  `/usr/bin/python` vs the Brew installation at `/usr/local/bin/python2`
- Overriding the values at `sys.path` using Pythons' `PYTHONPATH` environment
  variable

Lets compare the value of `sys.path` between our local Python shell and the GDB
Python environment.

#### macOS System

```python
(venv)
$ python
Python 2.7.15 (default, Feb 12 2019, 11:00:12)
[GCC 4.2.1 Compatible Apple LLVM 10.0.0 (clang-1000.11.45.5)] on darwin
>>> import sys
>>> sys.version
'2.7.15 (default, Feb 12 2019, 11:00:12) \n[GCC 4.2.1 Compatible Apple LLVM 10.0.0 (clang-1000.11.45.5)]'
>>> sys.path
['',
 '/Users/tyler/venv/lib/python27.zip',
 '/Users/tyler/venv/lib/python2.7',
 '/Users/tyler/venv/lib/python2.7/plat-darwin',
 '/Users/tyler/venv/lib/python2.7/plat-mac',
 '/Users/tyler/venv/lib/python2.7/plat-mac/lib-scriptpackages',
 '/Users/tyler/venv/lib/python2.7/lib-tk',
 '/Users/tyler/venv/lib/python2.7/lib-old',
 '/Users/tyler/venv/lib/python2.7/lib-dynload',
 '/usr/local/Cellar/python@2/2.7.15_3/Frameworks/Python.framework/Versions/2.7/lib/python2.7',
 '/usr/local/Cellar/python@2/2.7.15_3/Frameworks/Python.framework/Versions/2.7/lib/python2.7/plat-darwin',
 '/usr/local/Cellar/python@2/2.7.15_3/Frameworks/Python.framework/Versions/2.7/lib/python2.7/lib-tk',
 '/usr/local/Cellar/python@2/2.7.15_3/Frameworks/Python.framework/Versions/2.7/lib/python2.7/plat-mac',
 '/usr/local/Cellar/python@2/2.7.15_3/Frameworks/Python.framework/Versions/2.7/lib/python2.7/plat-mac/lib-scriptpackages',
 '/Users/tyler/venv/lib/python2.7/site-packages']
```

My shell's Python2 is from the Brew Python 2.7.15 installation and is importing
packages from the `site-packages/` directory within the virtual environment that
I have activated. This is perfect.

#### GDB

Now we launch `arm-none-eabi-gdb-py`, start a Python shell, and check
`sys.path`.

```python
(venv)
$ arm-none-eabi-gdb-py
GNU gdb (GNU Tools for Arm Embedded Processors 8-2018-q4-major) 8.2.50.20181213-git
...
(gdb) pi
>>> import sys
>>> sys.version
'2.7.10 (default, Aug 17 2018, 19:45:58) \n[GCC 4.2.1 Compatible Apple LLVM 10.0.0 (clang-1000.0.42)]'
>>> sys.path
['/Users/tyler/.local/gcc-arm-none-eabi-8-2018-q4-major/arm-none-eabi/share/gdb/python',
 '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python27.zip',
 '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7',
 '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/plat-darwin',
 '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/plat-mac',
 '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/plat-mac/lib-scriptpackages',
 '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/lib-tk',
 '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/lib-old',
 '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/lib-dynload',
 '/Library/Python/2.7/site-packages',
 '/System/Library/Frameworks/Python.framework/Versions/2.7/Extras/lib/python',
 '/System/Library/Frameworks/Python.framework/Versions/2.7/Extras/lib/python/']
```

It looks like GDB is using the macOS System Python version 2.7.10 and is pulling
in packages from the system installation at
`'/Library/Python/2.7/site-packages'`. This is _wrong_, as it should be looking
in our virtual environment's `site-packages/` directory.

So how do we get the `sys.path` value within GDB's Python updated so I can
import PrettyTable?!

### Setting `sys.path` within `~/.gdbinit`

We need three things:

1. Find a way to modify GDB on initialization
2. Extract our virtual environment's `sys.path` values
3. Append these paths to GDB's Python `sys.path` value.

#### 1. Modifying GDB on launch

To modify every instance of GDB that is launched, we will edit our `~/.gdbinit`
file. However, there are _many_ other ways to modify how GDB is launched, and
those are covered in detail
[here](http://sourceware.org/gdb/onlinedocs/gdb/Startup.html) in GDB's
documentation. In summary, here is the order of steps GDB takes during launch:

- Read and execute the system `gdbinit` file located at `/usr/share/gdb/gdbinit`
  or similar
- Read and execute `~/.gdbinit`
- Execute commands provided by args `-ix` and `-iex`
- Read and execute the local `.gdbinit` file in the current directory, provided
  `set auto-load local-gdbinit` is set to `on` (default)
- Execute commands provided by args `-ex` and `-x`

A tip to readers would be to use a combination of these setup steps, such as a
project `.gdbinit` file that is committed to the repo so that a team can share
common scripts!

#### 2. Extract `sys.path` values from `venv`

To output the value of `sys.path` in a shell, we can actually launch Python
[with a given command](https://docs.python.org/2/using/cmdline.html#cmdoption-c)
and have it print a value to standard out. Below, we import `sys` and
immediately print `sys.path` to standard out.

```python
(venv)
$ python -c "import sys;print(sys.path)"
['/Users/tyler/venv/lib/python2.7/site-packages', ...]
```

Perfect! We can see our virtual environment's `site-packages/` directory in that
list.

#### 3. Append `sys.path` to GDB's Python

Now we need to take the values from `sys.path` in
[Step #2](#2-extract-syspath-values-from-venv) and append them to GDB's
`sys.path`

In my local `~/.gdbinit` script, I will place the following code snippet at the
bottom.

```python
python
# Update GDB's Python paths with the `sys.path` values of the local
#  Python installation, whether that is brew'ed Python, a virtualenv,
#  or another system python.

# Convert GDB to interpret in Python
import os,subprocess,sys
# Execute a Python using the user's shell and pull out the sys.path (for site-packages)
paths = subprocess.check_output('python -c "import os,sys;print(os.linesep.join(sys.path).strip())"',shell=True).decode("utf-8").split()
# Extend GDB's Python's search path
sys.path.extend(paths)
end
```

This will allow any Python packages installed in the local installation or
virtual environment (the one active when launching GDB) to be accessible to GDB!

The snippet above can also be found in this
[`.gdbinit` Github Gist](https://gist.github.com/tyhoff/060e480b6cf9ad35dfd2ba9d01cad4b6).

### Test Importing PrettyTable within GDB

We can now test our setup by trying to launch GDB and import `prettytable`

```python
(venv)
$ arm-none-eabi-gdb-py
GNU gdb (GNU Tools for Arm Embedded Processors 8-2018-q4-major) 8.2.50.20181213-git
...
(gdb) pi
>>> import prettytable
>>> # Yay! It works.
```

## Using PrettyTable to Format Linked List

Now that we have GDB's Python setup to use PyPi packages, we can move on to
editing our initial UUID Linked List printer to use PrettyTable. We will start
with the previous implementation located on
[Github](https://github.com/memfault/interrupt/blob/master/example/gdb-python-post/custom_pretty_printers.py#L47-L89)
and we will use the same `.c` example located
[here]({% post_url 2019-07-02-automate-debugging-with-gdb-python-api %}#example-usage)

> If you need to get the previous `.c` file compiled and running to test out the
> following commands, refer to the [previous
> post]({% post_url 2019-07-02-automate-debugging-with-gdb-python-api %}) for
> instructions.

Below, I've removed some of the previous code and added comments about what
we'll need to do to format our Linked List with PrettyTable. Most of the
documentation of how to use PrettyTable can be found within the package's
[Tutorial](https://ptable.readthedocs.io/en/latest/tutorial.html) or by reading
the package's
[Source Code](https://github.com/kxxoling/PTable/blob/master/prettytable/prettytable.py)

```python
class UuidListDumpCmd(gdb.Command):
    """Prints the ListNode from our example in a nice format!"""

    def __init__(self):
        super(UuidListDumpCmd, self).__init__(
            "uuid_list_dump", gdb.COMMAND_USER
        )

    def _uuid_list_to_str(self, val):
        """Walk through the UuidListNode list.
        We will simply follow the 'next' pointers until we encounter NULL
        """

        # TODO: Initialize a PrettyTable object
        # TODO: Add Columns to the PrettyTable using `.add_columns()`

        idx = 0
        node_ptr = val
        result = ""
        while node_ptr != 0:
            uuid = node_ptr["uuid"]

            # TODO: Add PrettyTable row using `.add_row()`

            node_ptr = node_ptr["next"]
            idx += 1

        # TODO: Convert the PrettyTable object to a string

        result = ("Found a Linked List with %d nodes:" % idx) + result
        return result

    def complete(self, text, word):
        # We expect the argument passed to be a symbol so fallback to the
        # internal tab-completion handler for symbols
        return gdb.COMPLETE_SYMBOL

    def invoke(self, args, from_tty):
        # We can pass args here and use Python CLI utilities like argparse
        # to do argument parsing
        print("Args Passed: %s" % args)

        node_ptr_val = gdb.parse_and_eval(args)
        if str(node_ptr_val.type) != "UuidListNode *":
            print("Expected pointer argument of type (UuidListNode *)")
            return

        print(self._uuid_list_to_str(node_ptr_val))
```

Below, we fill in the gaps by initializing a `PrettyTable` object with columns
and while iterating over each node, adding a row with the appropriate data.

```python
def _uuid_list_to_str(self, val):
        """Walk through the UuidListNode list.
        We will simply follow the 'next' pointers until we encounter NULL
        """

        # Initialize PrettyTable and add columns
        x = PrettyTable()
        x.field_names = ["Idx", "Addr", "UUID"]

        idx = 0
        node_ptr = val
        result = ""
        while node_ptr != 0:
            uuid = node_ptr["uuid"]

            # Add a new row for each UUID in the Linked List
            x.add_row([idx, str(node_ptr), str(uuid)])

            node_ptr = node_ptr["next"]
            idx += 1

        # Convert the table to a string and prepend the count string.
        result = x.get_string()
        result = ("Found a Linked List with %d nodes:" % idx) + result
        return result
```

And that's it! If we load GDB, source the script, run our example code, and run
the GDB command, we get our desired output:

```
(gdb) source gdb_uuid_list.py
(gdb) uuid_list_dump s_list_head
Args Passed: s_list_head
Found a Linked List with 10 nodes:
+-----+-------------+--------------------------------------+
| Idx |     Addr    |                 UUID                 |
+-----+-------------+--------------------------------------+
|  0  | 0x1002002d0 | 008e7897-970e-3462-07d7-5e47a158298e |
|  1  | 0x1002002b0 | deb2f6b5-e24c-4fdd-9f88-67bdce1ff261 |
|  2  | 0x100200290 | 818fc33a-1e0f-c02c-4455-7ac8ab50c9b2 |
|  3  | 0x100200270 | 0ce10698-6d61-ff34-a11e-19fd3650e9b7 |
|  4  | 0x100200250 | d3feeda5-d5f3-d9e4-5bfa-6cc351e220ae |
|  5  | 0x100200230 | e9c5f1b0-c415-8ae5-9b4d-39f6f7e8a105 |
|  6  | 0x100200210 | 02353981-84b8-14a2-9cb4-5a672acae548 |
|  7  | 0x1002001f0 | 48336241-f30d-23e5-5f30-d1c8ed610c4b |
|  8  | 0x100200080 | 17119854-2f11-2d05-58f5-6bd688079992 |
|  9  | 0x100200060 | a7f1d92a-82c8-d8fe-434d-98558ce2b347 |
+-----+-------------+--------------------------------------+
```

## Closing

As we've said
[before]({% post_url 2019-07-02-automate-debugging-with-gdb-python-api %}) and
will continue to stress, using the GDB Python API takes debugging to another
level by allowed one to automate tedious tasks and make complex ones
reproducible and shareable across teams.

The above steps will get any GDB up and running using third-party PyPi packages.
If you want a simple snippet to use or share with teammates about how to set up
their GDB for PyPi package use, you can use
[this `.gdbinit` Github Gist](https://gist.github.com/tyhoff/060e480b6cf9ad35dfd2ba9d01cad4b6).
If you use `lldb`, here is an
[`.lldbinit` Github Gist](https://gist.github.com/tyhoff/7a286945ef75947ad49a347dbc8708ca).

_All the code used in this blog post is available on
[Github](https://github.com/memfault/interrupt/tree/master/example/gdb-python-pypi-post/)._
{% include submit-pr.html %}
