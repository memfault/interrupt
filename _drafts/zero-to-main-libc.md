---
title: "From zero to main(): Bootstrapping libc"
author: francois
tags: [zero-to-main]
---

## Introduction

In this series of post, we’ve worked methodically to demystify what happens to firmware before the main() function is called. So far, we bootstrapped a C environment, wrote a linker script from scratch, and implemented our own bootloader.

And yet, we cannot even write a hello world program! Consider the following `main.c` file:

```c
#include <stdio.h>

int main() {
  printf("Hello, World\n");
  while (1) {}
}
```

Compiling this using our makefile and linker script from previous posts, we hit the following error:

```shell
$ make
...
Linking build/minimal.elf
/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/bin/ld: build/objs/a/b/c/minimal.o: in function `main':
/minimal/minimal.c:4: undefined reference to `printf'
collect2: error: ld returned 1 exit status
make: *** [build/minimal.elf] Error 1
```

Undefined reference to `printf`! How could this be? Our firmware’s C environment is still missing a key component: a working C standard library.  This means that commonly used functions such as `printf`, `memcpy`, or `strncpy` are all out of reach of our program so far.

In firmware-land, nothing comes free with the system: just like we had to explicitly zero out the `bss` region to initialize some of our static variables, we’ll have to port a `printf` implementation alongside a C standard library if we want to use them.

In this post, we will add RedHat’s Newlib to our firmware and highlight some of its features. We will implement syscalls, learn about constructors, and finally print out “Hello, World”!

## Setup
As we did in our previous posts, we are using Adafruit’s Metro M0 development board to run our examples. We use a cheap CMSIS-DAP adapter and openOCD to program it.

