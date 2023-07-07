---
title: Building a Tiny CLI Shell for Tiny Firmware
description:
  A command-line shell is a powerful way to interface with embedded devices,
  giving the developer the ability to automate tasks, inspections, and testing.
author: tyler
image: /img/firmware-shell/shell.png
tags: [best-practices, mcu]
---

The most common and simplest interface to a computer and operating system is a
command-line shell. It allows for quick operations, the ability to automate
operations through scripting, and is light on bandwidth usage.

A class of devices which makes heavy use of command-line shells are low-power
embedded devices running bare-metal or with an RTOS. They are
resource-constrained and have limited bandwidth, so a text-based interface is
ideal.

<!-- excerpt start -->

In this post, we go over why an embedded firmware should include a command-line
shell interface, common use cases for it, how to build and extend one, and cover
common issues that developers run into while build and maintaining one. The
shell we will build will work over a UART serial port, which should make it
applicable to most embedded systems.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Why Build a Device Shell?

During all phases of development, you need a way to interact with the device.
You need to be able to query the device for its configuration, state, and
information for debugging purposes, as well as send commands for setup and
operation. For most devices, the easiest way is to have a direct serial
connection. However, just having a serial connection is not enough. There needs
to be a structured protocol on top of the serial port, and having a shell could
meet those criteria.

There are alternatives to building a CLI over serial that should be mentioned.
I've worked on projects that use binary protocols and interfaces on top of a
serial connection, either by using custom byte-packing, Protobuf, or CBOR. I've
also heard of projects not use a serial connection at all, and instead use an
existing Bluetooth or Wi-Fi connection to channel data over. The problem with
both of these alternatives is that to make these connections and protocols
_useful_ and _reliable_ to the developer and automated systems, many extra
layers need to be built on top of them, including a shell. Bluetooth and Wi-Fi
may also fail due to software bugs or radio interference. This is not the case
with serial over UART.

## Common Use Cases

There are many reasons to build a command-line shell to a device. Here are the
most common reasons I have found.

### Inspections, Validations, & Overrides

Using a device shell is a great way for a developer to bypass the layers and
systems that would normally be in place in production. For example, a device
might communicate directly with a centralized cloud service or mobile phone to
receive its up-to-date configuration, such as its name or individual function
within a collection of devices.

When doing internal development, these extra layers are usually not important
for certain tasks, such as bringing up a peripheral or low-level system like a
file system. You would be better off setting the configuration yourself or even
mocking it out. It could also be the case that the cloud infrastructure or
mobile phone companion application isn't ready yet.

Below are some example commands that would normally be set through formalized
infrastructure but are easily set through a shell and make it easy for a
developer to configure the device quickly.

```
$ device_name set "Kitchen Timer"
> OK

$ bluetooth_address get
> F0:5C:12:E3:02:BC

$ wifi set MyHotSpot WPA2 mypassword
> OK
```

A problem that some might find with the shell is that it isn't testing the
production interfaces or processes. For example, if a device's name is normally
set through a web portal and sent down to the device over Wi-Fi, adding a
command in the shell to set the device name could be seen as a bad idea. By
doing so, developers aren't testing the normal path that customers will use and
the process may inadvertently break due to lack of testing.

In practice, this is a rare occurrence given the end-to-end flows are formally
tested in QA or automated tests. A CLI allows developers to configure their
devices quickly and get to work without standing up a massive amount of
infrastructure.

### Assembly Line Provisioning & Testing

Along a factory assembly line, it is common to talk to devices on the line over
UART because they work on Windows, Linux, and macOS and do not require an
Internet connection. Trying to work with USB, Ethernet, or other protocols
introduces complexity to the process and increases the failure rate.

During assembly, different stations will perform tasks on a single unit, such as
populating various chips or buttons and testing them, provisioning private keys
into a device's persistent storage, or burning in the device's serial number.
Each station needs to be able to communicate with the device, run several
commands or verifications, and receive a pass or fail response.

Each of these stations could run a set of shell commands and automate the
validation of responses to ensure a device meets expectations.

### Integration & Automated Tests

Every firmware project ultimately wants to have a way to perform tests on real
hardware. Software unit testing can cover the vast majority of software testing,
but it can not reproduce 100% what is happening on a real device. There are also
methods to do "hardware unit testing", where a single module (e.g. Bluetooth
chip) is controlled by a test harness.

