---
title: "Pocket article: Debug vs. Release Builds Considered Harmful"
# title: "Pocket article: Debug vs. Release Builds and why you shouldn't separate them"
description: "Why having separate Debug/Release builds should be avoided"
author: noah
image:
tags: [better-firmware, toolchain, build-system]
---

<!-- excerpt start -->

Separate "debug" and "release" builds are very common in embedded development.
Typically the notion is improved debug capabilities (less aggressive compiler
optimizations, more debugging information like logs) vs. highly optimized and
hardened production release builds. I'm here to describe disadvantages to this
practice, and why it might make sense to consolidate to a single build!

<!-- excerpt end -->

{% include newsletter.html %}

> 🌶️🔥 Warning! this article is taking a strong position on debug-vs-release
> builds. I'm hoping to highlight a few pitfalls you may run into, but it's not
> intended to be a complete discounting of the reasons we end up with bifurcated
> builds (despite the clickbait title)! Please enjoy and as always comments are
> greatly appreciated 🙏

{% include toc.html %}

## Debug vs. Release Builds for Embedded Applications

Many embedded development IDEs provide separate "Debug" and "Release" build
configurations out of the box. Generally the differences between those can be
summarized as:

| Build config | Debug info enabled?          | Compiler optimization?      | `NDEBUG` preprocessor definition?          |
| ------------ | ---------------------------- | --------------------------- | ------------------------------------------ |
| **Debug**    | ✅ enabled (`-g` or similar) | low (`-O0` or `-Og`)        | `DEBUG` is set, or `#define NDEBUG 0`, etc |
| **Release**  | ❌ disabled                  | high (`-O3` or `-Os`/`-Oz`) | `NDEBUG` is set                            |

Let's go one-by-one through these items!

## Debug info

Debug info ("debugging information") is metadata that's included in the output
symbol file (typically `project.elf`/`project.out`/`project.axf`, but almost
always it's an ELF file).

This debugging information is placed into the symbol file in "non-loadable"
sections, which means:

- When flashing the symbol file to a device, the debug info is not loaded (it's
  completely ignored)
- When converting the symbol file to a `.bin` or `.hex` format for factory
  programming, the debug info is omitted

Since enabling debug info has no impact on the final loaded executable, the only
downsides to enabling it are:

- _Slightly_ longer build times (can be ~10%, but difficult to measure, and
  negligible if you're using [build caching]({% post_url
  2020-02-11-improving-compilation-times-c-cpp-projects %})!)