- [ ] You can find a step by step guide in our previous post: [Programming the ATSAMD21 with IBDAP | Interrupt](https://interrupt.memfault.com/blog/getting-started-with-ibdap-and-atsamd21g18).

As with previous examples, we start with our “minimal” example which you can find [on GitHub](https://github.com/memfault/zero-to-main/tree/master/minimal). I’ve reproduce the source code for `main.c` below:

```c
#include <samd21g18a.h>

#include <port.h>
#include <string.h>

#define LED_0_PIN PIN_PA17

static void set_output(const uint8_t pin) {
  struct port_config config_port_pin;
  port_get_config_defaults(&config_port_pin);
  config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
  port_pin_set_config(pin, &config_port_pin);
  port_pin_set_output_level(pin, false);
}

int main() {
  memcpy(NULL, NULL, 0);
  set_output(LED_0_PIN);
  while (true) {
    port_pin_toggle_output_level(LED_0_PIN);
    for (int i = 0; i < 100000; ++i) {}
  }
}
``` 

## Implementing Newlib
### Why Newlib?

There are several implementations of the C Standard Library, starting with the venerable `glibc` found on most GNU/Linux systems. Alternative implementations include Musl libc, Bionic libc, ucLibc, and dietlibc. 

Newlib is an implementation of the C Standard Library targeted at bare-metal embedded systems that is maintained by RedHat.

Newlib has become the de-facto standard in embedded software because it is complete, has optimizations for a wide range of architectures, and produces relatively small code.

Today it is bundled alongside toolchains and SDK provided by vendors such as ARM (`arm-none-eabi-gcc`) and Espressif (ESP-IDF for ESP32).

> newlib vs. newlib-nano: when code-size constrained, you may chooses to use a variant of newlib, called newlib-nano, which does away with some C99 features, and some `printf` bells and whistles to deliver a more compact standard library. Newlib-nano is enabled with the `—specs=nano.specs` CFLAG. You can read more about it in our [code size blog post](https://interrupt.memfault.com/blog/code-size-optimization-gcc-flags#c-library)  

### Enabling Newlib

Newlib is enabled **by default** when you build a project with `arm-none-eabi-gcc`. Indeed, you must explicitly opt-out with `-nostdlib` if you prefer to build your firmware without it.

This is what we do for our “minimal” example, to guarantee we do not include any libc functionality by mistake.

```makefile
PROJECT := minimal
BUILD_DIR ?= build

CFLAGS += -nostdlib

SRCS = \
	startup_samd21.c \
	$(PROJECT).c

include ../common-standalone.mk
```

It is very easy to add a dependency on the C standard library without meaning to, as GCC will sometimes use standard C functions implicitly. For example, consider this code used to zero-initialize a struct:

```c
int main() {
  int b[3] = {0}; // zero initialize a struct
  /* ... */
}
```

We added no new `#include`, nor any call to C library functions. Yet if we compile this code with `-nostdlib`,  we’ll get the following error:
```shell
...
Linking build/minimal.elf
/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/bin/ld: build/objs/a/b/c/minimal.o: in function `main':
/minimal/minimal.c:16: undefined reference to `memset'
collect2: error: ld returned 1 exit status
make: *** [build/minimal.elf] Error 1
```

If we remove `-nostdlib`, the program compiles and link without problems.

```
Linking build/minimal.elf
arm-none-eabi-objdump -D build/minimal.elf > build/minimal.lst
arm-none-eabi-objcopy build/minimal.elf build/minimal.bin -O binary
arm-none-eabi-size build/minimal.elf
   text    data     bss     dec     hex filename
   1292       0    8192    9484    250c build/minimal.elf
```

So here we are, using Newlib, and we did not have to do anything. Could it really be this simple?

### System Calls

Not so fast! Let’s go back to our “Hello World” example:

```c
int main() {
  printf("Hello World!\n");
  while(1) {}
}
```

Removing `-nostdlib` is not quite enough. Instead of `printf` being undefined, we now see a whole mess of undefined symbols:

```shell
Linking build/minimal.elf
/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/bin/ld: /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v6-m/nofp/libg_nano.
a(lib_a-sbrkr.o): in function `_sbrk_r':
sbrkr.c:(.text._sbrk_r+0xc): undefined reference to `_sbrk'
/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/bin/ld: /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v6-m/nofp/libg_nano.
a(lib_a-writer.o): in function `_write_r':
writer.c:(.text._write_r+0x10): undefined reference to `_write'
/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/bin/ld: /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v6-m/nofp/libg_nano.
a(lib_a-closer.o): in function `_close_r':
closer.c:(.text._close_r+0xc): undefined reference to `_close'
/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/bin/ld: /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v6-m/nofp/libg_nano.
a(lib_a-lseekr.o): in function `_lseek_r':
lseekr.c:(.text._lseek_r+0x10): undefined reference to `_lseek'
/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/bin/ld: /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v6-m/nofp/libg_nano.
a(lib_a-readr.o): in function `_read_r':
readr.c:(.text._read_r+0x10): undefined reference to `_read'
/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/bin/ld: /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v6-m/nofp/libg_nano.
a(lib_a-fstatr.o): in function `_fstat_r':
fstatr.c:(.text._fstat_r+0xe): undefined reference to `_fstat'
/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/bin/ld: /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v6-m/nofp/libg_nano.
a(lib_a-isattyr.o): in function `_isatty_r':
isattyr.c:(.text._isatty_r+0xc): undefined reference to `_isatty'
collect2: error: ld returned 1 exit status
```

Specifically, the compiler is asking for `_fstat`, `_read`, `_lseek`, `_close`, `_write`, and `_sbrk`.

The [newlib documentation](https://sourceware.org/newlib/libc.html#Syscalls) calls these functions “system calls”. In short, they are the handful of things newlib expects the underlying “operating system”. The complete list of them is provided below:

```
_exit, close, environ, execve, fork, fstat, getpid, isatty, kill, link, lseek, open, read, sbrk, stat, times, unlink, wait, write
``` 

You’ll notice that several of the syscalls relate to filesystem operation or process control. These do not make much sense in a firmware context, so we’ll often simply provide a stub that returns an error code.

Let’s look at the ones our “Hello, World” example requires. The man pages for them contains all the information we need.

#### fstat
`fstat`  returns the status of an open file.  The minimal version of this should identify all files as character special devices. This forces one-byte-read at a time.
```c
#include <sys/stat.h>
int fstat(int file, struct stat *st) {
  st->st_mode = S_IFCHR;
  return 0;
}
```

#### lseek

`lseek` repositions the file offset of the open file associated with the file descriptor `fd` to the argument `offset` according to the directive `whence`. 

Here we can simply return 0, which implies the file is empty.

```c
int lseek(int file, int offset, int whence) {
  return 0;
}
```

#### close

`close` closes a file descriptor `fd`.

Since no file should have gotten `open`-ed, we can just return an error on close:

```c
int close(int fd) {
  return -1;
}
```

#### write

This is where things get interesting! `write` writes up to `count` bytes from the buffer starting at `buf` to the file referred to by the file descriptor `fd`.

Functions like `printf` rely on `write` to write bytes to `STDOUT`. In our case, we will want those bytes to be written to serial instead.

On the SAMD21 chip we are using, writing bytes to serial is done using the `usart_serial_putchar` function. We can use it to implement `write`:

```c
static struct usart_module stdio_uart_module;

int _write (int fd, char *buf, int count) {
  int written = 0;

  for (; count != 0; --count) {
    if (usart_serial_putchar(&stdio_uart_module, (uint8_t)*buf++)) {
      return -1;
    }
    ++written;
  }
  return written;
}
```

We’ll also need to initialize the USART peripheral prior to calling printf:

```c
static void serial_init(void) {
  struct usart_config usart_conf;

  usart_get_config_defaults(&usart_conf);
  usart_conf.mux_setting = USART_RX_3_TX_2_XCK_3;
  usart_conf.pinmux_pad0 = PINMUX_UNUSED;
  usart_conf.pinmux_pad1 = PINMUX_UNUSED;
  usart_conf.pinmux_pad2 = PINMUX_PB22D_SERCOM5_PAD2;
  usart_conf.pinmux_pad3 = PINMUX_PB23D_SERCOM5_PAD3;

  usart_serial_init(&stdio_uart_module, SERCOM5, &usart_conf);
  usart_enable(&stdio_uart_module);
}

int main() {
  serial_init();

  printf("Hello, World!\n");
  while (1) {}
}
```

#### read

`read`  attempts to read up to `count` bytes from file descriptor `fd` into the buffer at `buf`. 

Similarly to `write`, we want `read` to read bytes from serial:

```c
int _read (int fd, char *buf, int count) {
  int read = 0;

  for (; count > 0; --count) {
    usart_serial_getchar(&stdio_uart_module, (uint8_t *)buf++);
    read++;
  }

  return read;
}
```

#### sbrk

`sbrk` increases the program’s data space by `increment` bytes. In other words, it increases the size of the heap.

What does `printf` have to do with the heap, you will justly ask? It turns out that newlib’s `printf` implementations allocates data on the heap and depends on a working `malloc` implementation. 

The source for `printf` is hard to follow, but you will find that indeed [it calls malloc](https://github.com/bminor/newlib/blob/6497fdfaf41d47e835fdefc78ecb0a934875d7cf/newlib/libc/stdio/vfprintf.c#L226)!

Here’s a simple implementation of `sbrk`:

```
caddr_t _sbrk(int incr) {
  static unsigned char *heap = HEAP_START;
  unsigned char *prev_heap = heap;
  heap += incr;
  return (caddr_t) prev_heap;
}
```

More often than not, we want the heap to use all the RAM not used by anything else. We therefore set `HEAP_START` to the first address not spoken for in our linker script. In our [previous post](https://interrupt.memfault.com/blog/how-to-write-linker-scripts-for-firmware#complete-linker-script), we had added the `_end` variable in our linker script to that end.

We replace `HEAP_START` with `_end` and get:

```
caddr_t _sbrk(int incr) {
  static unsigned char *heap = NULL;
  unsigned char *prev_heap;

  if (heap == NULL) {
    heap = (unsigned char *)&_end;
  }
  prev_heap = heap;

  heap += incr;

  return (caddr_t) prev_heap;
}
```

### Initializing State with Constructors & Destructors

<talk about using __libc_init_array to init uart>


New Reset_Handler:
```c 
void Reset_Handler(void)
{
        /* ... */
        /* Run constructors / initializers */
        __libc_init_array(); /* <--- This is new */

        /* Branch to main function */
        main();

        /* Infinite loop */
        while (1);
}

```

UART init function now has attribute

```
static void __attribute__((constructor)) serial_init(void) {
  struct usart_config usart_conf;

  usart_get_config_defaults(&usart_conf);
  usart_conf.mux_setting = USART_RX_3_TX_2_XCK_3;
  usart_conf.pinmux_pad0 = PINMUX_UNUSED;
  usart_conf.pinmux_pad1 = PINMUX_UNUSED;
  usart_conf.pinmux_pad2 = PINMUX_PB22D_SERCOM5_PAD2;
  usart_conf.pinmux_pad3 = PINMUX_PB23D_SERCOM5_PAD3;

  usart_serial_init(&stdio_uart_module, SERCOM5, &usart_conf);
  usart_enable(&stdio_uart_module);
}
```


## Implementing our own C standard library

### Replacing a function

<Talk about how all default libc symbols are weak for all intents and purposes>

### Full replacement

<Talk about using `-nostdlib` and linking your own. Perhaps use a 3rd party lib as example>

Reference: 
* [Howto: Porting newlib](https://www.embecosm.com/appnotes/ean9/ean9-howto-newlib-1.0.html)
* [The Red Hat newlib C Library](https://sourceware.org/newlib/libc.html#Stubs)