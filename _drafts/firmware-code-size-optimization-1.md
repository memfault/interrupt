---
title: "Firmware size optimizations: measuring"
author: francois
---

<!-- excerpt start -->

Every firmware engineer has run out of code size at some point or another.
Whether they are trying to cram in another feature, or to make enough space for
[A/B firmware
updates](https://sbabic.github.io/swupdate/overview.html#double-copy-with-fall-back)
more code space is always better.

In this series of posts, we'll explore ways to save code space and ways not to
do it. We will cover compiler options, coding style, logging, as well as
desperate hacks when all you need is another 24 bytes.

But first, let's talk about measuring code size.

<!-- excerpt end -->

## Why measure?

Optimization is often counter intuitive. Don't take my word for it: this is one
of the topics [Microsoft's Raymond
Wong](https://devblogs.microsoft.com/oldnewthing/20041216-00/?p=36973) and [Unix
programmers](https://homepage.cs.uri.edu/~thenry/resources/unix_art/ch12s02.html)
agree on!

So before you do anything, measure where your code size is going. You may be
surprised!

## Measuring overall code size

The simplest way to measure code size is to inspect the different sections of
your elf file. The ARM GCC toolchain comes with a handy utility to do just that:
`arm-none-eabi-size`.

Let's try running it on one of our ELF files:

```terminal
francois-mba:with-libc francois$ arm-none-eabi-size build/with-libc.elf
   text    data     bss     dec     hex filename
  10800     104    8272   19176    4ae8 build/with-libc.elf
```

So in this program we have:
* 10800 bytes of `text`, which is code
* 104 bytes of `data`, which is statically initalized memory
* 8272 bytes of `bcc`, which is zero-initialized memory

Unfortunately, the output of this tool is a little misleading. You may think
that your program will occupy `10800` bytes of flash, but this is not the case.

Indeed if we modify our linker script to specify that our flash size is 10800
exactly:

```diff
diff --git a/with-libc/samd21g18a_flash.ld b/with-libc/samd21g18a_flash.ld
index a07cc82b..f49e73e2 100644
--- a/with-libc/samd21g18a_flash.ld
+++ b/with-libc/samd21g18a_flash.ld
@@ -3,7 +3,7 @@ OUTPUT_ARCH(arm)
 
 MEMORY
 {
-  rom      (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00040000
+  rom      (rx)  : ORIGIN = 0x00000000, LENGTH = 10800
   ram      (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00008000
 }
```

We end up overflowing our flash!

```terminal
francois-mba:with-libc francois$ make
[...]
/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/bin/ld:
build/with-libc.elf section `.data' will not fit in region `rom'
/usr/local/Cellar/arm-none-eabi-gcc/8-2018-q4-major/gcc/bin/../lib/gcc/arm-none-eabi/8.2.1/../../../../arm-none-eabi/bin/ld:
region `rom' overflowed by 104 bytes
collect2: error: ld returned 1 exit status
make: *** [build/with-libc.elf] Error 1
```

The 104 bytes overflow hints at the reason: we are overflowing our flash by
the size of our `data` section. This is because initialization values for our
initialized static variables are stored in flash as well.

To avoid confusion, I prefer to use a small script to compute the sizes:

```bash
#!/bin/bash

if [  $# -le 2 ]
then
    echo "This script requires 3 arguments."
    echo -e "\nUsage:\nget-fw-size FILE MAX_FLASH_SIZE MAX_RAM_SIZE \n"
    exit 1
fi

file=$1
max_flash=$2
max_ram=$3

function print_region() {
    size=$1
    max_size=$2
    name=$3

    if [[ $max_size == 0x* ]];
    then
        max_size=$(echo ${max_size:2})
        max_size=$(( 16#$max_size ))
    fi

    pct=$(( 100 * $size / $max_size ))
    echo "$name used: $size / $max_size ($pct%)"
}

raw=$(arm-none-eabi-size $file)

text=$(echo $raw | cut -d ' ' -f 7)
data=$(echo $raw | cut -d ' ' -f 8)
bss=$(echo $raw | cut -d ' ' -f 9)

flash=$(($text + $data))
ram=$(($data + $bss))

print_region $flash $max_flash "Flash"
print_region $ram $max_ram "RAM"
```

which gives us:
```terminal
francois-mba:with-libc francois$ ./get-fw-size build/with-libc.elf 0x40000
0x8000
Flash used: 10904 / 262144 (4%)
RAM used: 8376 / 32768 (25%)
```

Stick that in your Makefiles, and you'll have the size as you build
your firmware.

## Digging into functions

The above tells us *how much* code space we are using, but not *why*. To answer
the latter, we need to go deeper.

This is where another tool bundled with the GNU ARM toolchain comes in handy:
`arm-none-eabi-nm`.

`nm` allows us to print out all of the symbols in an ELF file, and with the
`--size` option it will also print the size of each symbol, and even sort them
by size if you specify `--size-sort`.

So if we want to dig into what symbols consume the most code space, we simply
do:

```terminal
francois-mba:with-libc francois$ arm-none-eabi-nm --print-size --size-sort --radix=d build/with-libc.elf | tail -30
00004420 00000100 T _write
00006948 00000112 T __sinit
00002096 00000118 T _sercom_get_sync_baud_val
00002684 00000124 T sercom_set_gclk_generator
00009864 00000128 T _fflush_r
00010112 00000136 T __smakebuf_r
00007060 00000144 T __sfp
00005676 00000144 T system_gclk_chan_disable
00010304 00000148 T _free_r
00009104 00000172 T __swbuf_r
00000000 00000180 T exception_table
00005112 00000180 T system_clock_source_get_hz
00007276 00000188 T _malloc_r
00003894 00000190 t usart_get_config_defaults
00005352 00000196 T system_gclk_gen_get_hz
00003332 00000200 T Reset_Handler
00001636 00000214 T usart_read_wait
00008164 00000218 T _printf_common
00001872 00000224 t long_division
00009316 00000236 T __swsetup_r
00006320 00000266 T __udivsi3
00005984 00000272 t _system_pinmux_config
00009588 00000276 T __sflush_r
00001122 00000416 T usart_init
00002808 00000444 T _sercom_get_default_pad
00002216 00000468 T _sercom_get_async_baud_val
00008384 00000532 T _printf_i
00007544 00000620 T _vfiprintf_r
00007544 00000620 T _vfprintf_r
00000430 00000692 t _usart_set_config
```


