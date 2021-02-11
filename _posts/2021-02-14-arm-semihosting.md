---
title: "Introduction to ARM Semihosting"
description: "An introduction to ARM semihosting, folowed by and example with OpenOCD and arm-none-eabi toolchain"
tag: [cortex-m, semihosting, ARM]
author: ael-mess
---

Firmware developers are generally accustomed to using logger for some execution information. On microcontrollers, this is usually done through a serial interface attached to a terminal on the host.

But one might need more advanced features from the host 🤨, I was in that case not long ago.

When working on camera firmware, and particularly on the image processing algorithm, I needed constantly to store the captured image on the host to make sure everything was working fine.

So I started using a very interesting technique that has greatly facilitated not only the development but also the debugging of my embedded software.

This capability is known as **semihosting**, and it is one of the many interesting features available on ARM Cortex microcontrollers, it allows an embedded program to take advantage of the capabilities of an attached computer when the debugger is running.

<!-- excerpt start -->
This post introduces semihosting, and shows how to use it and integrate it in embedded projects.
<!-- excerpt end -->

{% include toc.html %}

## What is semihosting

According to ARM documentation[^1], **semihosting** is : .. a mechanism that enables code running on an ARM target to communicate and use the Input/Output facilities on a host computer that is running a debugger ..

In other words, an ARM based MCU can run C library functions such as, `int printf(const char *format, ...)`, `int scanf(const char *format, ...)`, or even `FILE *fopen(const char *filename, const char *mode)` on the host computer attached to it.

And by doing so, it can benefinit from the screen, the keybord, or the disk of the host.

![]({% img_url semihosting/semihosting_overview.svg %})

## How it works

This is done by halting the CPU target by the debuger agent, either by running into a breakpoint instruction (`BKPT 0xAB`  for ARMv6-M or ARMv7-M), or by sending a supervisor call instruction (`SVC 0xAB` or `SVC 0x123456`) depending on the target architecture or processor.

This indicates to the host that the target is requesting a semihosting operation.

The debuger agent then figures out the operation requested by the target, by reading the content of `r0`, and if necessary, accesses all other function parameters pointed by `r1`.

At this point the CPU is still halted, the host will execute the requested operation and return the result in `r0` before allowing the processor to continue running its program.

The following is a list of the semihosting operations defined by ARM[^2]:

```c
/* File operations */
SYS_OPEN        EQU 0x01 //Open a file or stream on the host system.
SYS_ISTTY       EQU 0x09 //Check whether a file handle is associated with a file or a stream/terminal such as stdout.
SYS_WRITE       EQU 0x05 //Write to a file or stream.
SYS_READ        EQU 0x06 //Read from a file at the current cursor position.
SYS_CLOSE       EQU 0x02 //Closes a file on the host which has been opened by SYS_OPEN.
SYS_FLEN        EQU 0x0C //Get the length of a file.
SYS_SEEK        EQU 0x0A //Set the file cursor to a given position in a file.
SYS_TMPNAM      EQU 0x0D //Get a temporary absolute file path to create a temporary file.
SYS_REMOVE      EQU 0x0E //Remove a file on the host system. Possibly insecure!
SYS_RENAME      EQU 0x0F //Rename a file on the host system. Possibly insecure!

/* Terminal I/O operations */
SYS_WRITEC      EQU 0x03 //Write one character to the debug terminal.
SYS_WRITE0      EQU 0x04 //Write a 0-terminated string to the debug terminal.
SYS_READC       EQU 0x07 //Read one character from the debug terminal.

/* Time operations */
SYS_CLOCK       EQU 0x10
SYS_ELAPSED     EQU 0x30
SYS_TICKFREQ    EQU 0x31
SYS_TIME        EQU 0x11

/* System/Misc. operations */
SYS_ERRNO       EQU 0x13 //Returns the value of the C library errno variable that is associated with the semihosting implementation.
SYS_GET_CMDLINE EQU 0x15 //Get commandline parameters for the application to run with (argc and argv for main())
SYS_HEAPINFO    EQU 0x16
SYS_ISERROR     EQU 0x08
SYS_SYSTEM      EQU 0x12
```

## How to use it

On this example, I run the program on an STM32, and use the `arm-none-eabi` toolchain allong with **openOCD** `gdbserver`. Other tools may be used instead depending on your hardware, for more details on which debug interface to use, It would be wise to read [this article](https://interrupt.memfault.com/blog/a-deep-dive-into-arm-cortex-m-debug-interfaces).

Once you are all set, we can start coding.

First, to enable semihosting you need to link:
* `libc.a`, the standard C library (*newlib*[^3] for `arm-none-eabi-gcc`),
* and `librdimon.a`, a part of the *libgloss*[^4] library (for platform-specific code) to your project.

This is done by adding a few options to the ARM linker:
* removing `-nostartfiles` flag to allow linking standards libraries,
* adding `-lc -lrdimon` flags,
* and changing the *spec strings* file[^5] to `--specs=rdimon.specs` to use the semihosted version of the *syscalls*.

The last point essentially means that the system calls can be used when a debugger is attached (note that the CPU may crash if a debugger is not present).

Now on the C code side, you need to call `initialise_monitor_handles()` before starting a semihosting operation.

```c
#ifdef SEMIHOSTING
extern void initialise_monitor_handles(void);
#endif

int main(void) {
#ifdef SEMIHOSTING
  	initialise_monitor_handles();
#endif

  	// other tasks ...

#ifdef SEMIHOSTING
    printf("hello world!\n");
    printf("hello world!\n");
#endif

    // other tasks ...

  	return 0;
}
```

If you are using `crt0` initialization function then, `initialise_monitor_handles()` has already been called for you before `main()`.

Once you have done this, you can compile and program your microcontroller as usual, however, once the microcontroller flashed it should be halted and not run yet.

In my case, I launch *OpenOCD* `gdbserver` in one terminal, to flash, reset and halt the CPU. And run `gdb` from another terminal to connect to the server, and enable the semihosting in the server side[^6].

```shell
arm-none-eabi-gdb -ex "target extended-remote localhost:3333" -ex "monitor reset halt" -ex "monitor arm semihosting enable"
```

If `initialise_monitor_handles` is called before enabling the semihosting in the server, a `HardFault` may occur due to an unexpected debug event.

Once semihosting enabled, you can run your program from `gdb`, and the semihosting operation will be handled by the `gdbserver`.

Isn't it great? 😋

## Conclusion

I hope it was fun reading this post, semihosting is not commonly used in embedded development because it slows down the execution speed, but nevertheless, it can be very handy in some cases.

I look forward to your remarks, tips and comments.

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## Useful links

<!-- prettier-ignore-start -->
[^1]: [ARM semihosting documentation](https://developer.arm.com/documentation/dui0471/i/semihosting/what-is-semihosting-?lang=en)
[^2]: [ARM semihosting operations](https://developer.arm.com/documentation/dui0471/i/semihosting/semihosting-operations?lang=en)
[^3]: [Newlib](https://www.embedded.com/embedding-with-gnu-newlib/)
[^4]: [Libgloss gcc](https://sca.uwaterloo.ca/coldfire/gcc-doc/docs/porting_1.html)
[^5]: [GCC Spec Files](https://gcc.gnu.org/onlinedocs/gcc/Spec-Files.html)
[^6]: [OpenOCD semihosting commands](http://openocd.org/doc/html/Architecture-and-Core-Commands.html#Architecture-and-Core-Commands)
<!-- prettier-ignore-end -->
