---
title: "Improving Compilation Time of Embedded Projects"
description:
  "Strategies for speeding up C/C++ compile times for embedded software projects
  using ccache, preprocessor optimizations, parallel execution, and other
  techniques."
tag: [better-firmware]
author: tyler
image: /img/faster-compilation/faster-builds-ccache-comparison.png
---

We've all likely worked on a software or firmware project where the build times
take a "coffee break" amount of time. It's become such a common occurrence that
there is an infamous [xkcd comic](https://xkcd.com/303/) related to the matter.

<!-- excerpt start -->

Build times don't have to be slow, even yours! However, many factors play a
role. In this post, we'll dive into which factors contribute to slower builds,
compare and contrast options, and also go over some easy wins which you can
immediately contribute to your project and share with your teammates. The
techniques discussed apply to most C/C++ compilers and projects, and there are a
couple extra tips for those using the GNU GCC compiler.

<!-- excerpt end -->

Hold on tight, as there is a lot to talk about.

_Like Interrupt? [Subscribe](http://eepurl.com/gpRedv) to get our latest posts
straight to your mailbox_

{:.no_toc}

## Table of Contents

<!-- prettier-ignore -->
* auto-gen TOC:
{:toc}

## Why Speed Up Firmware Build?

Waiting for a build to complete is a **waste of time**. It's as if someone
decided to tie your hands behind your back a few minutes, tens of hundreds of
times a day, ruining your productivity. If your team consists of 5 engineers and
the build takes 5 minutes, at 20 builds a day (conservative estimate), that's 8
hours...a whole work day for an extra single engineer!

Although we commonly run up against slow build times when trying to move
quickly, there are many other pieces of "infrastructure" (or lack thereof) that
can slow down the productivity of a firmware team.

- Manually verifying the build succeeds for all targets ([continuous
  integration]({% post_url 2019-09-17-continuous-integration-for-firmware %}))
- Manual and rigorous testing ([unit
  testing]({% post_url 2019-10-08-unit-testing-basics %}))
- Painstaking and repetitive debugging ([automated
  debugging]({% post_url 2019-07-02-automate-debugging-with-gdb-python-api %}))
- Poor developer environment management ([using an environment
  manager]({% post_url 2020-01-07-conda-developer-environments %}))
- Inability to reproduce a build ([reproducible
  builds]({% post_url 2019-12-11-reproducible-firmware-builds %}))

Despite the important topics listed above, I believe that the build system is
one of the most important pieces of "infrastructure". The build system needs
constant supervision to keep everyone on the team as productive as possible.
There's no use in paying an engineer (or ten) if the majority of the time during
the day is spent waiting for builds to complete.

## Preamble

To set things straight, I don't love build systems. They are complex,
unintuitive, and require a deep understanding of each one to do things _right_.
The [GNU Make Manual](https://www.gnu.org/software/make/manual/make.html) is one
of the few manuals I've read _cover to cover_, and it was one of the best uses
of my time (and my employer's). When you mash together string substitutions[^8],
shelling out to Python scripts, downloading files from remote servers and
package managers, and custom binary packaging scripts, the build infrastructure
could very easy slow down to a crawl if not done properly.

In this post, we will primarily focus on speeding up the compilation side of the
build system by testing out different compilers and compilation strategies.

## First Steps and Suggestions

These are some up-front suggestions that you should take before trying to speed
up the build using the latter, more time-consuming methods. If one of these
isn't satisfied, I'd start there.

- Don't settle for less than a quad-core CPU & solid-state disk on your
  development machine.
- Don't work on virtual file systems or network drives. Ensure your project is
  on the host's file system.
- Exclude your working directories from your your corporate or built-in
  anti-virus software. I've seen 2-3x slowdowns without the exclusion.
- Use the newest set of compilers and tools that you can reasonably use.
- Ensure incremental builds only compile the minimal set of files (the build
  shouldn't require a "clean" before each "build").
- Ensure no timestamps are injected into the build which would force rebuilds.
- Try not to enable link-time optimizations (LTO) as the linking step sometimes
  becomes as slow as the entire build itself! If you have enabled it due to code
  size constraints, I suggest checking out Interrupt's
  [code size posts](https://interrupt.memfault.com/blog/tag/fw-code-size) for
  easy wins.

## Example Build Environment

To put these suggestions to the test, I took the example
LwIP_HTTP_Server_Netconn_RTOS from STM32CubeF4 project[^4], found a GCC
port[^10], and used that to test the various operating systems, compilers, and
build systems. To easily get the same environment locally, one can clone the
project from the Interrupt repository and perform the appropriate steps for each
compiler.

```
$ git clone git@github.com:memfault/interrupt.git
$ cd interrupt/example/faster-compilation
```

> Make will automatically download the STM32CubeF4 and extract it to the
> appropriate place. When not using Make, download the distribution from
> [Github](https://github.com/STMicroelectronics/STM32CubeF4) and extract it to
> `stm32cube/`

- GCC - Using Make, run `make`. Works on macOS, Linux, and Windows
- Keil - Open
  `stm32cube/Projects/STM32F429ZI-Nucleo/Applications/LwIP/LwIP_HTTP_Server_Netconn_RTOS/MDK-ARM/Project.uvprojx`
- IAR - Open
  `stm32cube/Projects/STM32F429ZI-Nucleo/Applications/LwIP/LwIP_HTTP_Server_Netconn_RTOS/EWARM/Project.eww`

Below are the versions of the programs tested:

- Keil: MDK-Arm 5.29 on Windows, where `armcc` is ARM Compiler 5.06, and
  `armclang` is ARM Compiler 6.13
- IAR: IAR Embedded Workbench for ARM 8.42.1 on Windows
- GCC: GNU ARM Embedded Toolchain 8-2019-q3[^9] for macOS, Linux, and Windows

The operating system environments were as follows:

- Windows: a VirtualBox image of Windows 10 was used, with 8GB RAM and 4 CPU
- Linux: Ubuntu 18.04 on Docker, with 8GB RAM and 4 CPU
- macOS: 10.14 Mojave native installation, 16GB RAM and 4 CPU.

## Steps Towards Faster Builds

### Parallel Compilation

The first and easiest thing to do to speed up builds is ensure you are building
with multiple threads. Make, IAR, and Keil all support parallel compilation, so
ensure you and the entire team are using it.

Below are some times recorded compiling the example project with GCC and
different thread counts.

![comparison of compiler build times with GCC threads](/img/faster-compilation/gcc-threads-build-time-comparison.svg)

With Make, it is as simple as invoking Make with the argument `-j` or `-jN`,
where `N` is the number of desired threads to use. If you or your team members
occasionally forget to pass in this parameter, I suggest wrapping the command
using Invoke, as I wrote about in my [Building a CLI for Firmware
Projects]({% post_url 2019-08-27-building-a-cli-for-firmware-projects %}#adding-parallel-builds)
post.

If you are using Make in a recursive manner, **make sure** you are using
`$(MAKE)` instead of `make` when using it in a rule, as follows:

```make
# GOOD
somerule:
	$(MAKE) -C subdir

# BAD
somerule:
	make -C subdir
```

If you passed in `-j4 --output-sync=recurse` as arguments to your initial Make
call, these will also be passed through to future `$(MAKE)` calls. In the second
`# BAD` example above, they will not be propagated and Make will run `somerule:`
in a single thread.

### Use A Faster Compiler

Some compilers are faster than others. The build times of our example project
shown below were calculated by taking the average time among 5 builds for each
setup.

![comparison of compiler build times](/img/faster-compilation/all-compilers-build-time-comparison.svg)

GCC for macOS, Linux, and Windows performed the best regardless of operating
system, coming in just under 10 seconds. ARM Compiler 6 with Keil then IAR
followed close behind. Keil with ARM Compiler 5 (armcc) was _much_ slower. If
you are using armcc, I'd try to upgrade ASAP.

### ccache - Compiler Cache

> NOTE: This is trivial to set up when using GCC + Make. It might be more
> difficult or impossible with other build systems.

ccache[^1] is a compiler cache that speeds up recompilation. It compares the
input files and compilation flags to any previous inputs it has previously
compiled, and if there is a match, it will pull the compiled object file from
its cache and provide that instead of recompiling the input. It is useful when
switching branches often and in CI systems where subsequent builds are very
similar. It's not uncommon to see 10x speedups for large projects.

![comparison of compile build times with ccache](/img/faster-compilation/ccache-build-time-comparison.svg)

You can use ccache by prefixing the compiler when compiling individual files.

```
$ ccache arm-none-eabi-gcc <args>
```

ccache can be installed using in the following ways for each operating system:

- macOS - [Brew](https://formulae.brew.sh/formula/ccache) or
  [Conda](https://anaconda.org/conda-forge/ccache)
- Linux - [Apt](https://launchpad.net/ubuntu/bionic/+package/ccache) or
  [Conda](https://anaconda.org/conda-forge/ccache)
- Windows -
  [Github nagayasu-shinya/ccache-win64](https://github.com/nagayasu-shinya/ccache-win64)

I suggest **against** the commonly suggested way of symlinking your compilers to
the `ccache` binary and instead to set up your build system in the following
way:

```make
ARM_CC    ?= arm-none-eabi-gcc
CC_PREFIX ?= ccache
CC        ?= $(CC_PREFIX) $(ARM_CC)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<
```

With this setup, your environment remains simple and developers only need to
install ccache with no further setup.

You can easily check whether ccache is set up properly using the following steps
and verifying there are "cache hits".

```
# This initial build with populate the cache
$ make
...

# Clear ccache stats
$ ccache --zero-stats

# Run the build again (most artifacts should be cached)
$ make
...

# Ensure ccache had some hits
$ ccache --show-stats
cache directory                     /Users/tyler/.ccache
primary config                      /Users/tyler/.ccache/ccache.conf
secondary config      (readonly)    /usr/local/Cellar/ccache/3.7.3/etc/ccache.conf
stats updated                       Fri Jan  3 14:10:39 2020
cache hit (direct)                   540
cache hit (preprocessed)             140
cache miss                           303
cache hit rate                     69.18 %
called for link                       31
cleanups performed                     0
files in cache                       853
cache size                          76.1 MB
max cache size                       5.0 GB
```

> If you have a _very_ large project, I've heard good things about icecc[^2] and
> sccache[^3], but these are generally for projects with thousands of large
> files and many developers.

### Optimizing Header Includes and Dependencies

This section goes into how to clean up and optimize dependencies between your
source files and headers.

#### Large Files = Problem

Generally speaking, the larger the input file into the compiler is, the longer
it will take to compile. If the compiler is fed a file with 10 lines, it might
take 10 ms to compile, but with 5000 lines, it might take 100 ms.

One step that happens before a file is compiled is "reprocessing". A
preprocessor takes all `#include "file.h"` statements and replaces them with the
_entire contents_ of the file being included, and does this recursively. This
means that a `.c` file as small as the following could wind up being 5000+ lines
of code for the compiler!

```c
// extra.c

#include "stm32f4xx.h"

char *extra(void) {
  return "extra";
}
```

We can check the contents of the preprocessed (post-processed) file by calling
`gcc` with the `-E` flag.

```
$ arm-none-eabi-gcc -E -g -Os ... -c src/extra.c > preprocessed.txt
$ cat preprocessed.txt | wc -l
    5397
```

It turns out that the header file `stm32f4xx.h` and all of _its_ included header
files amass into a file of 5398 lines. To put some build numbers to this result,
let's test compiling this small file _with_ and _without_ the `stm32f4xx.h`
header.

```
# Header included
$ time arm-none-eabi-gcc ... -c -o obj/extra.o ./src/extra.c
0.17s user 0.03s system 96% cpu 0.206 total

# Header excluded
$ time arm-none-eabi-gcc ... -c -o obj/extra.o ./src/extra.c
0.03s user 0.02s system 90% cpu 0.053 total
```

This shows that, just by including a single header to our otherwise 3 line `.c`
file, the file went from being built in 50 ms to 200 ms. In the next section
we'll see how easy it is to slow down every compilation due to an accidental or
large included header file.

#### Preventing Dependencies is _hard_

It's _incredibly_ easy to accidentally add a nasty header dependency chain into
your code base. Take for example the file `FreeRTOSConfig.h` in the Amazon AWS
FreeRTOS nRF52840 port,
[linked here](https://github.com/aws/amazon-freertos/blob/e6d2e075e1dfa6729ab818a1b80cdb983b688492/vendors/nordic/boards/nrf52840-dk/aws_demos/config_files/FreeRTOSConfig.h).

That configuration file should have _no external dependencies_. It's also a file
that you should be free to import anywhere and everywhere, as it's small and
self contained, consisting only of macros. However, in this particular nRF52840
port, the `FreeRTOSConfig.h` file has a few extra seemingly innocent imports.

Check out the dependency graph below for the file `FreeRTOSConfig.h` mentioned
above:

<p align="center">
  <img width="600" src="/img/faster-compilation/aws-freertos-nrf52-dependencies.svg" alt="AWS FreeRTOS nrf52 config dependency issues" />
</p>

You can see that, while the `FreeRTOSConfig.h` header should have little to no
dependencies, it ultimately includes 30 other header files along with it.

If we take the FreeRTOS linked list implementation file `FreeRTOS/list.c` and
compile it, it will now include _all_ of those headers mentioned above, which
include the vendor and hardware specific files. A file that should take no more
than 50 ms to compile now takes roughly 500 ms on my machine!

To investigate for yourself which headers are recursively included for a single
`.c` file, you can use GCC's `-MMD` flag to print out intermediate `.d` files
for us during the compilation. These files are usually used as an input to Make
for auto dependency generation[^7], but we can also look at them by hand.

I've published the
[AWS FreeRTOS nRF52830 list.d file](https://gist.github.com/tyhoff/a7028c60f8e0db1a548d3f236967109f)
in a Github Gist.

Next time, before including that extra header file, especially _to another
header file_, think twice.

#### Exploring Header Dependency Graphs

Rather than just thinking twice before adding a header file, there are tools
(albeit not great ones) you can use to explore dependency graphs. I won't
explain them here as I have found
[this post by Gernot Klingler on the topic](http://gernotklingler.com/blog/open-source-tools-examine-and-adjust-include-dependencies/)
to be a really good resource on the topic.

The primary way I dig into dependency issues is by investigating GCC's `.d`
files that it produces and look for repeated trees of includes. If you find that
every file in your firmware includes FreeRTOS or standard peripheral headers,
there are probably issues to fix.

One tool I would be wary of using or investing time in is IWYU (Include What You
Use)[^6]. I have found that the egregious dependency issues, the ones that slow
down your build by minutes, can usually be fixed by resolving just _a few_
includes or headers. When running IWYU, it will complain about _every file_ and
will likely hide the root issues. I suggest investigating the `.d` files or one
of the other Graphiz style tools mentioned in the post linked above.

#### Fixing Header Dependency Issues

Speeding up the build by resolving header dependency chains is mostly a
guess-and-check process. You have to try something, do a clean rebuild, and then
compare times or lines of code. I've committed a number of these fixes to
previous projects I've worked on, and I have some tips to share.

1. Don't include headers in your header files (`.h`) that should have been
   included in your `.c` or `.cpp` files.
2. For headers that are included frequently and/or include many other headers,
   consider using forward declarations[^11]. For instance, let's look at the
   following header file.

   ```c
   // settings_file.h

   // Very Large Header File!
   #include "filesystem.h"

   bool settings_file_open(File *file);
   ```

   We are including `filesystem.h` _only_ to have an opaque pointer to a `File`
   defined, but we are paying the price by including such a large file. Since we
   don't need the full internal definition of `File`, we can instead use a
   forward declaration.

   ```c
   // settings_file.h

   typedef struct File File;

   bool settings_file_open(File *file);
   ```

   If you find yourself needing to forward declare this in many places, I
   suggest creating a header file `filesystem_types.h` which contains these
   declarations. Now other files can include this new `_types.h` header instead.

   ```c
   // filesystem_types.h

   typedef struct File File;

   // Other forward declarations
   ```

3. Use [include guards](https://en.wikipedia.org/wiki/Include_guard). I suggest
   using `#pragma once` since it is less error prone, but the standard C way
   works too.
4. Wrap third party library code in an abstraction layer so its internal types
   and headers do not leak where they do not belong. Vendors are not in the
   business of clean and quick compiling code, and you shouldn't be at the mercy
   of their bad habits.

The most significant patch I made related to header dependencies resulted in a
speedup of 30%. Before the change, almost every file in our firmware project had
incidentally included the entire set of our vendor's peripheral library header
files, which are usually **huge**. Each `.c` file was accidentally including
600+ headers.

The fix was to create a couple of `..._types.h` files which contained forward
declarations to help break the dependencies between our code and vendor code.

### Pre-compiled headers

> NOTE: This is a GCC/Clang only feature.

After cleaning up all of the header dependency issues that you can find, there
may still be some headers that you can't avoid including everywhere, and they
are slowing down your build. The good news is that there is a way to pre-compile
your headers with GCC[^12].

Pre-compiling a header means that the compiler will only need to compile a
header _once_, and whenever it is included by other files, it will reuse the
built object. It compiles the header into an intermediate file with the
extension `.h.gch`. If the location of these files is added as an include path
when compiling (before the real headers), GCC will use the pre-compiled versions
instead.

Let's go over the easiest way to do this in GCC, one that doesn't require you to
re-architect your project.

1. Create a header file `precompiled.h` and place it in your source tree.
2. Add a rule to compile this header, just as you would with a `.c` file. It
   should be built _before_ building the `.c` or `.cpp` files. With Make, this
   looks something like:

   ```make
   PRECOMPILED_H := "precompiled.h"
   PRECOMPILED_H_OBJ := $(PRECOMPILED_H).gch

   %.h.gch: $(PRECOMPILED_H)
   	$(CC) $(CFLAGS) -c -o $@ $<
   ```

3. Include this file using a CFLAG argument `-include <>` when compiling the
   `.c` files.

   ```make
   %.o : %.c | $(PRECOMPILED_H_OBJ)
   	$(CC) $(CFLAGS) -include $(PRECOMPILED_H) -c -o $@ $<
   ```

Time to try it out. Let's go back to our `extra.c` file we had defined above.

```c
// extra.c

#include "stm32f4xx.h"

char *extra(void) {
  return "extra";
}
```

Recall that the inclusion of the `stm32f4xx.h` header caused the build time of
this file to slow down by \~150 ms.

If we create a `precompiled.h` file with the contents below

```c
// precompiled.h

#include "stm32f4xx.h"

// Add other large headers
```

and compile our `extra.c` file with this included, we go back to our 50 ms
compile time!

## Profiling the Build System

I've now provided a few ways for you to speed up the compilation of files, but
how do you go about profiling the rest of the build system rules? I have some
good news and bad news.

The good news is that most modern build systems have a way to show the execution
trace of the entire build, broken down by each rule. This is a great way to find
out how long each file takes to build and also to find where the bottle necks
are during parallel builds.

The bad news is that if you are using Make, there isn't a great way to do
profiling. My ultimate suggestion would be to just not use Make and opt for
something more modern. Bazel and CMake are good options. However, continue
reading if you want a couple of hacks for profiling build steps with Make.

### Profiling Individual Make Build Rules

The easiest way to add timing information to compilation steps is to prepend the
`time` command to the desired rules. For instance, if we wanted to profile each
call to `$(CC)`, we could do something like

```make
CC = time arm-none-eabi-gcc
```

or

```make
%.o : %.c
	time $(CC) $(CFLAGS) -c -o $@ $<

```

> NOTE: When using parallel builds in Make, the logging output can get
> scrambled. My advice is to ensure you are using a newer version of GNU Make
> and the `--output-sync=recurse` command line option[^13].

### Profiling Every Make Build Rule

If you want to profile _every_ build rule in your Makefile, a
[clever approach](http://alangrow.com/blog/profiling-every-command-in-a-makefile)
is to override the `SHELL` Make variable with a timing script.

It produces a lot of noise, but it may help you discover slow steps in the
build.

{:.no_toc}

## Closing

I hope this post has helped uncover some ways that you could try to improve the
build time of your firmware projects.

I would argue that it may be more difficult to _keep_ the build time fast than
it is to initially make it fast. For this, I recommend tracking the build times
either in your CI system or in an external database that can be referenced
retroactively. If you notice that your build time somehow slipped from 1 minute
to 2 minutes, you can easily look through all master commits and see where the
build time regression took place.

Do you have any further ideas or ways that you have employed to speed up your
firmware builds? I'd love to hear them!

_All the code used in this blog post is available on
[Github](https://github.com/memfault/interrupt/tree/master/example/faster-compilation/).
See anything you'd like to change? Submit a pull request!_

{:.no_toc}

## References

[^1]: [ccache](https://ccache.dev/)
[^2]: [icecc - IceCream](https://github.com/icecc/icecream)
[^3]: [Mozilla sccache](https://github.com/mozilla/sccache)
[^4]: [STM32Cube LwIP_HTTP_Server_Netconn_RTOS](https://github.com/STMicroelectronics/STM32CubeF4/tree/master/Projects/STM32F429ZI-Nucleo/Applications/LwIP/LwIP_HTTP_Server_Netconn_RTOS)
[^6]: [Include What You Use](https://include-what-you-use.org/)
[^7]: [Make Auto Dependency Generation](http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/)
[^8]: [Make Manual - Functions for String Substitution and Analysis](https://www.gnu.org/software/make/manual/html_node/Text-Functions.html#Text-Functions)
[^9]: [GNU ARM Embedded Toolchain 8-2019-q3](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
[^10]: [stm32cube-gcc](https://github.com/stv0g/stm32cube-gcc)
[^11]: [Forward Declarations Blog Post](https://gieseanw.wordpress.com/2018/02/25/the-joys-of-forward-declarations-results-from-the-real-world/)
[^12]: [GCC - Using Precompiled Headers](https://gcc.gnu.org/onlinedocs/gcc/Precompiled-Headers.html)
[^13]: [Make - Parallel Output](https://www.gnu.org/software/make/manual/html_node/Parallel-Output.html)