- Much larger symbol file size (can go from approximately the on-target size of
  the program, to many megabytes). This is generally not a problem, except for
  some C++ projects, where the type information can grow dramatically (for
  example, building the Chrome browser with debug info enabled can
  [exceed 4GiB](https://randomascii.wordpress.com/2023/03/08/when-debug-symbols-get-large/)
  ❗)

A quick example demonstrating that compiling with debug info has **zero**
impact on the final program:

```bash
# a simple hello world program
❯ echo > hello.c <<EOF
#include <stdio.h>
int main(void) {
  printf("hello!\n");
  return 0;
}
EOF

# Compile with debug info disabled
❯ gcc -o hello-no-debug hello.c

# Compile it again with debug info enabled
❯ gcc -ggdb3 -o hello-with-debug hello.c

# Run the 'strip' tool to remove all data not part of the program contents.
# Note that we're also removing the (non-loaded) gnu build ID section, because
# the gnu build ID is computed over the program contents AND debug information;
# 'strip' doesn't remove it by default.
# See https://interrupt.memfault.com/blog/reproducible-firmware-builds for
# details!
❯ strip --remove-section=.note.gnu.build-id hello-no-debug hello-with-debug

# compare the two files; they're bit-for-bit identical
❯ diff hello-no-debug hello-with-debug && echo 'identical!'
identical!
```

**Summary: debug info should always be enabled!**

## Compiler optimization level

Setting a lower (or zero) compiler optimization level can help in some
step-debugging scenarios. It prevents the compiler from optimizing away
variables when they're no longer relevant, or rearranging/combining fragments of
code during optimization passes.

Optimization can make step-debugging confusing, but the meaningful information,
such as data structures, variables, and the call stack, is still present.

Anecdotally, I've personally never encountered a roadblock due to compiler
optimization in step-debugging. In the worst case, I need to peek at a few
assembly instructions to see what's going on, but that's very rare.

I've also used the compiler `#pramga GCC optimize ("O0")` or
`__attribute__((optimize("O0")))` to disable optimizations temporarily while
debugging a particular chunk of code or a single file, instead of disabling it
across an entire build.

There's a HUGE downside to having different optimization levels for debug vs.
release builds: we end up with a **different** executable:

- Sizes will likely be different: stack or heap usage (affecting program
  stability!), code space (affecting flash memory allocation, OTA)
- Different machine code executed by the CPU: this can impact **timing** (often
  where the nastiest bugs show up) and actual **program validity**, e.g.
  assumptions you are making about certain spots in your program may no longer
  be valid with different optimization levels (the classic one is instruction
  reordering causing concurrent access bugs!)

Because your debug (testing) build is not running the same code, you may see
these bugs only show up during field test, rather than when the device is on
your desk and connected to a debugger!

It's also generally not desirable to adjust compiler optimization level when a
project is nearing time to ship, to decrease risk. If the debug + release builds
are using the same optimization settings, it's less likely to hit a surprise
out-of-space issue that can stop a project in its tracks 😱.

**Summary: use the same optimization settings for all builds to decrease risk**

## `NDEBUG` preprocessor definition

Often these Debug/Release builds will have some preprocessor-defined symbols
such as:

- `DEBUG=1` (debug enabled)
- `NDEBUG=0` (indicating this is NOT a non-debug build)

Usually these flags will be used to adjust some of the following features for
production builds:

1. Changing **log verbosity** (disabling DEBUG or INFO levels in production, to
   prevent unnecessary flash wear)
   - I'd recommend instead, sticking with the **production** level, and in
     exceptional cases (problematic device), upgrade to a more verbose level
     temporarily
   - Depending on the particular system, it may be preferable to adjust
     verbosity at runtime and keep the build the same. For example, leave
     WARNING and ERROR enabled by default, but allow increasing verbosity via a
     persistent on-device setting, or a temporary runtime flag.
   - Be especially wary of builds with different log verbosity- it can impact
     timing and memory usage _dramatically_!
2. Disabling an **"engineering interface"** (eg ssh or local serial console
   backdoor)
   - This can be a security requirement. An alternative is to build
     authentication into the engineering console, but this can be error prone so
     proceed cautiously.
   - Having the interface available (in an authenticated way) on production
     units can be extremely useful when diagnosing faulty customer units that
     have been returned for analysis!
3. Enabling **signed/encrypted builds**
   - Also very tricky! Best practice is to verify the
     signing/encryption implementation works throughout the development and test
     cycle, instead of causing surprises at the end (execute-in-place encryption
     too slow, key management not actually production ready, etc).
   - It might make the most sense to have a separate "development" key for
     signing/encrypting development builds. A key that is separate and has different
     security concerns than the production key.
4. Disabling **runtime asserts**
    - If your system can tolerate a reset due
   to an `ASSERT` condition, these are _stupendously_ valuable in production
   builds- see [this
   post]({% post_url 2019-11-05-asserts-in-embedded-systems %}) for more
   details

I've seen the `DEBUG` flag used pretty aggressively to cut out entire subsystems
that aren't strictly required for production. This can cause a lot of confusion,
for example the following scenario:

1. Production test build encounters a problem
2. Engineer goes to retrieve logs, realizes the logging subsystem was entirely
   disabled in the "release" build
3. 😩

**Summary: it can be quite hazardous to allow splitting features based on
preprocessor flags, and should only be done if there's a strict requirement.
Recommendation is to only do it when generating builds from a CI server, if
absolutely necessary.**

I prefer feature-specific flags when necessary. This is less confusing than a
global `DEBUG` flag, and less likely to leak to other places:

```c
#if !defined(LOG_LEVEL)
#define LOG_LEVEL kLogLevel_Debug
#endif
```

A crucial thing to be mindful of is turning off watchdogs in your debug version.
Instead, it's strongly advised to insert `__asm("bkpt");` into the watchdog
handler. This will enable you to promptly debug any instances where a watchdog
is triggered during debugging.

Depending on the particular device, it may be necessary to add some GDB hooks
for halt + continue to temporarily disable/reset a watchdog peripheral, so it
doesn't trigger when single-stepping. Some chips (like STM32's) even have a
feature that pauses a watchdog for you when the chip is debugger halted, which
is great!

## Summary

🙅🙅🙅🙅

Avoid separate "Debug" and "Release" builds at all costs!

More seriously, there are many downsides(complexity, wasted time, etc) if a
different build is used during development vs. production. There are
few cases where this is actually required!

My advice is to consider very carefully if you need a separate Release build,
and make sure you're ready to commit to supporting it.

Often it's sufficient to enable or disable a few small features via a
compilation flag for production builds. This can be done, for example, in your
CI server, when preparing a tag for release. That build can also be where
production keys/signing is performed, so it's all in one place and can be
audited later. Avoid "Noah's laptop" builds making it to manufacturing 😅!

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}
