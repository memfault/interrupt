---
title: "Pocket article: Undefined Behavior Sanitizer Trap on Error"
description: Using Undefined Behavior Sanitizer on small embedded programs without libubsan
author: noah
image: img/ubsan-trap/sigill.png
tags: [toolchain, gdb]
---

<!-- excerpt start -->

This post is a brief overview on how the Undefined Behavior Sanitizer can be
used to trap unintentional or error prone parts of a C program. We're going to
look at how it's used on a desktop program, as well as a way to use it in small
embedded programs!

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## The Undefined Behavior Sanitizer

Undefined Behavior Sanitizer (UBSan) is part of a set of tools that perform
runtime checks on C family software. You can read more here:

[https://releases.llvm.org/15.0.0/tools/clang/docs/UndefinedBehaviorSanitizer.html](https://releases.llvm.org/15.0.0/tools/clang/docs/UndefinedBehaviorSanitizer.html)

[https://developers.redhat.com/blog/2014/10/16/gcc-undefined-behavior-sanitizer-ubsan](https://developers.redhat.com/blog/2014/10/16/gcc-undefined-behavior-sanitizer-ubsan)

The other sanitizers in the family include
[AddressSanitizer](https://releases.llvm.org/15.0.0/tools/clang/docs/AddressSanitizer.html),
[LeakSanitizer](https://releases.llvm.org/15.0.0/tools/clang/docs/LeakSanitizer.html), and others.

> Note: **_AddressSanitizer_** is a very powerful tool for detecting memory
> out-of-bounds errors, and I highly recommend enabling it in your unit tests. It
> is worthy of a dedicated post so we won't go into too much detail here, other
> than it needs to be carefully integrated with the host platform- it works best
> on Linux (though it is available on some other platforms)

UBSan can catch several types of undefined behavior, for example:

- **array bounds checking**
- **integer / floating point overflows** (note that these are undefined per the
  C spec, but are usually deterministic for a particular compiler + arch +
  lib... however, changing optimization level might change how the UB is
  expressed!)
- **alignment when dereferencing**; it's possible to trick a compiler when
  dereferencing type-punned objects and in other scenarios, this traps those
  operations, though they may also end up in fault handler anyway depending on
  your architecture
- **loading an out-of-range enum**

## Basic Usage

To enable UBSan, add this flag during compilation and linking:

`-fsanitize=undefined`

You may notice that enabling this flag increases the size of the compiled
program. This is due to the runtime checks inserted by the compiler, which also
will increase execution time. For this small example program, running the `size`
program shows a ~50% increase with `-fsanitize-undefined`!

```bash
❯ size ubsan-basic-noubsan ubsan-basic
   text    data     bss     dec     hex filename
   1894     616       8    2518     9d6 ubsan-basic-noubsan
   2874     944       8    3826     ef2 ubsan-basic
```

### Examples

Consider the small C program below:

```c
// ubsan-basic.c

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  (void)argc;

  const char *arr[] = {
      "0",
      "1",
  };

  // sketchy code below- argv may not have a second entry! could easily have an
  // out-of-bounds access in arr!
  int foo = atoi(argv[1]);
  printf("foo is %s\n", arr[foo]);

  return 0;
}
```

#### Without UBsan

Building that program normally:

```bash
❯ gcc ubsan-basic.c -o ubsan-basic
```

And running with problematic input:

```bash
❯ ./ubsan-basic 2
foo is
```

We get a weird result! we've indexed into the `arr` array at an out-of-bounds
position (2), but the program happily fetched that memory location and tried to
print it as a string 😬.

This type of bug is very popular over at
[https://cve.mitre.org/](https://cve.mitre.org/) 😔

#### With UBsan

Building the same program _with_ UBSan:

```bash
❯ gcc -fsanitize=undefined ubsan-basic.c -o ubsan-basic
```

We can see that the runtime was linked in:

```bash
❯ ldd ubsan-basic
        linux-vdso.so.1 (0x00007ffcccdfd000)

        # libubsan below
        libubsan.so.1 => /lib/x86_64-linux-gnu/libubsan.so.1 (0x00007f22c0cb5000)

        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f22c0ac9000)
        libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007f22c0ac2000)
        libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f22c0aa0000)
        libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007f22c0887000)
        libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007f22c086a000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f22c163f000)
        libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f22c071c000)
```

If we run that program with problematic input, UBSan prints a warning, and the
printf was modified to print a `(null)` value instead of the possibly problematic
fetch.

[![img/ubsan-trap/ubsan-basic.png](/img/ubsan-trap/ubsan-basic.png)](/img/ubsan-trap/ubsan-basic.png)

If it's preferable to have the program crash instead, use the
`-fno-sanitize-recover=all` to disable the error recovery functionality and
immediately crash (exit with non-zero exit code) on errors:

```bash
❯ gcc -fsanitize=undefined -fno-sanitize-recover=all ubsan-basic.c -o ubsan-basic
```

[![img/ubsan-trap/ubsan-basic-no-recover.png](/img/ubsan-trap/ubsan-basic-no-recover.png)](/img/ubsan-trap/ubsan-basic-no-recover.png)

See details on sanitize-recover here:

[https://gcc.gnu.org/onlinedocs/gcc-12.2.0/gcc/Instrumentation-Options.html#index-fno-sanitize-recover](https://gcc.gnu.org/onlinedocs/gcc-12.2.0/gcc/Instrumentation-Options.html#index-fno-sanitize-recover)

## `libubsan` library and `-fsanitize-undefined-trap-on-error`

For hosted platforms (eg Linux, BSD's, macOS, Windows), there's usually a
`libubsan` library installed with your toolchain. On my system it's here:

```bash
❯ ldconfig -p | grep libubsan
        libubsan.so.1 (libc6,x86-64) => /lib/x86_64-linux-gnu/libubsan.so.1
```

We saw above that it's linked into our executable, so we end up depending on it
at runtime; on unhosted platforms (eg, embedded bare metal, or RTOS's) it may
not exist! Luckily, both GCC and Clang support this flag:

`-fsanitize-undefined-trap-on-error`

Which will instead insert an undefined instruction (on armv7-m, `udf`) on
errors. Let's see how that works:

```bash
❯ gcc -fsanitize=undefined -fsanitize-undefined-trap-on-error ubsan-basic.c -o ubsan-basic
❯ ldd ubsan-basic
        linux-vdso.so.1 (0x00007ffc6c110000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f62c4b00000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f62c4d04000)

# running with problematic input results in a crash!
❯ ./ubsan-basic 2
[1]    529554 illegal hardware instruction (core dumped)  ./ubsan-basic 2
```

As seen above, the undefined instruction trap should cause your program to crash
in a noticeable way (eg, on Cortex-M processors, you end up in a UsageFault, and
the `UNDEFINSTR` bit will be set in the `UFSR` register). You can use this to
instrument/detect the cases of undefined behavior, and FIX them 🤩!

Compiling with this flag also reduces the output binary somewhat, here's a
comparison for our test program:

```bash
# compile without ubsan
❯ gcc ubsan-basic.c -o ubsan-basic-noubsan
# compile with normal ubsan
❯ gcc -fsanitize=undefined ubsan-basic.c -o ubsan-basic
# compile with ubsan and trap-on-error
❯ gcc -fsanitize=undefined -fsanitize-undefined-trap-on-error ubsan-basic.c -o ubsan-basic-trap-on-error

❯ size ubsan-basic-noubsan ubsan-basic ubsan-basic-trap-on-error
   text    data     bss     dec     hex filename
   1894     616       8    2518     9d6 ubsan-basic-noubsan
   2874     944       8    3826     ef2 ubsan-basic
   2054     616       8    2678     a76 ubsan-basic-trap-on-error
```

## `gdb`

One particularly nice result of using the undefined trap- loading the executable
from above into gdb (after recompiling with `-Og` and `-ggdb3`) and running it
with the bad input will halt at the location of the undefined trap:

```bash
❯ gcc -Og -ggdb3 -fsanitize=undefined -fsanitize-undefined-trap-on-error \
  ubsan-basic.c -o ubsan-basic-trap-on-error
❯ gdb -ex 'r' --args ubsan-basic-trap-on-error 2
```

[![undefined behavior trap in GDB](/img/ubsan-trap/ubsan-undefined-instruction-gdb.png)](/img/ubsan-trap/ubsan-undefined-instruction-gdb.png)

## Undefined trap on an embedded platform

On an embedded platform, you might need to insert a software breakpoint (eg
`__asm__("bkpt 1");` or similar) in the correct fault handler, and also deal
with pushing / popping the stack frame prior to the ISR.

The full description of that technique is beyond the scope of this article but
if you are headed down that path
[https://github.com/adamgreen/CrashCatcher](https://github.com/adamgreen/CrashCatcher)
is worth a look!

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

- [https://releases.llvm.org/15.0.0/tools/clang/docs/UndefinedBehaviorSanitizer.html](https://releases.llvm.org/15.0.0/tools/clang/docs/UndefinedBehaviorSanitizer.html)
- [https://developers.redhat.com/blog/2014/10/16/gcc-undefined-behavior-sanitizer-ubsan](https://developers.redhat.com/blog/2014/10/16/gcc-undefined-behavior-sanitizer-ubsan)
- [https://releases.llvm.org/15.0.0/tools/clang/docs/AddressSanitizer.html](https://releases.llvm.org/15.0.0/tools/clang/docs/AddressSanitizer.html)
- [https://releases.llvm.org/15.0.0/tools/clang/docs/LeakSanitizer.html](https://releases.llvm.org/15.0.0/tools/clang/docs/LeakSanitizer.html)
- [https://gcc.gnu.org/onlinedocs/gcc-12.2.0/gcc/Instrumentation-Options.html#index-fsanitize-undefined-trap-on-error](https://gcc.gnu.org/onlinedocs/gcc-12.2.0/gcc/Instrumentation-Options.html#index-fsanitize-undefined-trap-on-error)

<!-- prettier-ignore-start -->
<!-- prettier-ignore-end -->
