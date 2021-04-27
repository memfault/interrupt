---
title: "Automate Debugging with GDB Python API"
description: "How to write GDB Python scripts and Pretty Printers using the GDB Python API"
author: chris
tags: [python, gdb]
---

<!-- excerpt start -->

[Previously]({% post_url 2019-05-21-gdb-for-firmware-1 %}) we discussed how a significant portion of developer time is spent _debugging_ firmware and how GDB can be a powerful utility for this. In this article we will discuss how to become more efficient at debugging by leveraging GDB's [Python API](https://sourceware.org/gdb/onlinedocs/gdb/Python.html#Python).

<!-- excerpt end -->

> If you'd rather listen to me present this information and see some demos in action, [watch this webinar recording.](https://go.memfault.com/debugging-arm-cortex-m-mcu-webinar?utm_campaign=Debugging%20Cortex%20M%20Webinar&utm_source=blog&utm_medium=Interrupt&utm_term=Debug)

When a system hits a failure state, you will often find yourself looking at similar state to determine how the system got into the failure mode. This may entail inspecting linked lists, heaps, OS primitives such as mutexes or queues, state of peripherals, etc

Perhaps you are guilty of doing things like this to determine how much memory was burned up in a linked list:

```
(gdb) p s_list_head
$5 = (UuidListNode *) 0x200070c0
(gdb) p s_list_head->next
$6 = (struct UuidListNode *) 0x200070a8
(gdb) p s_list_head->next->next
$7 = (struct UuidListNode *) 0x20007088
(gdb) p s_list_head->next->next->next
$8 = (struct UuidListNode *) 0x20007070
(gdb) p s_list_head->next->next->next->next
$5 = (struct ListNode *) 0x0
```

While all this manual typing may be fine for debugging one crash or issue, it does not scale. When a new firmware is being QA'd for the first time on hundreds of devices, it's important to be able to quickly classify the issue so time spent debugging doesn't cause the schedule to slip. Even once a product makes it into the wild, an issue that appears on .1% of devices in a one million device fleet would generate 1000 reports per month!