The easiest way to interact with a device to run a suite of tests that exercise
the full stack is to write shell commands that return pass or fail status codes.

For example, a basic test that can be run is a factory reset. Almost every
firmware has one, and it is a feature that should never, ever break. We can
assume we have a command on our shell called `factory_reset`, and it will either
return `OK` or `FAIL,<code>`

```
$ factory_reset
> OK

$ factory_reset
> FAIL,6
```

This test can be one of many run in QA, and since it runs on the device you can
be 99% sure that it works properly. The only missing piece that isn't tested
through the shell is a button combination or a settings menu that would normally
trigger the reset.

## Building a Basic Shell

It's now time to build up a basic shell implementation, which we will expand
upon to cover the features that are most important for a good developer and QA
experience.

### Setup

For this setup we will use:

- a nRF52840-DK[^nrf5_sdk] (ARM Cortex-M4F) as our development board
- SEGGER JLinkGDBServer[^jlink] as our GDB Server
- GCC 8.3.1 / GNU Arm Embedded Toolchain as our compiler[^gnu_toolchain]
- GNU make as our build system

To keep things simpler in the blog post, we'll be modifying an example of the
nRF52 SDK, and will be using the soft device SDK. We will be modifying the
`nrf5_sdk/examples/peripheral/uart` example.

The only thing we need from the SDK is a `putc` and `getc` implementation, which
the nRF52 SDK provides as `app_uart_get` and `app_uart_put`[^sdk_uart]. As long
as you have the equivalent in your firmware project, the examples should easily
apply.

