---
title: Dealing with Large Symbol Files
description: Some tools and techniques for reducing debug symbol file sizes
author: noah
image: img/dealing-with-large-symbol-files/diff-debug-symbols.png
tags: [gdb, fw-code-size]
---

<!-- excerpt start -->

Large applications can produce _very_ large symbol files when debug information
is enabled (especially at the higher, more verbose levels of debug info!). This
article describes some techniques for reducing debug symbol file sizes!

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Building With Debug Info Enabled

Let's start with a small C program:

```c
#include <stdio.h>

int main(int argc, char **argv) {
  printf("hello!\n");
  return 0;
}
```

> Note: the example C file and a Makefile with the various `obcopy` and `ld`
> example invocations can be found here:
> <https://github.com/memfault/interrupt/blob/master/example/large-symbol-files/>

Compile it with `gcc hello.c -o hello`, and let's take a look at the output
size:

```bash
❯ size -Ax hello
hello  :
section               size     addr
.interp               0x1c    0x318
.note.gnu.property    0x30    0x338
.note.gnu.build-id    0x24    0x368
.note.ABI-tag         0x20    0x38c
.gnu.hash             0x24    0x3b0
.dynsym               0xa8    0x3d8
.dynstr               0x8d    0x480
.gnu.version           0xe    0x50e
.gnu.version_r        0x30    0x520
.rela.dyn             0xc0    0x550
.rela.plt             0x18    0x610
.init                 0x1b   0x1000
.plt                  0x20   0x1020
.plt.got              0x10   0x1040
.plt.sec              0x10   0x1050
.text                0x112   0x1060
.fini                  0xd   0x1174
.rodata                0xb   0x2000
.eh_frame_hdr         0x34   0x200c
.eh_frame             0xac   0x2040
.init_array            0x8   0x3db8
.fini_array            0x8   0x3dc0
.dynamic             0x1f0   0x3dc8
.got                  0x48   0x3fb8
.data                 0x10   0x4000
.bss                   0x8   0x4010
.comment              0x25      0x0
Total                0x7e9
```

Phew! lots of stuff in there. We can see a few typical sections like `.text`,
`.data`, `.bss`, and several other ones.

Overall file size of this binary is:

```bash
❯ ls -lh hello
-rwxrwxr-x 1 noah noah 16K Mar  2 12:26 hello
```

Wow, 16kB for printing `hello!`! <!-- ddevault rant here? -->

Loading this executable into gdb, we get the following warning:

```bash
Reading symbols from hello...
(No debugging symbols found in hello)
```

🥺

