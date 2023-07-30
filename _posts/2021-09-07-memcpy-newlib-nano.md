---
title: "Profiling newlib-nano's memcpy"
description:
  Quick look at performance of the memcpy implementation in newlib-nano
author: noah
tags: [toolchain, mcu, better-firmware]
---

[Newlib](http://www.sourceware.org/newlib/) is a very popular libc targeting
embedded systems. It's the libc that ships with the [GNU Arm Embedded
Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)
published by ARM.

<!-- excerpt start -->

This article takes a look at one of the commonly used functions provided by the
Newlib C library: `memcpy`. We'll examine the default nano implementation and
the performance implications, comparing it against the faster non-default
implementation.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Source for `memcpy` in Newlib-nano

You can find the implementation of the Newlib-nano `memcpy` function here:

[https://sourceware.org/git/?p=newlib-cygwin.git;a=blob;f=newlib/libc/string/memcpy.c;h=52f716b9275f5d24cedb7d66c41541945d13bfb6;hb=HEAD#l49](https://sourceware.org/git/?p=newlib-cygwin.git;a=blob;f=newlib/libc/string/memcpy.c;h=52f716b9275f5d24cedb7d66c41541945d13bfb6;hb=HEAD#l49)

If you're using the GNU Arm Embedded Toolchain, you can find the exact source
for the newlib library it ships by downloading the source tarball from the Arm
download page; for example, `gcc-arm-none-eabi-10.3-2021.07-src.tar.bz2` is the
latest at the moment.

Looking at the implementation, we can see that it's conditionally compiled in 2
different ways:

```c
void *
__inhibit_loop_to_libcall
memcpy (void *__restrict dst0,
	const void *__restrict src0,
	size_t len0)
{
#if defined(PREFER_SIZE_OVER_SPEED) || defined(__OPTIMIZE_SIZE__)
  char *dst = (char *) dst0;
  char *src = (char *) src0;

  void *save = dst0;

  while (len0--)
    {
      *dst++ = *src++;
    }

  return save;
#else
  // this section skipped for brevity
...
#endif /* not PREFER_SIZE_OVER_SPEED */
}
```

The default compilation used by the GNU Arm Embedded Toolchain is with `-O2` as
the optimization level; however, newlib is also built with `-Os` for the "nano"
variant (both are provided with the pre-built toolchain). We can check this by
looking at the source tarball, where the build script sets these CFLAGS (see
`build-toolchain.sh` line 382):

```plaintext
    saveenvvar CFLAGS_FOR_TARGET '-g -Os -ffunction-sections -fdata-sections'
```

When compiling with optimization level `-Os`, the GCC compiler will set `#define
__OPTIMIZE_SIZE__ 1` as a built-in define. You can see this by running the
following command, which dumps the built-in preprocessor definitions to stdout:

```bash
arm-none-eabi-gcc -Os -dM -E - < /dev/null

# diff between -O2 and -Os
diff -du1w <(arm-none-eabi-gcc -O2 -dM -E - < /dev/null) <(arm-none-eabi-gcc -Os -dM -E - < /dev/null)
```

```diff
@@ -317,2 +317,3 @@
 #define __UINT64_TYPE__ long long unsigned int
+#define __OPTIMIZE_SIZE__ 1
 #define __LLFRACT_MAX__ 0X7FFFFFFFFFFFFFFFP-63LLR
```

This results in the bytewise `memcpy` implementation we saw above!

```c
  while (len0--)
    {
      *dst++ = *src++;
    }
```

> An interesting side-effect of the bytewise implementation I've encountered: a
> program used structure assignment to initialize a set of 32-bit hardware
> registers. This was problematic when the compiler inserted a bytewise memcpy,
> because the registers would be written in 8-bit chunks, putting the hardware
> in an undefined state!

If we examine the other implementation (the non-size-optimized version), we can
see that it attempts to copy 4-byte words at a time, which is the native word
size on a Cortex-M processor, and should speed things up a lot!

```c
  /* If the size is small, or either SRC or DST is unaligned,
     then punt into the byte copy loop.  This should be rare.  */
  if (!TOO_SMALL(len0) && !UNALIGNED (src, dst))
    {
      aligned_dst = (long*)dst;
      aligned_src = (long*)src;

      /* Copy 4X long words at a time if possible.  */
      while (len0 >= BIGBLOCKSIZE)
        {
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          *aligned_dst++ = *aligned_src++;
          len0 -= BIGBLOCKSIZE;
        }

      /* Copy one long word at a time if possible.  */
      while (len0 >= LITTLEBLOCKSIZE)
        {
          *aligned_dst++ = *aligned_src++;
          len0 -= LITTLEBLOCKSIZE;
        }

       /* Pick up any residual with a byte copier.  */
      dst = (char*)aligned_dst;
      src = (char*)aligned_src;
    }

  while (len0--)
    *dst++ = *src++;
```

Note that some chips will have multiple-load-store or other SIMD/vector
instructions that can possibly speed up the loops even more (though it's always
important to profile when playing with compiler optimizations).

## Performance: speed

Let's first take a look at the performance of this implementation by measuring
the number of cycles it takes to copy different amounts of data. We're going to
do our experiment on a Cortex-M4F processor (specific chip was an STM32F407),
using the Cortex-M CYCCNT register (for more information, and the implementation
of the cycle-counting functions, see [this post on profiling]({% post_url
2020-06-02-profiling-firmware-on-cortex-m %})).

```c
  struct test_struct {
    char data[4096];
  };
  // instantiate 2 structs. for our purposes, we don't care what data is in
  // there. set them to `volatile` so the compiler won't optimize away what we
  // do with them
  volatile struct test_struct dest, source;

  enable_cycle_counter(); // << Enable Cycle Counter

  // run through powers-of-two memcpy's, printing stats for each test
  for (size_t len = 1; len <= sizeof(dest); len <<= 1) {
    uint32_t start = read_cycle_counter(); // << Start count
    memcpy((void *)&dest, (void *)&source, len);
    uint32_t stop = read_cycle_counter(); // << Stop count

    // print out the cycles consumed
    printf("len = %lu, cyccnt = %lu, cycles/byte = %0.3f\n",
           (uint32_t)len, stop - start,
           (float)(stop - start) / (float)len);
  }
```

The output of this test is:

```plaintext
len = 1, cyccnt = 30, cycles/byte = 30.000
len = 2, cyccnt = 41, cycles/byte = 20.500
len = 4, cyccnt = 63, cycles/byte = 15.750
len = 8, cyccnt = 107, cycles/byte = 13.375
len = 16, cyccnt = 195, cycles/byte = 12.188
len = 32, cyccnt = 371, cycles/byte = 11.594
len = 64, cyccnt = 723, cycles/byte = 11.297
len = 128, cyccnt = 1427, cycles/byte = 11.148
len = 256, cyccnt = 2835, cycles/byte = 11.074
len = 512, cyccnt = 5651, cycles/byte = 11.037
len = 1024, cyccnt = 11283, cycles/byte = 11.019
len = 2048, cyccnt = 22547, cycles/byte = 11.009
len = 4096, cyccnt = 45075, cycles/byte = 11.005
```

To replace the default `memcpy` implementation with an alternative, what we can
do is:

1. copy the newlib `memcpy` function into a file in our project, eg `memcpy.c`
2. add the file to the sources we're compiling
3. we have to make a couple of modifications to get the result we want:
   1. add a line `#undef __OPTIMIZE_SIZE__` to the file; we saw GCC will set
      this flag if we enable `-Os` when compiling
   2. add the following definition (which is present in the newlib build):

      ```c
      # define __inhibit_loop_to_libcall \
        __attribute__ ((__optimize__ ("-fno-tree-loop-distribute-patterns")))
      ```

      This is required to prevent the C compiler from inserting `memcpy` calls
      within the body of memcpy, which will end up recursing repeatedly and
      probably overflowing the stack 😩.

Rebuilding, we should see our version of memcpy present if we examine the `.map`
file.

Running the same experiment, the results are:

```plaintext
len = 1, cyccnt = 44, cycles/byte = 44.000
len = 2, cyccnt = 58, cycles/byte = 29.000
len = 4, cyccnt = 86, cycles/byte = 21.500
len = 8, cyccnt = 142, cycles/byte = 17.750
len = 16, cyccnt = 71, cycles/byte = 4.438
len = 32, cyccnt = 91, cycles/byte = 2.844
len = 64, cyccnt = 131, cycles/byte = 2.047
len = 128, cyccnt = 211, cycles/byte = 1.648
len = 256, cyccnt = 371, cycles/byte = 1.449
len = 512, cyccnt = 691, cycles/byte = 1.350
len = 1024, cyccnt = 1331, cycles/byte = 1.300
len = 2048, cyccnt = 2611, cycles/byte = 1.275
len = 4096, cyccnt = 5171, cycles/byte = 1.262
```

At the larger sizes, this implementation is ~ **8.7** times faster!

## Performance: size

Let's compare the default implementation with the non-size-optimized one.

```bash
# built with default implementation:
❯ arm-none-eabi-nm build/main.elf --print-size | grep memcpy
0800038a 0000001c T memcpy

# built with the faster, larger implementation
❯ arm-none-eabi-nm build/main.elf --print-size | grep memcpy
08000010 00000094 T memcpy
```

That's 148 bytes (0x94 hex) vs. 28 bytes (0x1c hex), or 120 bytes larger.

## `memcpy` calls inserted during compilation

One side effect of replacing `memcpy` in the entire executable is that the C
compiler will often insert calls to memcpy when performing certain operations-
for example, when initializing a struct. See an example here:

[https://godbolt.org/z/ar3rPdv85](https://godbolt.org/z/ar3rPdv85)

This can have significant performance implications if you have a hot path that
is using C structs for passing data; you might see `memcpy` end up at the top of
your profiling results.

## Newlib's non-nano implementation

The above version of `memcpy` is the one used when linking with
`--specs=nano.specs`; the "nano" version of libc, which is intended to optimize
for code space (the nano specs override some libraries with the "nano" variant,
see
[here](https://cygwin.com/git/?p=newlib-cygwin.git;a=blob;f=libgloss/arm/elf-nano.specs;h=82594bd03cea8584188f625826deb20a50fee603;hb=HEAD)).

Linking without selecting the nano specs yields a much larger binary (in the
test repo, my application went from ~17kB to ~33kB). However, not only do the
non-nano versions more closely match glibc implementations (eg more
`printf/scanf` formatters are supported by default), the implementations tend to
be much more optimized for speed. This implementation of `memcpy` is 308 bytes-
it's implemented in assembly with several unrolled loop optimizations:

> [https://cygwin.com/git/?p=newlib-cygwin.git;a=blob;f=newlib/libc/machine/arm/memcpy-armv7m.S;h=c8bff36f60cf6ed520172c85406f6a9529f41de3;hb=HEAD](https://cygwin.com/git/?p=newlib-cygwin.git;a=blob;f=newlib/libc/machine/arm/memcpy-armv7m.S;h=c8bff36f60cf6ed520172c85406f6a9529f41de3;hb=HEAD)

The same test as we did above results in:

```plaintext
len = 1, cyccnt = 39, cycles/byte = 39.000
len = 2, cyccnt = 40, cycles/byte = 20.000
len = 4, cyccnt = 37, cycles/byte = 9.250
len = 8, cyccnt = 45, cycles/byte = 5.625
len = 16, cyccnt = 43, cycles/byte = 2.688
len = 32, cyccnt = 60, cycles/byte = 1.875
len = 64, cyccnt = 81, cycles/byte = 1.266
len = 128, cyccnt = 134, cycles/byte = 1.047
len = 256, cyccnt = 240, cycles/byte = 0.938
len = 512, cyccnt = 452, cycles/byte = 0.883
len = 1024, cyccnt = 876, cycles/byte = 0.855
len = 2048, cyccnt = 1724, cycles/byte = 0.842
len = 4096, cyccnt = 3420, cycles/byte = 0.835
```

About **13x** faster than the original newlib-nano implementation! 🥳

## Outro

That's all for this article! Hopefully we've illuminated an interesting nook
inside the Newlib C library, and shown a way to quickly compare (lib)c function
implementations.

You can see the example project here:

[https://github.com/noahp/pico-c-cortex-m/tree/c7e9a3178ff7b11828482af989e216c75cd696a2](https://github.com/noahp/pico-c-cortex-m/tree/c7e9a3178ff7b11828482af989e216c75cd696a2)

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}