Manually inspecting hundreds of crashes is no fun ... but fortunately, GDB allows users to extend its core functionality in a number of ways, including via its [Python API](https://sourceware.org/gdb/onlinedocs/gdb/Extending-GDB.html#Extending-GDB) . This allows one to accelerate and in many cases even **automate** debugging!

In this series of posts we will explore some of the GDB Python APIs and how they can be used.

{% include newsletter.html %}

## Getting started with GDB Python

GDB's [Python API](https://sourceware.org/gdb/onlinedocs/gdb/Python.html#Python) was introduced as part of GDB 7.0 (all the way back in 2009!) and continues to receive new features and improvements as gdb is updated (look for "Python" in the [gdb release notes](https://www.gnu.org/software/gdb/news/))

### Check if GDB was compiled with Python support

The GDB Python API is a GDB compile time option that can be enabled (with the `--with-python` configuration argument). We can easily check if gdb has this feature enabled by checking:

```bash
$ /path/to/gdb  --config
This GDB was configured as follows:
[...]
             --with-python=/usr
[...]
```

If you are doing any type of ARM Cortex-M development, you are in luck, the [official ARM Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) publishes a GDB variant with Python enabled at `./bin/arm-none-eabi-gdb-py` in each release!

To use it you just need Python installed on your system (i.e On ubuntu, `apt-get install python2.7 python-dev` or on OSX at the moment, the pre-installed system Python is used)

## Example Usage

Let's walk through a trivial example snippet of code to get an idea of how some of the APIs work!

The snippet tracks UUIDs in RAM in a linked list. On a real embedded system, we may have a bunch of items, identified by their UUID, stored on an external flash. If the list of items is looked at frequently, it may be cached in RAM for faster access and better power.

```c
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct Uuid {
  uint8_t bytes[16];
} Uuid;

typedef struct UuidListNode {
  struct UuidListNode *next;
  Uuid uuid;
} UuidListNode;

static UuidListNode *s_list_head = NULL;

void list_add_uuid(const Uuid *uuid) {
  UuidListNode *node_to_add = malloc(sizeof(UuidListNode));
  assert(node_to_add != NULL);

  *node_to_add = (UuidListNode) {
    .uuid = *uuid,
  };

  // Add entry to the front of the list
  node_to_add->next = s_list_head;
  s_list_head = node_to_add;
}

//
// Code to populate the list with fake data
//

// A completely fake uuid-generator implementation
static void generate_fake_uuid(Uuid *uuid) {
  for (size_t i = 0; i < sizeof(*uuid); i++) {
    uuid->bytes[i] = rand() & 0xff;
  }
}

static void generate_fake_uuid_list(void) {
  for (int i = 0; i < 10; i++) {
    Uuid uuid;
    generate_fake_uuid(&uuid);
    list_add_uuid(&uuid);
  }
}

int main(void) {
  generate_fake_uuid_list();
  // Let's just spin loop so we can break at any point and dump the list
  while (1) { }
  return 0;
}
```

### GDB Pretty Printers

When we print a structure, GDB will display each of the individual fields. The `s_list_head` in the code above would look something like this:

```
(gdb) p s_list_head
$1 = (UuidListNode *) 0x200070c0
(gdb) p *s_list_head
$2 = {next = 0x200070a8, uuid = {bytes = "\235]D@\213Z#^\251\357\354?\221\234zA"}}
```

We can clean it up a little by enabling pretty printing which will put each field on a newline:

```
(gdb) set print pretty
(gdb) p s_list_head
$4 = (UuidListNode *) 0x200070c0
(gdb) p *s_list_head
$5 = {
  next = 0x200070a8,
  uuid = {
    bytes = "\235]D@\213Z#^\251\357\354?\221\234zA"
  }
}
```

We can take it a step farther by [writing our own pretty printers](https://sourceware.org/gdb/onlinedocs/gdb/Writing-a-Pretty_002dPrinter.html#Writing-a-Pretty_002dPrinter).

UUIDs are typically displayed as a string with the format `xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx`. Let's see if we can update GDB to display our `Uuid` struct like this!

#### Writing a Custom Pretty Printer with GDB Python

To add a custom pretty printer, we just need to:

- define a class that can detect whether or not pretty printing is supported given a particular type. The detector needs to
  define a `__call__` function which given a [`gdb.Value`](https://sourceware.org/gdb/onlinedocs/gdb/Values-From-Inferior.html#Values-From-Inferior) either returns a custom printer or `None` if it can't handle the value.
- The printer class itself, which needs to define a `to_string` function that GDB will call to display the value.

Let's start with some boilerplate for a UUID pretty printer implementation in a file called `custom_gdb_extensions.py`:

```python
from gdb.printing import PrettyPrinter, register_pretty_printer
import gdb
import uuid

class UuidPrettyPrinter(object):
    """Print 'struct Uuid' as 'xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx'"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "TODO: Implement"


class CustomPrettyPrinterLocator(PrettyPrinter):
    """Given a gdb.Value, search for a custom pretty printer"""

    def __init__(self):
        super(CustomPrettyPrinterLocator, self).__init__(
            "my_pretty_printers", []
        )

    def __call__(self, val):
        """Return the custom formatter if the type can be handled"""

        typename = gdb.types.get_basic_type(val.type).tag
        if typename is None:
            typename = val.type.name

        if typename == "Uuid":
            return UuidPrettyPrinter(val)
```

We then need to register the custom pretty print detector with gdb by appending the following to our script:

```python
register_pretty_printer(None, CustomPrettyPrinterLocator(), replace=True)
```

The `replace=True` is useful while developing a pretty printer because when the script is re-source'd, it will overwrite the previous pretty printer version. Let's source the script and see what happens! We should see a new pretty printer with the name we gave above, `my_pretty_printers`, in the list!

```
(gdb) source custom_gdb_extensions.py
(gdb) info pretty-printer
global pretty-printers:
  builtin
    mpx_bound128
  my_pretty_printers
```

Now let's try to print the head list node and see what we get:

```
(gdb) p *s_list_head
$1 = {
  next = 0x200070a8,
  uuid = TODO: Implement
}
```

Great! We see the pretty printer got called for `Uuid` and _"TODO: Implement"_ was returned which matches the current `to_string` implementation. Now let's fill it in with an actual implementation:

```python
    def to_string(self):
        # gdb.Value can be accessed just like they would be in C. Since
        # this is a Uuid struct we can index into "bytes" and grab the
        # value from each entry in the array
        array = self.val["bytes"]
        # Format the byte array as a hex string so Python uuid module can
        # be used to get the string
        uuid_bytes = "".join(
            ["%02x" % int(array[i]) for i in range(0, array.type.sizeof)]
        )
        return str(uuid.UUID(uuid_bytes))
```

Let's re-source the script and see what we get now:

```
(gdb) source custom_gdb_extensions.py
(gdb) p *s_list_head
$6 = {
  next = 0x200070a8,
  uuid = 9d5d4440-8b5a-235e-a9ef-ec3f919c7a41
}
(gdb) p *s_list_head->next
$7 = {
  next = 0x20007088,
  uuid = 8afe211b-197e-e475-8ec6-878558dd24e6
}
```

### GDB Custom Commands

GDB also let's the user define custom commands to aid in debugging. The list of custom commands available can always easily be found by running `help user-defined` from within GDB

In our example above it's great we can now see the Uuid formatted in a normal way but it's still painful to display the entire list or compute it's size. We still have to walk it manually ...

```
(gdb) p *s_list_head->next->next->next->next->next->next->next->next
$8 = {
  next = 0x20006fc8,
  uuid = 7a96092c-a557-7464-c4af-1528a4e957db
}
(gdb) p *s_list_head->next->next->next->next->next->next->next->next->next
$9 = {
  next = 0x0,
  uuid = 2dcf4629-04b4-78d8-68a7-ff3f2bf1fcd9
}
```

Instead, let's add a custom GDB command that can do the work for us!

#### Adding a Custom GDB Command with GDB Python

New CLI commands can be added via a Python script by creating a new subclass of [gdb.Command](https://sourceware.org/gdb/onlinedocs/gdb/Commands-In-Python.html#Commands-In-Python)

We can use the `gdb.parse_and_eval` utility to convert arbitrary arguments passed to the command into a `gdb.Value`. From there we can index into C struct members to traverse the object. In this example we will parse a `UuidListNode` pointer and following the `next` pointer until we reach the end of the list. Let's add the following to our `custom_gdb_extensions.py` script:

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
        idx = 0
        node_ptr = val
        result = ""
        while node_ptr != 0:
            uuid = node_ptr["uuid"]
            result += "\n%d: Addr: 0x%x, uuid: %s" % (idx, node_ptr, uuid)
            node_ptr = node_ptr["next"]
            idx += 1
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

To actually load the command all we need to do is instantiate the object by adding the following to the end of `custom_gdb_extensions.py`:

```python
UuidListDumpCmd()
```

The command should then appear in our user-defined list:

```
(gdb) help user-defined
User-defined commands.
The commands in this class are those defined by the user.
Use the "define" command to define a command.

List of commands:

uuid_list_dump -- Prints the ListNode from our example in a nice format!
```

Let's try it out!

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

Or we could even start dumping the list in the middle. For example, let's take the Address at entry `5`:

```
(gdb) uuid_list_dump (UuidListNode*)0x20007038
Args Passed: (UuidListNode*)0x20007038
Found a Linked List with 5 nodes:
0: Addr: 0x20007038, uuid: 30d4349d-3a0d-0fbd-2fa1-f70fd968f4d9
1: Addr: 0x20007018, uuid: 437bbe90-1689-9d7e-77c6-2f269888f5b4
2: Addr: 0x20007000, uuid: 5e20fb38-a84e-a614-9325-562444df598d
3: Addr: 0x20006fe0, uuid: 7a96092c-a557-7464-c4af-1528a4e957db
4: Addr: 0x20006fc8, uuid: 2dcf4629-04b4-78d8-68a7-ff3f2bf1fcd9
```

### Auto Loading scripts

You can automatically load custom extensions by adding the following to your [gdbinit](https://sourceware.org/gdb/onlinedocs/gdb/gdbinit-man.html) file which is invoked anytime gdb starts

```
source /path/to/custom_gdb_extensions.py
```

Alternatively, you could also get this to run automatically when gdb is started by adding `--ex="source /path/to/custom_gdb_extensions.py"` as an argument

## Closing

We hope this post gave you a useful overview of how to add custom GDB commands and pretty printers using the Python API. Sometimes adding GDB commands like these can even be used to reduce [code size]({% post_url 2019-06-06-best-firmware-size-tools %}) by replacing a on-device CLI command that would display the same information!

Do you already have some ideas about how the Python API could be applied to automate parts of your debugging flow ... perhaps a command to walk custom heaps or display the contents in a memory-mapped filesystem? Or maybe you are already have some great examples of how you have used GDB Python? Either way, let us know in the discussion area below!

Next time, we'll talk about how to use third-party Python packages within GDB using virtual environments.

_EDIT: Post written!_ - [Using Python PyPi Packages with GDB]({% post_url 2019-07-23-using-pypi-packages-with-GDB %})

_All the code used in this blog post is available on [Github](https://github.com/memfault/interrupt/tree/master/example/gdb-python-post/)._

> Interested in learning more about debugging HardFaults? [Watch this webinar recording.](https://go.memfault.com/debugging-arm-cortex-m-mcu-webinar?utm_campaign=Debugging%20Cortex%20M%20Webinar&utm_source=blog&utm_medium=Interrupt&utm_term=Debug).

{% include submit-pr.html %}
