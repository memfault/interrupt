---
title: "Get the most out of the linker map file"
author: cyril
---

<!-- excerpt start -->

In this article, I wanted to highlight how simple map files are and how much they can teach you about the program you are working on. 

<!-- excerpt end -->

I noticed recently that the *map* file generated during the build process and before the creation of the ELF file, is not used enough by some of us, Firmware developers 🤔.

The map file provides valuable information that helps understand and optimize memory. I highly recommend keeping that file for any firmware running in production. This is one of the few artifacts I keep in [my CD pipeline](https://medium.com/equisense/firmware-quality-assurance-continuous-delivery-125884194ea5).
The map file is a symbol table for the whole program. Let's dive into it to see how simple it is and how you can effectively use it. I will try to illustrate with some examples, all described with GNU binutils.

## Generating the map file

To get started, make sure you generate the map file.

Using GNU binutils, the generation of the map file must be explicitly requested by setting the right flag. To print the map to *output.map* with GCC:

```Makefile
    -Wl,-Map=output.map
```

Most compilers have an option to enable the same kind of file, `--map` for ARM compiler for example.

## Blinky

What's better than our good old friend to explain the basics of the map file?

In order to learn about map files, let's compile a simple LED-blink program, and modify it to add a call to `atoi`. We will then use the map file to dissect the difference between both programs. The main file, available [here](../example/linker-map-post/main.c) for you to play with it, can be used as a replacement for the Blinky example from the nRF5 SDK.

Let's build the simplest version of that project:
```c

#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    /* Configure board. */
    bsp_board_init(BSP_INIT_LEDS);

    /* Toggle LEDs. */
    while (true)
    {
        for (int i = 0; i < LEDS_NUMBER; i++)
        {
            bsp_board_led_invert(i);
            nrf_delay_ms(300);
        }
    }
}
```

Compiling:
```
    $ make
    Assembling file: gcc_startup_nrf52840.S
    Compiling file: boards.c
    Compiling file: main.c
    Compiling file: system_nrf52840.c
    Linking target: _build/nrf52840_xxaa.out
       text	   data	    bss	    dec	    hex	filename
       1704	    108	     28	   1840	    730	_build/nrf52840_xxaa.out
    Preparing: _build/nrf52840_xxaa.hex
    Preparing: _build/nrf52840_xxaa.bin
    DONE nrf52840_xxaa
```

The generated map file is 563-line long 😮, even if it does nothing more than blink an LED. That many lines cannot be left unseen, there must be some serious information in there...

Let's now modify our program to add a call to atoi. Instead of using an integer directly for the delay, we'll encode it as a string and decode it with atoi.

```c

#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"
#include "stdlib.h"

static const char* _delay_ms_str = "300";

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    /* Configure board. */
    bsp_board_init(BSP_INIT_LEDS);
    int delay = atoi(_delay_ms_str);

    /* Toggle LEDs. */
    while (true)
    {
        for (int i = 0; i < LEDS_NUMBER; i++)
        {
            bsp_board_led_invert(i);
            nrf_delay_ms(delay);
        }
    }
}
```

After compilation, the whole program goes from 1840 bytes to 2396 bytes. 

```
       text	   data	    bss	    dec	    hex	filename
       2256	    112	     28	   2396	    95c	_build/nrf52840_xxaa.out
```

We expected more code to come with calling atoi, but a 30% increase in our program size is huge!

Now that I have two map files, I want to know the differences between the two.

## Digging into the map files

### Archives linked

What? An archive in my binary? I'm only blinking an LED!

Don't be afraid, the good news is that the map file does mention **the specific symbols which caused the archive members to be brought in**, in the first lines of the file.

Here is my first lines:

```
    Archive member included to satisfy reference by file (symbol)
    
    /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc_nano.a(lib_a-exit.o)
                                  /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/crt0.o (exit)
```

The format is:

```
    The archive file location (compilation unit)
    			The compilation unit referencing the archive (symbol called)
```

It means that a `crt0` file is calling the `exit` function from `exit.o` included in `libc_nano.a`. 

The reason is not in the scope of this article, but there are indeed standard libraries that are provided by your toolchain (here the GNU toolchain). Those are available to provide standard functions such as `atoi`. In that example, I specified to the linker to use the `nano.specs` file. That's why standard functions all come from `libc_nano.a`. You can read more about this on [lb9mg.no](https://lb9mg.no/2018/08/14/reducing-firmware-size-by-removing-libc/).

Now, comparing the two generated map files, the first difference spotted is that some other archive members are included in the program: `atoi`, which itself needs `_strtol_r` which itself needs `_ctype_`:

```
    /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc_nano.a(lib_a-atoi.o)
                                  _build/nrf52840_xxaa/main.c.o (atoi)
    /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc_nano.a(lib_a-strtol.o)
                                  /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc_nano.a(lib_a-atoi.o) (_strtol_r)
    /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc_nano.a(lib_a-ctype_.o)
                                  /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc_nano.a(lib_a-strtol.o) (_ctype_)
```

We now have a better sense of the files actually included into our program, and the reason why they are there. What's more inside that file?

### Memory map

The most straightforward pieces of information in the map file are the actual memory regions:

```
    Memory Configuration
    
    Name             Origin             Length             Attributes
    FLASH            0x0000000000001000 0x00000000000ff000 xr
    RAM              0x0000000020000008 0x000000000003fff8 xrw
    *default*        0x0000000000000000 0xffffffffffffffff
```

Following that table is the `Linker script and memory map` which directly indicates the `text` area size and its content (`text` is our compiled code, as opposed to `data` which is program data). As always, the interrupt vectors (here under section `.isr_vector`) are present at the beginning of the executable, defined in `gcc_startup_nrf52840.S`:

```
    Linker script and memory map
    
    .text           0x0000000000001000      0x8c8
     ***(.isr_vector)**
     .isr_vector    0x0000000000001000      0x200 _build/nrf52840_xxaa/**gcc_startup_nrf52840.S.o**
                    0x0000000000001000                __isr_vector
     *(.text*)
     .text          0x0000000000001200       0x40 /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/thumb/v7e-m+fp/hard/crtbegin.o
     .text          0x0000000000001240       0x74 /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/crt0.o
                    0x0000000000001240                _mainCRTStartup
                    0x0000000000001240                _start
     .text          0x00000000000012b4       0x3c _build/nrf52840_xxaa/gcc_startup_nrf52840.S.o
                    0x00000000000012b4                Reset_Handler
                    0x00000000000012dc                NMI_Handler
    
    [...]
    
     .text.**bsp_board_led_invert**
                    0x00000000000012f0       **0x34** _build/nrf52840_xxaa/**boards.c.o**
                    0x00000000000012f0                bsp_board_led_invert
```

Those lines give us the address of each function and its size. Above, you can read the address of `bsp_board_led_invert`, coming from `boards.c.o` (compilation unit of `board.c` as you guessed) which as a size of 0x34 bytes in the `text` area. That way, we are able to locate each function used in the program.

My constant string `_delay_ms_str` is obviously included in the program as it is initialized. Read-only data are saved as `rodata` and kept in the `FLASH` region as per my linker script (stored in Flash and not copied in RAM as it is constant). I can find it under that line:

```
    .rodata.main.str1.4
    	              0x00000000000017b8        0x4 _build/nrf52840_xxaa/main.c.o
```

I also noticed that the inclusion of `_ctype_` added 0x101 bytes of read-only data in the `text` area 😯.

```
    .rodata._ctype_
                    0x00000000000017c0      **0x101** /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc_nano.a(lib_a-ctype_.o)
                    0x00000000000017c0                **_ctype_**
```

Interestingly, the addition of `atoi` not only increased our code size (the `text` area), but also our data size (the `data` area) I used a diff tool to make things simpler and discovered data that was before discarded by the linker:

```
    .data._impure_ptr
    								0x0000000000000000        0x4 /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc_nano.a(lib_a-impure.o)
```

One last piece of information to keep in mind: initialized variables have to be kept in Flash but they appear in `RAM` in the map file because they are copied into RAM before entering the `main` function. Here, symbols `__data_start__` and `__data_end__` keep track of the area used in RAM to keep initialized variables. Those values are stored in Flash starting at `0x00000000000018d0`:

```
    .data           0x0000000020000008       0x70 load address 0x00000000000018d0
                    0x0000000020000008                __data_start__ = .
    [...]
                    0x0000000020000078                __data_end__ = .
```

### Discarded sections

Note that symbols that are compiled but not needed are also listed in the map file in the `Discarded input sections` part.

Several usages of the map file are possible. Most of the time, you will have an address and you will want to know which function or data is being used. But some other times, it will be useful for debugging...

## Debugging a linking error

Map files are created while the built code (`.o` files) is linked together which means it can be used to resolve errors in the linking process. I remember working on a bootloader that was contained in a few Flash pages. At some point, I wanted to use `atoi` but the bootloader didn't compile anymore because there was no more space available.

Using the previous example, let's say I now have only 0x800 bytes of Flash. Compiling the first example, without `atoi`, there are no issues. The second example however would overflow our small Flash:

```
    $ make
    Compiling file: main.c
    Linking target: _build/nrf52840_xxaa.out
    /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/bin/ld: _build/nrf52840_xxaa.out section `.text' will not fit in region `FLASH'
    /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/bin/ld: region FLASH overflowed with .data and user data
    /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/bin/ld: region `FLASH' overflowed by 208 bytes
    collect2: error: ld returned 1 exit status
    make: *** [_build/nrf52840_xxaa.out] Error 1
```

That's annoying! `atoi` is such a simple function... But as we have seen, it takes more Flash than we would expect using `libc_nano.a`.

Let's try to to implement our own version of `atoi`, it's not that difficult after all. Here is the result after compilation:

```
    Linking target: _build/nrf52840_xxaa.out
       text	   data	    bss	    dec	    hex	filename
       1740	    108	     28	   1876	    754	_build/nrf52840_xxaa.out
```

Way better! The code can now be crammed into 0x800 bytes to meet our (fake) requirements 🥳

Reading the map file will teach you a lot about the code you are working on, and it is the first step to better Firmware.

Feel free to share your story with any interesting usage of the map files.

## Sources

[https://www.embeddedrelated.com/showarticle/900.php](https://www.embeddedrelated.com/showarticle/900.php)

[https://stackoverflow.com/questions/755783/whats-the-use-of-map-files-the-linker-produces](https://stackoverflow.com/questions/755783/whats-the-use-of-map-files-the-linker-produces)

[https://sourceware.org/binutils/docs/ld/Options.html#index-_002d_002dprint_002dmap](https://sourceware.org/binutils/docs/ld/Options.html#index-_002d_002dprint_002dmap)