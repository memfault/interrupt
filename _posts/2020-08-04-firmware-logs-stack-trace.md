---
title: Parsing Logs Messages for Instant Crash Analysis
description: A stack trace for embedded software. Get all the information you need to debug firmware crashes right into your logs, in a few easy steps.
image: /img/logs-stack-trace/colorful_stack_unbundling.gif
author: cyril
tags: [logging, stack]
toc: true
---


<!-- excerpt start -->

Having a logger display execution information on the terminal is pretty common for firmware developers. What's less common is having an instant stack trace when the program crashes.

<!-- excerpt end -->

In this post, I will introduce a few tools to implement a beautiful boosted logger. The example presented here is based on the nRF52 device and its SDK compiled with the GCC toolchain but can be ported to any other Cortex-M microcontroller.

> This post was originally published on [Cyril's website](http://www.cyrilfougeray.com/2020/07/27/firmware-logs-with-stack-trace.html).

{% include newsletter.html %}

![Colorful stack]({% img_url logs-stack-trace/colorful_stack_unbundling.gif %})

{% include toc.html %}

## 📰 The Logging Interface

Logging was historically done using the UART peripheral on the microcontroller. One could also use semihosting but it is more and more common to see Segger's RTT (Real-Time Transfer) being used as it is very efficient: it doesn't affect real-time behavior. I am still using the serial interface for some projects because using RTT requires to connect the debugger, which I don't do every time. Moreover, the RTT Client used to be a bit buggy. I hope that's not the case anymore and I should probably consider using RTT more than I do. In the last few years, I have been using RTT only to debug the bootloader on several targets. 🙄

In order to implement stack unbundling from the terminal client, we need to receive and parse the received characters. In the scope of this article, we are going to use the serial interface to make it easy to implement and focus on the important parts.

We could implement stack tracing from an RTT Client but it might be more interesting to have GDB connected if your debugger is attached anyway. For a good starting point, we have you covered[^debug_fault]

## 💥 Catching Crashes

You might have guessed already that in order to have a stack trace, we need to dump the stack when the crash occurs. To do so, we are going to modify the `HardFault_Handler` to send the stack region through our logging interface. Then we need to parse that stack helped with the symbols defined in our program and GDB.

Hopefully, we won't reinvent the wheel as Adam Green[^adamgreen] made those tools available on Github with CrashCatcher[^crashcatcher] and CrashDebug[^crashdebug]. 🙏

### 🧪 Stack Dumping

We need to start by reimplementing a "useful" `HardFault_Handler`. I said "useful" because most default handlers do nothing but rebooting or looping indefinitely.

To perform the stack dump, we are going to use CrashCatcher. If you want to read more about that tool, head on to the git repo[^crashcatcher]. Let's start.

First, we need to clone the CrashCatcher repo. Using nRF52's SDK, I find it convenient to clone it in the `external` directory.

```bash
# From the SDK root directory
$ cd external
$ git clone --recursive https://github.com/adamgreen/CrashCatcher.git
```

The nRF52 SDK provides an implementation of the HardFault handler that can be enabled through a defined macro in the config file `sdk_config.h` using `HARDFAULT_HANDLER_ENABLED`. We don't want to enable the default handler as we will reimplement our own so make sure the macro equals `0`.

Now, we need to add the source files in the Makefile. In the case you want to implement a stack trace only for the debug target, you could add target-specific source files in the Makefile. We only need 2 files from the CrashCatcher repo:

```makefile
SRC_FILES ?= \
    [...] \
    $(SDK_ROOT)/external/CrashCatcher/Core/src/CrashCatcher.c \
    $(SDK_ROOT)/external/CrashCatcher/Core/src/CrashCatcher_armv7m.S
    # ☝️ for armv7m, there are other architectures available, make sure to use
    # the good one for your project

INC_FOLDERS += \
    [...] \
    $(SDK_ROOT)/external/CrashCatcher/include \
    $(SDK_ROOT)/external/CrashCatcher/Core/src/
```

If you take a look at `CrashCatcher_armv7m.S`, you will find a definition, in Assembler, of the HardFault handler we are going to use. This one is preparing the stack to be dumped and then calls `CrashCatcher_Entry`, defined in `CrashCatcher.c`.

Let's jump directly in `CrashCatcher_Entry`. Adam did all the work of getting the stack and registers ready for us to be printed out to the serial interface.

CrashCatcher is flexible enough so that we can choose how the data is sent for post-mortem analysis by defining the functions to do so ourselves. I chose to transfer it directly using the serial interface but we could use RTT or probably any other interface. We could have chosen to store the dumped memory into Flash for a transfer later on, using Wi-Fi or Bluetooth for example. We need to keep in mind that we are in the HardFault handler though...

Last but not least, we can also define which memory regions to be dumped.

To continue, we are going to define those functions in a new file. Those are called from `CrashCatcher.c`:

- `CrashCatcher_DumpStart(const CrashCatcherInfo* pInfo)`
    - In our case, we start by initializing the UART peripheral. Characters will be sent one by one and we are going to wait for them to be sent. We don't want to use interrupts here.
    - Send a flag to warn the serial client that a dump is being sent. We are going to send `###CRASH###`.
- `const CrashCatcherMemoryRegion* CrashCatcher_GetMemoryRegions(void)`
    - Returns an array of regions to be dumped. On the nRF52 and for my specific program, I use the RAM down to `0x20002558`. You can add any region, even memory-mapped peripheral registers to debug a peripheral.

        ```c
        const CrashCatcherMemoryRegion* CrashCatcher_GetMemoryRegions(void)
        {
            static const CrashCatcherMemoryRegion regions[] = {
        #if defined(BOARD_CUSTOM)
                {0x20002558, 0x20010000, CRASH_CATCHER_BYTE}, /* RAM content */
                {0xFFFFFFFF, 0xFFFFFFFF, CRASH_CATCHER_BYTE}
        #else
            #error "Target device isn't supported."
        #endif
            };
            return regions;
        }
        ```

- `void CrashCatcher_DumpMemory(const void* pvMemory, CrashCatcherElementSizes elementSize, size_t elementCount)`
    - Print HEX data, line by line.
- `CrashCatcherReturnCodes CrashCatcher_DumpEnd(void)`
    - Send a flag to warn that the dump is fully sent: `###END###`

As an example, I made that C source file available on Github[^log_files_example]. Make sure to add yours to your Makefile before compiling your program. You don't need a header file as the signatures are taken from `CrashCatcher.h`.

Now, we have the RAM content dumped to our host using the serial interface whenever a HardFault occurs. Let's try.

In order to generate a HardFault, I am going to use a builtin function: `__builtin_trap()` made available if you compile using GCC. Once you compile your program, launch it while connecting a client like `minicom` to your serial interface.

You'll see something like those characters showing up when your program crashes:

```
<INFO> Everything is going well so far 🌸
<INFO> but we are about to 💥

###CRASH###
63430300
01000000
0000000000E100E001000000C0350020
F20000007800000000E0070068FF0020
[...]
```

We can see printed the registers and memory regions in the hexadecimal format. We need to save that stack trace and make use of it.

### 🔬 Stack Analysis

We are going to use a Python script to print the log and the crash information. The script will take the serial port such as `/dev/ttyUSB0` on Linux-based OSes and the path to the program file as arguments (`path/to/app_elf.out`).

Thanks to the `pyserial` package, it is very easy to read the log, line by line. For each line, we want to check if the message contains the starting flag: `###CRASH###`. Once we detect the flag, we are going to store the messages printed into a temporary file while we wait for the ending flag: `###END###`. If you don't have it already, install Python (>=3.5) and `pyserial` using `pip` or using [Conda]({% post_url 2020-01-07-conda-developer-environments %}).

As we want to print the stack trace directly from our client, we are going to use GDB and CrashDebug together. The full command is described in the documentation[^crashdebug]. In order to print the crash location and the local variables I will need to execute `bt full` right from GDB and do not forget to quit to get back to the Python script:

```
$ arm-none-eabi-gdb main.elf -ex "set target-charset ASCII" \
    -ex "target remote | CrashDebug --elf main.elf \
    --dump MainCrashDump.txt" -ex "bt full" -ex "quit"
```

You can add some GDB commands to print even more information like `info registers`.

Make sure to get the CrashDebug executable here[^crashdebug_exe] or compile it yourself following the instructions from the git repo[^crashdebug]. The path to CrashDebug is hardcoded in the following example so you probably want to change it.

Below is a simple Python script that does the job:


```python
from serial import Serial
import sys
import platform
import subprocess

port = sys.argv[1]
path_to_elf = sys.argv[2]
ser = Serial(port, 1000000) # Configure speed depending on your config

while 1:
    # Read line by line (waiting for '\n')
    line = ser.readline()
    if not line:
        break

    # When crash is detected
    # Crash dump is added into a temporary file
    # GDB is used to back trace the crash
    if b"###CRASH###" in line.strip():
        print("Crash detected, retrieving crash info...")
        dump_filename = "last_crash_dump.txt"
        dump_file = open(dump_filename, 'wb+')

        # Get the CrashDebug executable right into the local directory
        crashdebug_exe = "../CrashDebug/lin64/CrashDebug"
        if platform.system() == "Darwin":
            crashdebug_exe = "../CrashDebug/osx64/CrashDebug"

        cmd = "arm-none-eabi-gdb --batch --quiet " + path_to_elf + "  -ex \"set target-charset ASCII\" -ex \"target remote | " + crashdebug_exe + " --elf " + path_to_elf + " --dump " + dump_filename + "\" -ex \"set print pretty on\" -ex \"bt full\" -ex \"quit\""
        # For debugging purpose, we may want to print the command
        print(cmd)

        # We are now storing the stack dump into the file
        line = ser.readline()
        dumping = True
        while b"###END###" not in line.strip():
            dump_file.write(line)
            line = ser.readline()

        print("Crash info retrieved.\n")

        dump_file.close()

        # We can call GDB and CrashDebug using the command and print the results
        process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
        output, error = process.communicate()

        print(output.decode("utf-8"))
        print("---------\n")
        line = b""

    # Print out the line
    print(line.decode('utf-8').rstrip())
```

Let's test with our latest program to see how awesome it is. Here is an example of a program of mine. The crash is happening in `app.c`, line 1173. We can see some local variables in the current context `app_start()` such as `err_code` but also variables from the caller `main()`:

```
$ python dump.py /dev/ttyUSB0 \
    ../../../my-awesome-product/awesome-app/awesome-board/_build/app_debug.out

[...]
<INFO> Everything is going well so far 🌸
<INFO> but we are about to 💥

Crash detected, retrieving crash info...
arm-none-eabi-gdb --batch --quiet ../../../my-awesome-product/awesome-app/awesome-board/_build/app_debug.out -ex "set target-charset ASCII" -ex "target remote | ../CrashDebug/lin64/CrashDebug --elf ../../../my-awesome-product/awesome-app/awesome-board/_build/app_debug.out --dump last_crash_dump.txt" -ex "set print pretty on" -ex "bt full" -ex "quit"
Crash info retrieved.

0x00022190 in app_start () at ../../app.c:1173
1173        _app_started = true;
#0  0x00022190 in app_start () at ../../app.c:1173
        err_code = 0x0
        app_start_config = 0x0
#1  0x0002069e in main () at ../../main.c:708
        bl_rev = 0x5
        app_start_config = 0x0
        err_code = 0x0
        bt_addr = {
          addr_id_peer = 0x0,
          addr_type = 0x1,
          addr = "\332\241\255x\362\342"
        }

---------

```

Isn't it awesome? 😃 No, it's not, it's crashing. 😜

From now on, whenever you will have a HardFault, you'll now be able to debug it quite quickly. 🚀

## 🌈 Bonus: Make it Colorful

I have long been using the same Python script to print out the log messages. This one is making things clear by printing the host date, the target date (and the difference between the two), the active tasks count, the filename and line the message is being printed from, all of this using colors!

Here is a snippet of the log after a pin reset. We can see the main product information as an example. I synced the time using an Android application before the reset but we can still see the internal clock being 2 seconds behind the host clock, This is because of the pin reset which takes time for that specific application. Depending on the log level, colors like green, orange, and red are used.

<pre style="font-size:0.8em;">2020-07-26 11:08:13.040 [<font color="#268BD2">1595754491 </font><font color="#839496"><b>(-2.041s)</b></font>:<font color="#268BD2">0</font>:<font color="#2AA198">../../main.c</font>:<font color="#D33682">643</font>] ⚡ My awesome product
2020-07-26 11:08:13.042 [<font color="#268BD2">1595754491 </font><font color="#839496"><b>(-2.042s)</b></font>:<font color="#268BD2">0</font>:<font color="#2AA198">../../main.c</font>:<font color="#D33682">644</font>] ID: 0x1a9d1d52
2020-07-26 11:08:13.043 [<font color="#268BD2">1595754491 </font><font color="#839496"><b>(-2.044s)</b></font>:<font color="#268BD2">0</font>:<font color="#2AA198">../../main.c</font>:<font color="#D33682">647</font>] FW rev: 2.10.4 DEV
2020-07-26 11:08:13.044 [<font color="#268BD2">1595754491 </font><font color="#839496"><b>(-2.045s)</b></font>:<font color="#268BD2">0</font>:<font color="#2AA198">../../main.c</font>:<font color="#D33682">654</font>] BL rev: 5
2020-07-26 11:08:13.045 [<font color="#268BD2">1595754491 </font><font color="#839496"><b>(-2.046s)</b></font>:<font color="#268BD2">0</font>:<font color="#2AA198">../../main.c</font>:<font color="#D33682">656</font>] HW rev: 1
2020-07-26 11:08:13.047 [<font color="#268BD2">1595754491 </font><font color="#839496"><b>(-2.048s)</b></font>:<font color="#268BD2">0</font>:<font color="#2AA198">../../main.c</font>:<font color="#D33682">658</font>] Git branch: master commit:0x70954c0c
2020-07-26 11:08:13.049 [<font color="#268BD2">1595754491 </font><font color="#839496"><b>(-2.049s)</b></font>:<font color="#268BD2">0</font>:<font color="#2AA198">../../main.c</font>:<font color="#D33682">659</font>] WDT active: true
2020-07-26 11:08:13.072 [<font color="#268BD2">1595754491 </font><font color="#839496"><b>(-2.072s)</b></font>:<font color="#268BD2">0</font>:<font color="#2AA198">../../main.c</font>:<font color="#D33682">330</font>] <font color="#B58900">Reset: Pin reset</font>
2020-07-26 11:08:13.390 [<font color="#268BD2">1595754491 </font><font color="#839496"><b>(-2.390s)</b></font>:<font color="#268BD2">0</font>:<font color="#2AA198">../../main.c</font>:<font color="#D33682">285</font>] RAM_START: 0x20002558
2020-07-26 11:08:13.392 [<font color="#268BD2">1595754491 </font><font color="#839496"><b>(-2.392s)</b></font>:<font color="#268BD2">0</font>:<font color="#2AA198">../../main.c</font>:<font color="#D33682">702</font>] BT address: e2:f2:78:ad:a1:da
2020-07-26 11:08:13.400 [<font color="#268BD2">1595754491 </font><font color="#839496"><b>(-2.400s)</b></font>:<font color="#268BD2">4</font>:<font color="#2AA198">../../device_settings.c</font>:<font color="#D33682">523</font>] Writing device setting (#18, 617)
2020-07-26 11:08:13.800 [<font color="#268BD2">1595754491 </font><font color="#839496"><b>(-2.800s)</b></font>:<font color="#268BD2">3</font>:<font color="#2AA198">../../device_settings.c</font>:<font color="#D33682">365</font>] <font color="#859900">Device setting #18 written</font>
[...]
</pre>

The Python script is not the only piece needed to print such a beautiful log obviously. On the target side, I have to send the information to print. Macros are used to make it easier to print *upgraded* messages. You will find those two files in my Github repo[^log_files_example] if you want to use them.

## 👋 Closing

Implementing this trick will probably take you dozens of minutes but can make you gain way more if done well. It will be a pleasure to have instant analysis when you usually needed to replicate the bug.

Such stack tracing has long been implemented in higher-level programming languages but tools for Firmware development have always been lagging compared to most of the software industry. This post provides yet another attempt to bridge the gap by building our tools.

I look forward to your comments and tips in the comments.


## 🔗 References

<!-- prettier-ignore-start -->
[^debug_fault]: [Debug faults on Cortex-M](https://interrupt.memfault.com/blog/cortex-m-hardfault-debug)
[^adamgreen]: [Adam Green's Github page](https://github.com/adamgreen)
[^crashcatcher]: [CrashCatcher repository](https://github.com/adamgreen/CrashCatcher)
[^crashdebug]: [CrashDebug repository](https://github.com/adamgreen/CrashDebug)
[^log_files_example]: [Source files example for dumping the stack on the target](https://github.com/fouge/nrf_utils/blob/master/debug/log)
[^crashdebug_exe]: [CrashDebug executables](https://github.com/adamgreen/CrashDebug/tree/master/bins)
<!-- prettier-ignore-end -->