Ok, let's now compile with debug information enabled. Checking the
[manual for GCC](https://gcc.gnu.org/onlinedocs/gcc-12.2.0/gcc/Debugging-Options.html), we
see several different options for debug information. Let's go with the highest
level, `ggdb3`:

```bash
❯ gcc -ggdb3 hello.c -o hello.debug
❯ ls -lh hello*
-rwxrwxr-x 1 noah noah  16K Mar  2 12:26 hello
-rw-rw-r-- 1 noah noah   90 Mar  2 12:25 hello.c
-rwxrwxr-x 1 noah noah  42K Mar  2 12:30 hello.debug
```

Ok, our 90 byte source file compiled to 16kB without debug information, and
**42kB** with debug information 😮!

Checking the section sizes again, we see some new sections prefixed with
`.debug_` that account for all of the added size:

```bash
# diff the result of the 'size' command for the 'hello' and 'hello.debug' files
❯ diff -duw <(size -Ax hello) <(size -Ax hello.debug)
```

```diff
--- /proc/self/fd/13    2022-03-02 12:34:14.796360776 -0500
+++ /proc/self/fd/15    2022-03-02 12:34:14.796360776 -0500
@@ -1,4 +1,4 @@
-hello  :
+hello.debug  :
 section               size     addr
 .interp               0x1c    0x318
 .note.gnu.property    0x30    0x338
@@ -27,6 +27,13 @@
 .data                 0x10   0x4000
 .bss                   0x8   0x4010
 .comment              0x25      0x0
-Total                0x7e9
+.debug_aranges         0x30      0x0
+.debug_info            0xb9      0x0
+.debug_abbrev          0x66      0x0
+.debug_line            0xe8      0x0
+.debug_str           0x4e94      0x0
+.debug_line_str       0x1f5      0x0
+.debug_macro         0x1233      0x0
+Total                0x6cdc
```

> Note: if you're using the `arm-none-eabi` tools from ARM, you might want to
> use those variants of the binutils referenced in this article (`size`,
> `strip`, `objcopy`), for example `arm-none-eabi-size` here instead of `size`

## Strategies for Reducing Debug Info

Let's go through some techniques for reducing the debug info section sizes.

### Remove Debug Info with `strip`

It's possible to entirely remove debug info after building, using the `strip`
binutil:

```bash
❯ strip hello.debug -o hello.strip
# now diff the original, non-debug build with the stripped build
❯ diff -duw <(size -Ax hello) <(size -Ax hello.strip)
```

```diff
--- /proc/self/fd/13    2022-03-02 13:43:27.298081401 -0500
+++ /proc/self/fd/15    2022-03-02 13:43:27.298081401 -0500
@@ -1,4 +1,4 @@
-hello  :
+hello.strip  :
 section               size     addr
 .interp               0x1c    0x318
 .note.gnu.property    0x30    0x338
```

The debug data is entirely removed from the binary:

```bash
❯ ls -l hello*
-rwxrwxr-x 1 noah noah     15952 Mar  2 12:26 hello
-rw-rw-r-- 1 noah noah        90 Mar  2 12:25 hello.c
-rwxrwxr-x 1 noah noah     42336 Mar  2 12:30 hello.debug
-rwxrwxr-x 1 noah noah     14464 Mar  2 13:43 hello.strip
```

The stripped binary is actually a bit smaller than the one compiled without
debug sections (stripping that binary results in an identical file size).
Identifying the additional data that's been stripped is left for a future
exercise 😬.

Since the debug info is removed, as in the original binary, it's no longer
possible to get rich backtraces etc. when debugging the binary.

### Extract Debug Info with `objcopy`

It's possible to copy the debug info sections into a separate file using
`objcopy`:

```bash
❯ objcopy --only-keep-debug hello.debug hello.debug.dbg
❯ ls -lh hello.debug*
-rwxrwxr-x 1 noah noah 42K Mar  2 12:30 hello.debug
-rwxrwxr-x 1 noah noah 31K Mar  2 14:00 hello.debug.dbg
```

Note that the section headers for the non-debug sections are still present in
the file, so using `size` to examine the binary will show deceptive information
(eg, `.text`, `.data`, etc). From the manual:

> Note - the section headers of the stripped sections are preserved, including
> their sizes, but the contents of the section are discarded. The section
> headers are preserved so that other tools can match up the debuginfo file with
> the real executable, even if that executable has been relocated to a different
> address space.

You can then strip the debug binary:

```bash
❯ strip -o hello.debug.stripped hello.debug
```

And in GDB, you can manually load a symbol file:

```bash
Reading symbols from hello.debug.stripped...
(No debugging symbols found in hello.debug.stripped)
>>> symbol-file hello.debug.dbg  # using this command to load the symbol file
Reading symbols from hello.debug.dbg...
```

From the same manual page, it's suggested to add a debuglink so gdb can
automatically locate the debug information:

```bash
objcopy --add-gnu-debuglink=hello.debug.dbg hello.debug.stripped
```

Then GDB auto-loads the debug info file:

```bash
Reading symbols from hello.debug.stripped...
Reading symbols from /home/noah/dev/test/misc/hello.debug.dbg...
```

This only works if the position of the files remains consistent.

> Note that there's a newish approach to using debug info files called
> `debuginfod`, which enables GCC to retrieve debug info from a remote host. You
> can read more about that here:
> <https://developers.redhat.com/blog/2019/10/14/introducing-debuginfod-the-elfutils-debuginfo-server>

### Compress Debug Info with `objcopy --compress-debug-sections`

Instead of `strip`ing or splitting out the debug info, it can be compressed
in-place:

```bash
❯ objcopy --compress-debug-sections hello.debug hello.debug.compressed
❯ ls -lh hello.debug*
-rwxrwxr-x 1 noah noah 42K Mar  2 12:30 hello.debug
-rwxrwxr-x 1 noah noah 26K Mar  2 14:09 hello.debug.compressed
```

The binary size is greatly reduced, even for a small program like this (still
quite a bit larger than the stripped binary though).

View of the debug info section reduction:

```bash
❯ diff -duw <(size -Ax hello.debug) <(size -Ax hello.debug.compressed)
```

```diff
--- /proc/self/fd/13    2022-03-02 14:09:36.028276594 -0500
+++ /proc/self/fd/15    2022-03-02 14:09:36.024276703 -0500
@@ -1,4 +1,4 @@
-hello.debug  :
+hello.debug.compressed  :
 section                size     addr
 .interp                0x1c    0x318
 .note.gnu.property     0x30    0x338
@@ -28,12 +28,12 @@
 .bss                    0x8   0x4010
 .comment               0x25      0x0
 .debug_aranges         0x30      0x0
-.debug_info            0xb9      0x0
+.debug_info            0x98      0x0
 .debug_abbrev          0x66      0x0
-.debug_line            0xe8      0x0
-.debug_str           0x4e94      0x0
-.debug_line_str       0x1f5      0x0
-.debug_macro         0x1233      0x0
-Total                0x6cdc
+.debug_line            0xb6      0x0
+.debug_str           0x1853      0x0
+.debug_line_str       0x103      0x0
+.debug_macro          0xabe      0x0
+Total                0x2de1
```

We can see the `.debug_str` section is most aggressively reduced, but all around
improvement.

There shouldn't be a significant performance hit when loading a compressed vs.
uncompressed binary into GDB; on my computer, it's a few ms for this small
program (using the excellent [hyperfine](https://github.com/sharkdp/hyperfine)
benching tool):

```bash
❯ hyperfine 'gdb -batch -ex quit hello.debug' 'gdb -batch -ex quit hello.debug.compressed'
Benchmark 1: gdb -batch -ex quit hello.debug
  Time (mean ± σ):     418.8 ms ±   9.3 ms    [User: 350.2 ms, System: 67.6 ms]
  Range (min … max):   399.8 ms … 428.9 ms    10 runs

Benchmark 2: gdb -batch -ex quit hello.debug.compressed
  Time (mean ± σ):     425.9 ms ±  14.7 ms    [User: 365.6 ms, System: 60.0 ms]
  Range (min … max):   407.7 ms … 451.3 ms    10 runs

Summary
  'gdb -batch -ex quit hello.debug' ran
    1.02 ± 0.04 times faster than 'gdb -batch -ex quit hello.debug.compressed'
```

With larger files it might be more significant, but I haven't tested that.

Note that it's possible to _decompress_ the debug sections with `objcopy` using
the `--decompress-debug-sections` flag.

### Compress Debug Info when Linking

GNU `ld` and LLVM's `lld` support the `--compress-debug-sections=zlib` option,
which performs the debug section compression when generating the binary rather
than after the fact as in `objcopy --compress-debug-sections`:

<https://sourceware.org/binutils/docs-2.39/ld/Options.html#index-_002d_002dcompress_002ddebug_002dsections_003dzlib>

This gives pretty equivalent results as doing it post-build, and is a great
option to enable if you prefer always-compressed debug info.

## Compress Debug Info with `dwz`

The `dwz` tool available [here](https://sourceware.org/dwz/) (and from many
package managers) performs some nice optimizations of DWARF info (the
fundamental format of the debug info data). You can read some more details in
the man page, either by downloading the source and loading it:

```bash
❯ man -l dwz.1
```

Or from one of the many online copies, for example
[Debian Linux](https://manpages.debian.org/testing/dwz/dwz.1.en.html).

Running it on our test binary:

```bash
❯ dwz hello.debug -o hello.debug.dwz
❯ ls -l hello.debug*
-rwxrwxr-x 1 noah noah 42336 Mar  2 12:30 hello.debug
-rwxrwxr-x 1 noah noah 26144 Mar  2 14:11 hello.debug.compressed
-rwxrwxr-x 1 noah noah 42304 Mar  2 14:22 hello.debug.dwz
-rwxrwxr-x 1 noah noah 14568 Mar  2 14:07 hello.debug.stripped
```

Ok, that didn't really improve much 😕 it's possible the debug info for such a
small program is already pretty optimized, or maybe `dwz` doesn't bother
optimizing below a threshold (I haven't checked).

Let's try it again on a large binary, the `cmake` program compiled with
`CMAKE_BUILD_TYPE=Debug`:

```bash
# use dwz to optimize the debug information in 'bin/cmake'
❯ dwz bin/cmake -o cmake.dwz
# use objcopy to compress debug info in the result
❯ objcopy --compress-debug-sections cmake.dwz cmake.dwz.compressed
# and for completeness, also compress debug info in the original cmake file
❯ objcopy --compress-debug-sections bin/cmake cmake.compressed
# let's take a look at file sizes now
❯ ls -lh bin/cmake cmake.dwz cmake.dwz.compressed cmake.compressed
-rwxrwxr-x 1 noah noah 179M Mar  2 14:46 bin/cmake
-rwxrwxr-x 1 noah noah  88M Mar  2 15:45 cmake.compressed
-rwxrwxr-x 1 noah noah 129M Mar  2 14:47 cmake.dwz
-rwxrwxr-x 1 noah noah  70M Mar  2 14:48 cmake.dwz.compressed
```

To summarize:

| Strategy                            | File Size | Size Compared to Base |
| ----------------------------------- | --------- | --------------------- |
| base                                | 179MB     | 100%                  |
| `dwz`                               | 129MB     | 72%                   |
| `--compress-debug-sections`         | 88MB      | 49%                   |
| `dwz` + `--compress-debug-sections` | 70MB      | 39%                   |

I also tested on another large C++ program, and the savings were very
impressive:

```bash
❯ ls -lh program.*
-rwxrwxr-x 1 noah noah 146M Mar  2 16:51 program
-rw-rw-r-- 1 noah noah  66M Mar  2 16:52 program.compressed
-rwxrwxr-x 1 noah noah  37M Mar  2 16:53 program.dwz
-rwxrwxr-x 1 noah noah  16M Mar  2 16:53 program.dwz.compressed
```

146MB for the original binary, down to 16M for the `dwz` and compressed one, or
11% of the original!

## Analyzing Binaries for Bloat Using `bloaty`

An interesting tool for examining binary bloat is `bloaty`:

<https://github.com/google/bloaty>

It's pretty straightforward to build, I followed the instructions here:

<https://github.com/google/bloaty#install>

It has a nice diff view, for example:

```bash
# run bloaty on the cmake binary with debug symbols.
# this output should look pretty similar to 'size -Ax'
❯ ~/dev/github/bloaty/build/bloaty bin/cmake
    FILE SIZE        VM SIZE
 --------------  --------------
  68.2%   121Mi   0.0%       0    .debug_info
  11.2%  19.9Mi   0.0%       0    .debug_str
   5.9%  10.5Mi  71.9%  10.5Mi    .text
   5.1%  9.04Mi   0.0%       0    .strtab
   3.5%  6.28Mi   0.0%       0    .debug_line
   1.2%  2.21Mi  15.1%  2.21Mi    .eh_frame
   1.2%  2.08Mi   0.0%       0    .symtab
   1.0%  1.81Mi   0.0%       0    .debug_abbrev
   1.0%  1.73Mi   0.0%       0    .debug_aranges
   0.7%  1.17Mi   0.0%       0    .debug_rnglists
   0.4%   724Ki   4.8%   724Ki    .rodata
   0.3%   543Ki   3.6%   543Ki    .eh_frame_hdr
   0.2%   307Ki   2.0%   307Ki    .gcc_except_table
   0.1%   149Ki   1.0%   149Ki    .rela.dyn
   0.0%  63.4Ki   0.4%  63.4Ki    .data.rel.ro
   0.0%  60.4Ki   0.0%       0    .debug_line_str
   0.0%  59.9Ki   0.3%  49.5Ki    [24 Others]
   0.0%       0   0.3%  49.8Ki    .bss
   0.0%  23.4Ki   0.2%  23.4Ki    .dynstr
   0.0%  21.4Ki   0.1%  21.4Ki    .dynsym
   0.0%  19.4Ki   0.1%  19.4Ki    .rela.plt
 100.0%   178Mi 100.0%  14.6Mi    TOTAL

# now run it against the compressed debug info version, and
# also passing a '-- BASE_FILE' to show the difference
❯ ~/dev/github/bloaty/build/bloaty cmake.compressed -- bin/cmake
    FILE SIZE        VM SIZE
 --------------  --------------
  +8.1%      +3  [ = ]       0    .comment
 -85.3% -51.5Ki  [ = ]       0    .debug_line_str
 -66.6%  -801Ki  [ = ]       0    .debug_rnglists
 -78.2% -1.35Mi  [ = ]       0    .debug_aranges
 -79.1% -1.43Mi  [ = ]       0    .debug_abbrev
 -68.1% -4.28Mi  [ = ]       0    .debug_line
 -89.8% -17.9Mi  [ = ]       0    .debug_str
 -53.2% -64.7Mi  [ = ]       0    .debug_info
 -50.7% -90.5Mi  [ = ]       0    TOTAL
```

It's pretty handy for quickly examining binaries, but has a lot of other
features for pretty deep analysis, check out the manual here:

<https://github.com/google/bloaty/blob/master/doc/using.md>

## `upx`: the "Ultimate Packer for eXecutables"

This tool compresses a binary file, adds a wrapper that's capable of unpacking
it into memory at runtime, and bundles that into a standalone executable that
when run behaves like the original binary.

It achieves great compression ratios:

```bash
❯ upx --preserve-build-id -o cmake.upx bin/cmake
❯ ls -lh bin/cmake cmake.upx*
-rwxrwxr-x 1 noah noah 179M Mar  2 14:46 bin/cmake
-rwxrwxr-x 1 noah noah  42M Mar  2 14:46 cmake.upx
```

It also preserves debug information (you have to decompress the file with
`upx -d` first though), what's not to love! (it does require certain features of
the host runtime environment so the executable can unpack itself into memory on
launch and before executing the compressed binary 😅).

## GDB Index Files for Large Symbol Files

Something you may notice when manipulating large binaries in GDB: lots of
operations are slowwww, including the initial load. Fortunately, there's an easy
way to speed this up: pre-generate the GDB index.

For the cmake executable, this works like so:

```bash
❯ cp bin/cmake bin/cmake.gdbindex
❯ gdb-add-index bin/cmake.gdbindex
# let's take a look at file sizes:
❯ ls -lh bin/cmake*
-rwxrwxr-x 1 noah noah 179M Mar  2 14:46 bin/cmake
-rwxrwxr-x 1 noah noah 203M Mar  2 17:44 bin/cmake.gdbindex
# 😓 ouch, that added a lot to the file size
```

Now benchmarking the speed at which gdb can load the files:

```bash
❯ hyperfine 'gdb -batch -ex quit bin/cmake' 'gdb -batch -ex quit bin/cmake.gdbindex'
Benchmark 1: gdb -batch -ex quit bin/cmake
  Time (mean ± σ):      4.105 s ±  0.045 s    [User: 4.814 s, System: 0.146 s]
  Range (min … max):    4.040 s …  4.193 s    10 runs

Benchmark 2: gdb -batch -ex quit bin/cmake.gdbindex
  Time (mean ± σ):      1.046 s ±  0.023 s    [User: 1.847 s, System: 0.128 s]
  Range (min … max):    1.013 s …  1.079 s    10 runs

Summary
  'gdb -batch -ex quit bin/cmake.gdbindex' ran
    3.93 ± 0.09 times faster than 'gdb -batch -ex quit bin/cmake'
```

😮 impressive speed up! It's somewhat of a niche operation to optimize for, but
since it's part of the edit-compile-test loop, it could be worth doing.

## Splitting Debug Info at Build Time

GCC and Clang both support an option to split out DWARF info when compiling
source files into object files:

<https://gcc.gnu.org/onlinedocs/gcc-12.2.0/gcc/Debugging-Options.html#index-gsplit-dwarf>

Here's an example:

```bash
# compile + link with '-gsplit-dwarf'
❯ gcc -ggdb3 -gsplit-dwarf hello.c -o hello.debug

# examine file sizes- note the .dwo file,
# and the smaller hello.debug file size
❯ ls -l hello.debug*
-rwxrwxr-x 1 noah noah 17776 Mar  2 18:03 hello.debug
-rw-rw-r-- 1 noah noah 31208 Mar  2 18:03 hello.debug-hello.dwo

# test that symbols are working correctly
❯ gdb -batch --ex 'info line main' hello.debug
threads module disabled
Line 3 of "hello.c" starts at address 0x1149 <main> and ends at 0x115c <main+19>.

# delete the dwo file and retest
❯ rm hello.debug-hello.dwo
❯ gdb -batch --ex 'info line main' hello.debug
threads module disabled

warning: Could not find DWO CU hello.debug-hello.dwo(0xef39648d198fbc2d) referenced by CU at offset 0x0 [in module /home/noah/dev/test/misc/hello.debug]
Dwarf Error: unexpected tag 'DW_TAG_skeleton_unit' at offset 0x0 [in module /home/noah/dev/test/misc/hello.debug]
No line number information available for address 0x1149 <main>
```

This has a couple of advantages:

- small (essentially stripped) binaries are produced without post-processing
- link times can be dramatically improved, because the linker has to load _much_
  less data when it `memmap`'s the input object files

The disadvantage is you'll need to keep those `.dwo` files around in order for
GDB to locate them when you load the binary, and they're not very portable.

In theory there is the `dwp` tool to pack up the `.dwo` files into a single
archive that can be shipped with the binary, but I had a bad time trying to get
it to work 😞.

<https://gcc.gnu.org/wiki/DebugFissionDWP>

## Clang's `-gembed-source`

This is a very interesting feature that Clang provides (alas, it's not present
in GCC, at least as of GCC 11):

<https://releases.llvm.org/15.0.0/tools/clang/docs/ClangCommandLineReference.html#cmdoption-clang-gembed-source>

Using it goes something like this:

```bash
# build with clang, with -gembed-source.
# also strip the current working directory from the debug prefixes:
# https://interrupt.memfault.com/blog/reproducible-firmware-builds#fdebug-prefix-map
❯ clang -gdwarf-5 -gembed-source -fdebug-prefix-map=$(pwd)=. hello.c -o hello.embedsource
```

(This actually increases the size of the binary, unlike the other tools in this
article, but it's such a nice feature I couldn't resist including it here).

You can access the source data using llvm-dwarfdump or llvm-objcopy:

```bash
❯ llvm-dwarfdump-12 --color --debug-line-str hello.embedsource
hello.embedsource:      file format elf64-x86-64

.debug_line_str contents:
0x00000000: "."
0x00000002: "hello.c"
0x0000000a: "#include <stdio.h>\n\nint main(int argc, char **argv) {\n  printf(\"hello!\\n\");\n  return 0;\n}\n"
```

```bash
❯ llvm-objdump-13 --disassemble-symbols=main --source hello.embedsource

hello.embedsource:      file format elf64-x86-64

Disassembly of section .text:

0000000000401130 <main>:
; int main(int argc, char **argv) {
  401130: 55                            pushq   %rbp
  401131: 48 89 e5                      movq    %rsp, %rbp
  401134: 48 83 ec 10                   subq    $16, %rsp
  401138: c7 45 fc 00 00 00 00          movl    $0, -4(%rbp)
  40113f: 89 7d f8                      movl    %edi, -8(%rbp)
  401142: 48 89 75 f0                   movq    %rsi, -16(%rbp)
;   printf("hello!\n");
  401146: 48 bf 04 20 40 00 00 00 00 00 movabsq $4202500, %rdi          # imm = 0x402004
  401150: b0 00                         movb    $0, %al
  401152: e8 d9 fe ff ff                callq   0x401030 <printf@plt>
;   return 0;
  401157: 31 c0                         xorl    %eax, %eax
  401159: 48 83 c4 10                   addq    $16, %rsp
  40115d: 5d                            popq    %rbp
  40115e: c3                            retq
```

(Unfortunately I couldn't convince `lldb` to show this information
automatically, but hopefully that will be available eventually!)

## Summary

In general, I think the best option to use is:

```bash
objcopy --compress-debug-sections
```

This requires no changes when using the file- debuggers and binutils are all
good about unpacking the data, and worst case it can be reversed with
`--decompress-debug-sections`.

The `dwz` tool definitely performs nice optimizations on larger binaries, but it
seems to be very dependent on the specific file. In some cases it provides
_superb_ optimization.

The remaining strategies covered in this article are more situation-specific.

Hope you enjoyed reading this! I certainly learned a lot investigating these
tools.

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->

- <https://gcc.gnu.org/onlinedocs/gcc-12.2.0/gcc/Debugging-Options.html> GCC debugging options
- <https://sourceware.org/binutils/docs-2.38/binutils/objcopy.html#objcopy> objcopy manual
- <https://sourceware.org/gdb/onlinedocs/gdb/Index-Files.html> GDB index files
- <https://developers.redhat.com/articles/2022/01/10/gdb-developers-gnu-debugger-tutorial-part-2-all-about-debuginfo>
Nice explanation of debug info
- <https://gcc.gnu.org/wiki/DebugFission> DWARF extensions for separate debug info files

<!-- prettier-ignore-end -->
