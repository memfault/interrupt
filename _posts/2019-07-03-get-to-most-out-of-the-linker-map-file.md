---
title: "Get the most out of the linker map file"
author: cyril
---

<!-- excerpt start -->

In this article, I wanted to highlight map files simplicity and how much it can teach you about the program you are working on. 

<!-- excerpt end -->

It recently appears to me that the *map* file generated during the linking process and before the creation of the ELF file, is not used enough by us, Firmware developers 🤔.

The map file provides valuable information that helps understand and optimize memory. I highly recommend keeping that file for any firmware running in production. This is one of the few artifacts I keep in [my CD pipeline](https://medium.com/equisense/firmware-quality-assurance-continuous-delivery-125884194ea5).
Let's dive into the *simplicity* of that file, seen as a symbol table for the whole program, and how you can effectively use it. I will try to illustrate with some examples, all described with GNU binutils.

## Generating the map file

To get things started, make sure you generate the map file.

Using GNU binutils, the generation of the map file must be explicitly set providing the right flag. To print the map to *output.map* with GCC:

```Makefile
    -Wl,-Map=output.map
```

Other compilers do have an option to enable the same kind of file, `--map` for ARM compiler for example.

## Blinky

What's better than our good old friend to explain the basics of the map file?

In order to explain some of the map file concepts, I compiled a program called Blinky, which I'm sure you already know. I then added a call to the famous `atoi` function and dissected the difference between both programs using the two generated map files.

The Makefile compiles 4 files. The first two files are explained in [the article from François](https://interrupt.memfault.com/blog/zero-to-main-1). The other two are making the LEDs blink:

```
    gcc_startup_nrf52840.S    
    system_nrf52840.c
    boards.c
    main.c
```

Let's build the simplest version of that project:

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

The generated map file is 563-line long 😮, even if it does pretty much nothing but blinking an LED. That many lines can't be let unseen, it must have an actual meaning...

In my program, a delay is used to toggle the LED. In order to use `atoi`, I will use a string stored directly into the main module as a static variable and convert that string to an integer to set the delay.

```c
    static const char* _delay_ms_str = "300";
```

After compilation, the whole program went from 1840 bytes to 2396 bytes. 

```
       text	   data	    bss	    dec	    hex	filename
       2256	    112	     28	   2396	    95c	_build/nrf52840_xxaa.out
```

Obviously more code has to be included in the final executable as we now call `atoi`, but that looks like a lot to me!

Now that I have two map files, I want to know the differences between the two.

### Archives included in the binary

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

The reason is not in the scope of this article, but there are indeed standard libraries that are provided by your toolchain (here the GNU toolchain). Those are available to provide standard functions such as `atoi`. In that example, I specified to the linker to use the `nano.specs` file. That's why standard functions all come from `libc_nano.a`. You can [check out that article to learn more](https://lb9mg.no/2018/08/14/reducing-firmware-size-by-removing-libc/).

Now, comparing the two generated map files, the first difference spotted is that some other archive members are included in the program: `atoi`, which itself needs `_strtol_r` which itself needs `_ctype_`:

```
    /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc_nano.a(lib_a-atoi.o)
                                  _build/nrf52840_xxaa/main.c.o (atoi)
    /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc_nano.a(lib_a-strtol.o)
                                  /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc_nano.a(lib_a-atoi.o) (_strtol_r)
    /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc_nano.a(lib_a-ctype_.o)
                                  /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc_nano.a(lib_a-strtol.o) (_ctype_)
```

We now have a better sense of the files actually included into our program, and the reason why they are part of the game. What's more inside that file?

### Memory map

The most straightforward pieces of information in the map file are the actual memory regions:

```
    Memory Configuration
    
    Name             Origin             Length             Attributes
    FLASH            0x0000000000001000 0x00000000000ff000 xr
    RAM              0x0000000020000008 0x000000000003fff8 xrw
    *default*        0x0000000000000000 0xffffffffffffffff
```

Following that table is the `Linker script and memory map` which directly indicates the `text` area size and its content. As always, the interrupt vectors (here under section `.isr_vector`) are present at the beginning of the executable, defined in `gcc_startup_nrf52840.S`:

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

Scrolling those lines give the address of each function and its size. Above, you can read the address of `bsp_board_led_invert`, coming from `boards.c.o` (compilation unit of `board.c` as you guessed) which as a size of 0x34 bytes in the `text` area. That way, we are able to locate each function used in the program.

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

Including a call to `atoi` made the `text` area to be increased but also the `data` area. I used a diff tool to make things simpler and discovered data that was before discarded by the linker:

```
    .data._impure_ptr
    								0x0000000000000000        0x4 /usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/lib/thumb/v7e-m+fp/hard/libc_nano.a(lib_a-impure.o)
```

One last piece of information to keep in mind: initialized variables have to be kept in Flash but they appear in `RAM` in the map file because they are copied into RAM before entering the `main` function. It means that you better have to initialize variables to `0` to put them in the `BSS` section in order to free up some Flash. Here, symbols `__data_start__` and `__data_end__` keep track of the area used in RAM to keep initialized variables. Those values are stored starting at `0x00000000000018d0`:

```
    .data           0x0000000020000008       0x70 load address 0x00000000000018d0
                    0x0000000020000008                __data_start__ = .
    [...]
    								0x0000000020000078                __data_end__ = .
```

Note that symbols that are compiled but not needed are also listed in the map file in the `Discarded input sections` part.

Most of the information from the map file has been analyzed. 

Several usages of the map file are possible. Most of the time, you will have an address and you will want to know which function or data is being used. But some other times, it will be useful for debugging...

## Debugging a linking error

Map files are created while the built code (`.o` files) is linked together which means it can be used to resolve errors in the linking process. I remember working on a bootloader that was contained in a few Flash pages. At some point, I wanted to use `atoi` but the bootloader didn't compile anymore because there were no more space available...

Using the previous example, let's say I now have only 0x800 bytes of Flash. If I want to compile the example that is not using `atoi`, I don't have any issue. But if I add the call to `atoi`, which needs several symbols to be linked into the executable and I obtain:

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

Reading the map file will teach you a lot about the code you are working on, and it's the first path to better quality Firmware.

Feel free to share your story with any interesting usage of the map files.

## Sources

[https://www.embeddedrelated.com/showarticle/900.php](https://www.embeddedrelated.com/showarticle/900.php)

[https://stackoverflow.com/questions/755783/whats-the-use-of-map-files-the-linker-produces](https://stackoverflow.com/questions/755783/whats-the-use-of-map-files-the-linker-produces)

[https://sourceware.org/binutils/docs/ld/Options.html#index-_002d_002dprint_002dmap](https://sourceware.org/binutils/docs/ld/Options.html#index-_002d_002dprint_002dmap)