All the code can be found on the
[Interrupt Github page](https://github.com/memfault/interrupt/tree/master/example/firmware-shell)
with more details in the `README` in the directory linked.

To download the code yourself:

```
$ git clone https://github.com/memfault/interrupt.git
$ cd examples/firmware-shell

# Build and flash device
$ make part1_flash
```

### Input and Output

The first thing we need to figure out is how to receive and print characters to
the serial port. In the nRF52 SDK, the two functions `app_uart_get` and
`app_uart_put` are given to us. Let's wrap these so they are easier to use for
our use case.

```c
#include "app_error.h"
#include "app_uart.h"

void console_putc(char c) {
  app_uart_put(c);
}

char console_getc(void) {
  // Blocks until a character is captured from the UART
  uint8_t cr;
  while (app_uart_get(&cr) != NRF_SUCCESS);
  return (char)cr;
}
```

We can now easily get and put characters from the serial port. Let's now build a
way to capture full lines more easily. We take the implementation from
libopencm3's uart_console example[^libopencm3_uart_console], which provides a
clean `console_gets` implementation.

```c
void console_puts(char *s) {
  while (*s != '\0') {
    if (*s == '\r') {
      console_putc('\n');
    } else {
      console_putc(*s);
    }
    s++;
  }
}

int console_gets(char *s, int len) {
  char *t = s;
  char c;

  *t = '\000';
  /* read until a <LF> is received */
  while ((c = console_getc()) != '\n') {
    *t = c;
    console_putc(c);
    if ((t - s) < len) {
      t++;
    }
    /* update end of string with NUL */
    *t = '\000';
  }
  return t - s;
}
```

> NOTE: We are using newlines instead of carriage returns. Make sure to
> configure the console as such!

With these two functions implemented, we now can build our main loop that
constantly consumes input from the shell and outputs it to the user.

```c
int main(void) {
  [...]

  char buf[128];
  int len;
  while (true) {
    console_puts("$ ");
    len = console_gets(buf, 128);
    console_putc('\n');
    if (len) {
      console_puts("\n");
      console_puts("OK");
      console_puts("\n");
    }
  }
}
```

To view the entire file, including the base nRF52 required initialization,
please check out the
[`main.c` on Github](https://github.com/memfault/interrupt/tree/master/example/firmware-shell/part1/main.c).

We can launch PySerial's `miniterm.py`[^miniterm] to connect to my nRF52. When I
initialized the UART, I used 115200 as the baud rate, so we'll make sure to pass
it as an argument.

```
$ make part1_flash
[...]

$ miniterm.py "/dev/cu.usbmodem*" 115200 --eol LF
--- Miniterm on /dev/cu.usbmodem0006839616591  115200,8,N,1 ---
--- Quit: Ctrl+] | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H ---
$ test
> OK
$ another
> OK
```

And that's a bare-bones console! What remains now is parsing the command name
and arguments. However, instead of building upon this example, we are going to
take the software engineering approach and use what already is out there and
open-sourced.

### Integrating Memfault's CLI Shell

I have pulled in an example of the command-line shell we use at Memfault for
tiny embedded devices and copied it into the example repository for this post,
This is what we'll use going forward.

This CLI was chosen primarily for its simplicity, ease of use of arguments, and
registering new commands, and because, frankly, there aren't many open-source
shells with firmware in mind.

Our goal in this section is to build a simple Hello World example with this new
shell implementation.

#### Build System Integration

Let's first go through what you would need to do to copy the Memfault CLI
implementation into your own project's build system.

```
$ cd example/firmware-shell/part2/
```

Next, you would copy the following directory and files into your project.

```
part2/shell
├── include
│   └── shell
│       └── shell.h
└── src
    └── shell.c
```

Last, add the following paths to your build system.

```
SRC_FILES += \
    shell/src/shell.c \
    shell_commands.c  \   # Doesn't exist yet!

INC_FOLDERS += \
    shell/include
```

#### Main Loop Integration

Now that the project is integrated into our build system, let's initialize the
shell in our `main.c` file. Thankfully, it's very simple since we already have
`console_getc` and `console_putc` built already.

We replace our previous `while ()` loop with one that passes all characters
consumed from the UART to the shell implementation.

```c
int main(void) {
  [...]

  // Configure shell
  sShellImpl shell_impl = {
    .send_char = console_putc,
  };
  shell_boot(&shell_impl);

  // Consume and send characters to shell...forever
  char c;
  while (true) {
    c = console_getc();
    shell_receive_char(c);
  }
}
```

> TIP: It's worthwhile to look at the source code to
> [`shell_receive_char`](https://github.com/memfault/interrupt/blob/master/example/firmware-shell/part2/shell/src/shell.c#L126-L140).

#### Creating and Registering Commands

The last piece of our Hello World example is to create and register a command
with the shell. To do this, create a file called `shell_commands.c` (which we've
already added to our build system) and paste in the following code:

```c
#include "shell/shell.h"

#include <stddef.h>
#include <stdio.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// A simple Hello World command which prints "Hello World!"
int cli_cmd_hello(int argc, char *argv[]) {
  shell_put_line("Hello World!");
  return 0;
}

static const sShellCommand s_shell_commands[] = {
  {"hello", cli_cmd_hello, "Say hello"},
  {"help", shell_help_handler, "Lists all commands"},
};

const sShellCommand *const g_shell_commands = s_shell_commands;
const size_t g_num_shell_commands = ARRAY_SIZE(s_shell_commands);
```

Let's break this down.

```c
int cli_cmd_hello(int argc, char *argv[]) {
  shell_put_line("Hello World!");
  return 0;
}
```

Each command registered into the shell needs to accept two arguments, `int argc`
and `char *argv[]`. The two arguments contain arguments specific to the command,
but we'll cover this in the next section.

Within the command, we call `shell_put_line` with a string, which our shell will
then print on our behalf, with a newline after. Behind the scenes, it will use
our `console_putc` which we configured the shell with earlier.

The latter part of the larger code snippet above can be understood better by
looking at the
[`include/shell/shell.h`](https://github.com/memfault/interrupt/tree/master/example/firmware-shell/part2/shell/include/shell/shell.h)
file.

```c
typedef struct ShellCommand {
  const char *command;
  int (*handler)(int argc, char *argv[]);
  const char *help;
} sShellCommand;

extern const sShellCommand *const g_shell_commands;
extern const size_t g_num_shell_commands;
```

This tells us that we need to define a `g_shell_commands` global variable, which
is a list of `sShellCommand`, each of which contains a command name, a function
pointer, and a help string, which we do in the snippet below.

```c
static const sShellCommand s_shell_commands[] = {
  {"hello", cli_cmd_hello, "Say hello"},
  {"help", shell_help_handler, "Lists all commands"},
};

const sShellCommand *const g_shell_commands = s_shell_commands;
const size_t g_num_shell_commands = ARRAY_SIZE(s_shell_commands);
```

#### Testing Our Hello World Command

And that's it! All of the code above is available
[here](https://github.com/memfault/interrupt/tree/master/example/firmware-shell/part2)
on Interrupt's Github, so be sure to check it out there for a deeper analysis.

If we compile and run this example, we can see that our simple shell works!

```
$ miniterm.py "/dev/cu.usbmodem*" 115200 --eol LF
--- Miniterm on /dev/cu.usbmodem0006839616591  115200,8,N,1 ---
--- Quit: Ctrl+] | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H ---

shell> hello
Hello World!
shell> test
Unknown command: test
Type 'help' to list all commands
```

With the use of the basic shell provided, we were able to get up and running
very quickly. If you don't already have a command-line shell to your device, I
hope this inspires you to at least get this minimal amount running.

#### Help Command

You may have noticed there is a `help` command in our shell. This allows the
user to type `help` into the shell and get a list of all the commands that are
available, along with the `char *help` string that was passed into the
registration table.

```
shell> help
hello: Say hello
help: Lists all commands
```

You can find the full source of the help command
[on Github](https://github.com/memfault/interrupt/blob/master/example/firmware-shell/part2/shell/src/shell.c#L148-L156)

### Command Arguments

In our Hello World command, there were two arguments, `int argc` and
`char *argv[]`, which we skimmed over.

```c
int cli_cmd_hello(int argc, char *argv[]) {
  shell_put_line("Hello World!");
  return 0;
}
```

These might seem cryptic at first, but this is the standard way to pass
command-line arguments to C/C++ programs.[^command_line_args]

**argc** - number of strings pointed to by `argv`<br> **argv** - 1 plus the
number of arguments passed to the function. The first argument is the command
name.<br>

To further understand how these two arguments work together in our shell, let's
build a command called `kv_write` that could hook into the Key/Value Store from
a previous Interrupt post, [Unit Testing
Basics]({% post_url 2019-10-08-unit-testing-basics %}#basic-implementation-of-keyvalue-store).
This command will take two arguments from the user, a key and value, and work in
the following way:

```
$ kv_write device_name "Device ABCD"
> OK
```

We need to write a function for the command now, which will call
`kv_store_write()` with our key and value.

```c
int cli_cmd_kv_write(int argc, char *argv[]) {
  // We expect 3 arguments:
  // 1. Command name
  // 2. Key
  // 3. Value
  if (argc != 3) {
    shell_put_line("> FAIL,1");
    return;
  }

  const char *key = argv[1];
  const char *value = argv[2];

  bool result = kv_store_write(key, value, strlen(value));
  if (!result) {
    shell_put_line("> FAIL,2");
    return;
  }
  shell_put_line("> OK");
  return 0;
}
```

> NOTE: Notice that we use a different FAIL code for each failure case. This is
> to assist in debugging when a failure is hit.

The final part would be to add it to the list within `shell_commands.c`.

```c
static const sShellCommand s_shell_commands[] = {
  {"kv_write", cli_cmd_kv_write, "Write a Key/Value pair"},  // Added!
  {"hello", cli_cmd_hello, "Say hello"},
  {"help", shell_help_handler, "Lists all commands"},
};
```

## Automating Shell Interaction with Python

One of the neatest things about building a formatted and consistent shell
interface is that you can then automate interactions with it! Using our example
code above running on our nRF52, the following code will connect to the serial
port, write the command `help`, capture the output, and print that to the user.

```python
#!/usr/bin/env python

import serial
import io

# Connect to a device matching what we want
port = list(list_ports.grep(r"/dev/cu.usbmodem*"))[0]
ser = serial.Serial(port.device, 115200, timeout=1)
sio = io.TextIOWrapper(io.BufferedRWPair(ser, ser))

# Write our command and send immediately
sio.write("hello\n")
sio.flush()

# Our serial implementation prints the input to the shell
command = sio.readline()
# Read the response on the next line
response = sio.readline()
# output to user
print(response)
```

Here's what happens when we run this script:

```
$ python auto_serial.py
Hello World!
```

This ability opens up a huge amount of automation opportunities. By using
scripts to interact with the shell, you can easily:

- Read the values of arbitrary variables and regions of RAM or flash for
  inspection without the use of a debugger.
- Send and receive files from a device's file system.
- Run a suite of automated tests, all of which are individual shell commands
  that return success codes.
- Perform before and after analysis on heaps, battery life, etc. when running
  automated tests.
- Manage a large number of devices connected to a single computer.

### Sharing Automation Scripts

I usually write one-off serial-port-based scripts when I'm frustrated at how
difficult it is to type a very long sequence of commands in, especially on a
firmware shell without history built-in.

Imagine something like:

```
$ wifi on
> OK
$ wifi set MyHotSpot WPA2 mysuperl0ngcomplicat3dpAssw0rd
> OK
$ wifi connect
> OK
```

Typing this in every time I change Wi-Fi hotspots will become tedious for me and
my teammates. In these cases, I suggest taking a few more minutes to productize
these scripts by adding them to a [project
CLI]({% post_url 2019-08-27-building-a-cli-for-firmware-projects %}) to easily
share them with others working on the same project.

This way, I could write a simple command that would accept the Wi-Fi hotspot
credentials, and the script would handle connecting to the serial port, running
the commands, and ensuring they were successful.

```
$ invoke wifi --ssid MyHotSpot --key mypassword
Turning on Wifi...
Setting Credentials...
Connecting to Wi-Fi...
Done!
```

## Unit Testing

No new software is complete without tests, and the same applies for our
command-line shell, even though it seems to be quite low-level. The purpose of
unit tests for a shell would be to ensure that each command never breaks for
developers or on build configurations that aren't used often, such a debug or
factory builds. Nobody wants to test 50 shell commands, so let unit tests do it!

I won't go very deep in this section, but I do want to give a rough idea of how
one would implement unit tests for a shell.

Using CppUTest and a similar structure to our previous [Unit Testing
Basics]({% post_url 2019-10-08-unit-testing-basics %}) post, we arrive at the
following structure within our `part2/` example:

```
$ tree -L 2  part2/tests
part2/tests
├── Makefile
├── MakefileWorker.mk
├── MakefileWorkerOverrides.mk
├── makefiles
│   └── Makefile_test_shell_commands.mk
└── src
    ├── AllTests.cpp
    └── test_shell_commands.cpp
```

The only source file in `Makefile_test_shell_commands.mk` that we'll include is
`shell_commands.c` since we can mock everything else out.

```makefile
COMPONENT_NAME=test_shell_commands

SRC_FILES = \
  $(PROJECT_SRC_DIR)/shell_commands.c \

TEST_SRC_FILES = \
  $(UNITTEST_SRC_DIR)/test_shell_commands.cpp

include $(CPPUTEST_MAKFILE_INFRA)
```

The other file to look at is `test_shell_commands.cpp`, which is where our tests
reside.

Let's look at what a test for `cli_cmd_hello` would look like. We create our two
arguments, `argc` and `argv`. Next, we pass those directly into the function.
Finally, we ensure that the output was as we expected.

```c++
TEST(TestShellCommands, hello) {
  int argc = 1;
  char *argv[] = {
    (char *)"hello"
  };

  cli_cmd_hello(argc, argv);
  STRNCMP_EQUAL("Hello World!", s_resp_buffer, sizeof(s_resp_buffer));
}
```

How did we capture this output? By using a basic mock in our test.

```c++
static char s_resp_buffer[1024];

// Poor man's mock
void shell_put_line(const char *str) {
  strncpy(s_resp_buffer, str, sizeof(s_resp_buffer));
}
```

`cli_cmd_hello` outputs its response through a function called `shell_put_line`,
which we convert into a mock for the purposes of testing.

We can now run our simple test and verify it works!

```
$ cd part2/tests

$ make
/usr/bin/make -f interrupt/example/firmware-shell/part2/tests/makefiles/Makefile_test_shell_commands.mk
compiling test_shell_commands.cpp
compiling AllTests.cpp
compiling shell_commands.c
Building archive build/test_shell_commands/lib/libtest_shell_commands.a
a - build/test_shell_commands/objs/interrupt/example/firmware-shell/part2/shell_commands.o
Linking build/test_shell_commands/test_shell_commands_tests
Running build/test_shell_commands/test_shell_commands_tests
.
OK (1 tests, 1 ran, 1 checks, 0 ignored, 0 filtered out, 0 ms)
```

For more reference, it's worth checking out how the
[Memfault Firmware SDK](https://github.com/memfault/memfault-firmware-sdk/blob/master/tests/src/test_memfault_demo_cli.cpp)
and
[Skip Scooter's Anchor](https://github.com/rideskip/anchor/tree/master/console/tests)
libraries test their command-line implementations.

## Tips and Considerations

### Formatted Output

There are two consumers of a device's shell: a developer and a machine hooked up
to the device through serial. Therefore, an output format should be both human
and machine-readable.

I suggest that all commands succeed or fail in the same way, either printing
`OK` or `FAIL,<code>`. It might help to build helper functions that do this for
you:

```c
void shell_result_success(void) {
  shell_put_line("> OK");
}

void shell_result_fail(int code) {
  char buf[32];
  snprint(buf, sizeof(buf), "> FAIL,%d", code);
  shell_put_line(buf);
}

```

One common format that is used is CSV, which is mostly readable depending on
what is being output. Below, we show an example of a `dump_heap` command which
would spit out a CSV, which a developer could then copy into Excel or Google
Sheets and do further inspections.

```
$ dump_heap
addr,size
0x00200000,64
0x00200040,128
...
```

If you are trying to output files or raw binary buffers into the shell, I
recommend using base64 or hexdump encoding, which will make it easier for
automated systems to ingest and convert the output.

### Extra Shell Features

Every developer loves a full-featured shell. These features might include being
able to backspace and delete keys, use the left and right arrow keys for
in-place editing, Emacs shortcuts, command history and completion, argument
parsing, and help menus.

To learn how to add some of these features to your project's shell, it's best to
look at a few open-source projects. Unfortunately, there aren't many _great_
shell's publicly available for tiny firmware.

- [Zephyr Shell](https://github.com/zephyrproject-rtos/zephyr/blob/master/subsys/shell)
- [Skip Scooters Anchor Console](https://github.com/rideskip/anchor/tree/master/console)
- [ESP-IDF Console](https://github.com/espressif/esp-idf/tree/master/components/console)

I would say the Zephyr shell is the opposite of small and lightweight, but that
is because it can be run through the RTT console, telnet, UART, and likely
arbitrary transports. It contains every shell feature you have come to love on
your UNIX shell, such as history, wildcard support, hexdumps, and support for
many keyboard shortcuts. If you are looking for _how_ to implement a certain
shell feature, I'd check out this repo.

If you are looking for something a bit more featured than the shell built
earlier, I'd give Anchor a look. It provides history, completion support, and
has an API built on top of macros for easier registering of commands and
arguments.

If you want to take inspiration from my favorite solution, check out the
ESP-IDF's
[`idf_monitor.py`](https://github.com/espressif/esp-idf/blob/master/tools/idf_monitor.py#L3-L10).
It is based upon PySerial's miniterm but adds convenient features host-side such
as automatically `addr2line` address lookups and rebuild-and-flash in one step.

There is no one-size-fits-all shell, as every project has different requirements
based upon memory, functionality, and code space.

### Stable Automated & Factory Tests

For a suite of factory verifications and automated tests to function 100% of the
time, the command arguments, behavior, and output (including formatting!) **can
not** change between builds. This is because these systems have parsers on the
output and those would break if the responses weren't in the correct format.

The simplest way to verify that the input and output do not change between
builds is to invest in a suite of unit tests with [proper
mocks]({% post_url 2020-05-12-unit-test-mocking %}) that assert that every
character being output remains the same. These should then be run within CI for
every pull request and master branch commit.

### Slimming Down Shell Commands

Every command that you add to the shell will take up a non-trivial amount of
code space. The command implementations usually contain a lot of strings and
formatting code, which doesn't optimize well. When the firmware is out of space
([don't let it]({% post_url 2020-03-18-code-size-deltas %})), these shell
features are usually some of the first to go.

You usually want to perform automated tests on a production-like build. These
builds will have fewer developer-facing bells and whistles and only contain the
bare minimum set of commands to successfully and completely run through the set
of automated tests that your team has built.

To make sure that the tests that need to be stable do remain stable, I advise
heavy usage of comments and also tests to confirm the outputs don't change.

```c
static const sShellCommand s_shell_commands[] = {

#if !PRODUCTION_BUILD
  {"help", shell_help_handler, "Lists all commands"},
  {"history", shell_cmd_shell_history, "Show last 10 shell commands"},
  {"heap_dump", shell_cmd_heap_dump, "Parses and pretty-prints the heap"},
#endif

// ==========================================================
// DON'T CHANGE THESE COMMANDS. YOU'LL BREAK AUTOMATED TESTS!
// ==========================================================
  {"get_device_info", shell_cmd_get_device_info, "Get device info"},
  {"reboot", shell_cmd_system_reboot, "Reboot system"},
  {"resets", shell_cmd_system_reset, "Factory resets system"},
};
```

```c
// ==========================================================
// DON'T CHANGE THIS COMMAND. YOU'LL BREAK AUTOMATED TESTS!
// ==========================================================
// Output Format:
// > OK
void shell_cmd_system_reset(void) {
  [...]
}
```

### Securing the Shell

Before the device leaves the factory and goes out to the end customer, you'll
likely want to secure the serial port and shell in some way. This is to ensure
bad actors do not gain access to the system. I'll quickly go over a few ways
that you can lock down the shell to limit access to approved developers or
technicians.

#### Password Verification

This is the simplest approach to locking down the serial port. If the
verification process is successful, the shell will unlock itself!

Although its security is questionable, it is a good base-line I would put in
place to restrict a chunk of hobby hackers from gaining access to the system.

Note that this would be easily thwarted by anyone running
[`strings`]({% post_url 2020-04-08-gnu-binutils %}#strings) on the firmware, as
the password will be seen in plain text. So you might want to add a basic cipher
to the password, such as the following:

```c
bool validate_password(const char *attempt) {
  // UNLOCK with each char rotated by 1 character
  // U + 1 = V, etc
  const char *password = "VOMPDL"

  for (int i = 0; i < sizeof(password) - 1; i++) {
    // Apply simple cipher, compare character
    if ((password[i] - 1) != attempt[i]) {
      return false;
    }
  }

  return true;
}
```

Still not great security, but it is better than nothing.

#### Knock Pattern

This method using timing and logic to send a pattern to the device that it will
verify. If the verification process is successful, the shell will unlock itself.

This method can be compared to
[this knocking sequence (video)](https://www.youtube.com/watch?v=1BB6wj6RyKo)
that GE Lighting built into their light bulbs to place them into "firmware
update" mode.

#### Signed Verification

Most devices today include a public or private key baked in to allow for secure
communication with remote servers. If this system is in place, it wouldn't take
much more effort to use this system to help unlock the shell.

A basic approach would be for the central server to sign the phrase "UNLOCK"
with the device's key. This payload can then be sent over the UART, and, if the
device verifies the authenticity of the payload, it will unlock the shell.

This is all the detail I'll give about this process for now, as the topic is
worthy of a blog post itself.

#### Different Firmware

The most secure way to lock down the shell is to not include the shell at all.
Simple, but it will severely limit your ability to debug a faulty production
device should you receive one from customer support.

## When Not To Build a CLI Shell

No article would be useful without the counter-arguments. I've seen a few times
where having a CLI shell over serial was **not** the tool for the job. It might
be worth the time to invest in building something else if:

- The connection to the device is not reliable. You'll need to be able to detect
  dropped bytes and attempt retries.
- The footprint and breadth of the CLI has gotten too big, and it would be
  better to have most of the strings, help menus, argument parsing, and error
  checking be contained with a host-side application.
- You need to maintain multiple connections to a single device over a single
  serial port.
- You are sure that any consumer of the interface (developers, QA, factory
  harnesses) will use the tools your team builds. In this case, I would
  definitely go for the binary protocol route.

## Final Thoughts

As an evangelist for developer productivity, I love finding repeated tasks and
automating the processes, whether that is in the firmware shell, a [project CLI
on the host
machine]({% post_url 2019-08-27-building-a-cli-for-firmware-projects %}), or in
[the
debugger]({% post_url 2019-07-02-automate-debugging-with-gdb-python-api %}).

I hope this post conveyed the reasons why having a developer-focused interface
into a firmware-based device is helpful. I'd love to hear about how you think
about command-line interfaces and also how you strike the balance between
features and code space usage in your shell.

You can find the examples shown in this post
[here](https://github.com/memfault/interrupt/tree/master/example/firmware-shell).

{% include submit-pr.html %}

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^sdk_uart]: [Nordic nRF52 SDK - UART](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.3.0%2Fgroup__app__uart.html)
[^libopencm3_uart_console]: [libopencm3 UART Console example](https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f4/stm32f429i-discovery/usart_console/usart_console.c)
[^miniterm]: [PySerial Miniterm](https://pyserial.readthedocs.io/en/latest/tools.html#module-serial.tools.miniterm)
[^gnu_toolchain]: [GNU ARM Embedded toolchain for download](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
[^jlink]: [JLinkGDBServer](https://www.segger.com/products/debug-probes/j-link/tools/j-link-gdb-server/about-j-link-gdb-server/)
[^nrf5_sdk]: [nRF52840 Development Kit](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK)
[^command_line_args]: [GNU - Program Arguments](https://www.gnu.org/software/libc/manual/html_node/Program-Arguments.html)
<!-- prettier-ignore-end -->
