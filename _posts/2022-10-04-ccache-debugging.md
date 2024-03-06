---
title: "Pocket article: Debugging ccache misses"
description: Fixing cache misses when using the ccache compiler cache
author: noah
image: /img/ccache-debugging/ccache.png
tags: [c, c++, build-system]
---

<!-- excerpt start -->

This article provides a few tips and tricks for diagnosing `ccache` misses, to
keep your C/C++ build nice and snappy!

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## ccache: The C Compiler Cache

We've written about the excellent `ccache` tool on Interrupt before:

["Improving Compilation Time of C/C++ Projects"]({% post_url
2020-02-11-improving-compilation-times-c-cpp-projects %})

`ccache` provides a wrapper around C/C++ compiler calls that _caches_ the output
object file, so that future calls with unmodified source files will just copy
the output file from the cache instead of wasting computation time recompiling
for the same result.

Getting the most out of `ccache` has a lot of overlap with Reproducible Builds,
which we've also covered on Interrupt:

[Reproducible Firmware Builds]({% post_url
2019-12-11-reproducible-firmware-builds %})

Using `ccache` can improve compilations times _dramatically_; often a 25x
speedup or faster, depending on the project. Keeping `ccache` working well can
also have a big impact on snappy CI builds, which improves developer quality of
life!

Let's dive right in.

## Diagnosing cache misses

The first step in dealing with `ccache` misses is identifying the cause of the
misses. There's three levels of debug techniques I use here:

### 1. Measure total cache misses

> Note: the example program and Makefile used in this article can be found in
> GitHub here:
> <https://github.com/memfault/interrupt/tree/master/example/ccache-debugging>

`ccache` keeps track of cache hits + misses, which can be viewed by running
`ccache -s` (or `ccache -sv` for more verbose output). When measuring cache
misses, first run `ccache -z` to zero out the stats, then run the build being
measured, then run `ccache -sv` to see the cache hit rate:

```bash
❯ ccache -sv
Summary:
  Cache directory:    /home/noah/.cache/ccache
  Primary config:     /home/noah/.config/ccache/ccache.conf
  Secondary config:   /usr/local/etc/ccache.conf
  Stats updated:      Fri Sep 30 14:30:03 2022
  Hits:                  550 /  998 (55.11 %)
    Direct:              550 /  998 (55.11 %)
    Preprocessed:          0 /  448 (0.00 %)
  Misses:                448
    Direct:              448
    Preprocessed:        448
  Uncacheable:             2
Primary storage:
  Hits:                 1100 / 1996 (55.11 %)
  Misses:                896
  Cache size (GB):      4.64 / 5.00 (92.80 %)
  Files:              127188
  Cleanups:                1
Uncacheable:
  Called for linking:      2
```

In the above example, the hit rate is about 55%, which is pretty poor. It's nice
to have zero cache misses when possible! But often there's a few compilation
steps that use dynamic content and aren't practical to cache- in a large build,
having a few non-cached compilations is usually not too critical.

### 2. Identify objects that are missing the cache

The `ccache` manual does a good job of describing this process in the section
here:

<https://ccache.dev/manual/4.6.3.html#_cache_debugging>

The workflow I use is something like this:

```bash
# run the build command, outputting the ccache debug data into a folder named
# "ccache-debug-1":
❯ CCACHE_DEBUG=1 CCACHE_DEBUGDIR=ccache-debug-1 make
# clean the build
❯ make clean
# run the build command again, outputting ccache debug data into a different
# folder, "ccache-debug-2"
❯ CCACHE_DEBUG=1 CCACHE_DEBUGDIR=ccache-debug-2 make
```

Now there's 2 sets of ccache debug data. Then I run this command to remove some
of the extra output to make it easier to debug (this is just one technique;
using a diff tool with filename filters would be another way):

```bash
# remove all but the *.ccache-input-text ccache debug files
❯ find . \( -name '*.ccache-input-[cdp]' -o -name '*.ccache-log' \) -exec rm {} \;
```

Then I use a directory diff tool, of which there are a lot of options-
personally I use `meld`:

```bash
❯ meld ccache-debug-1 ccache-debug-2
```

![]({% img_url ccache-debugging/meld-dirs.png %})

![]({% img_url ccache-debugging/meld-input-text.png %})

Using `diff`, the output looks like this:

```diff
❯ diff -dur ccache-debug-1 ccache-debug-2
diff --color -dur ccache-debug-1/home/noah/dev/memfault/interrupt/example/ccache-debugging/main.o.ccache-input-text ccache-debug-2/home/noah/dev/memfault/interrupt/example/ccache-debugging/main.o.ccache-input-text
--- ccache-debug-1/home/noah/dev/memfault/interrupt/example/ccache-debugging/main.o.ccache-input-text   2022-09-30 14:48:11.301685115 -0400
+++ ccache-debug-2/home/noah/dev/memfault/interrupt/example/ccache-debugging/main.o.ccache-input-text   2022-09-30 14:48:15.333828955 -0400
@@ -15,7 +15,7 @@
 ### manifest version
 2
 ### arg
--DBUILD_UUID="0fb1039e-3cb7-4761-a3a1-7cb0d87c48ad"
+-DBUILD_UUID="ba796adf-7946-41e9-a295-919c0a785d94"
 ### inputfile
 main.c
 ### sourcecode
@@ -1022,8 +1022,8 @@
 int main(int argc, char **argv) {
   (void)argc, (void)argv;

-  printf("%s\n", "0fb1039e-3cb7-4761-a3a1-7cb0d87c48ad");
-  printf("%s : %s\n", "Sep 30 2022", "14:48:11");
+  printf("%s\n", "ba796adf-7946-41e9-a295-919c0a785d94");
+  printf("%s : %s\n", "Sep 30 2022", "14:48:15");

   return 0;
 }
```

From this, we can see that there's a compiler argument, `-DBUILD_UUID`, that
changed from the first build to the second, and caused the cache miss.

We can also see there's a timestamp that changed from the first build to the
second.

### 3. Detailed examination of the ccache debug log

`ccache` in debug mode will spit out a detailed debug log. If the simple diff
technique above doesn't uncover the issue, digging through the log of a miss
should show the problem, though it's a bit more tedious.

To find `ccache` debug log files with cache misses, I use the following
`ripgrep` command:

```bash
❯ rg --no-ignore 'Result: .*_miss'
ccache-debug-1/home/noah/dev/memfault/interrupt/example/ccache-debugging/main.o.ccache-log
65:[2022-09-30T14:58:02.545438 137617] Result: cache_miss
66:[2022-09-30T14:58:02.545440 137617] Result: direct_cache_miss
67:[2022-09-30T14:58:02.545441 137617] Result: preprocessed_cache_miss
68:[2022-09-30T14:58:02.545443 137617] Result: primary_storage_miss
```

Opening up that log file, we can see some interesting entries below:

```log
[2022-09-30T14:58:02.527849 137617] Command line: ccache cc -DBUILD_UUID="00000000-0000-0000-0000-000000000000" -c main.c -o main.o
[2022-09-30T14:58:02.527854 137617] Hostname: thinkpad-x13
[2022-09-30T14:58:02.527856 137617] Working directory: /home/noah/dev/memfault/interrupt/example/ccache-debugging
[2022-09-30T14:58:02.527857 137617] Compiler type: gcc
[2022-09-30T14:58:02.527878 137617] Source file: main.c
[2022-09-30T14:58:02.527880 137617] Object file: main.o
[2022-09-30T14:58:02.528061 137617] Trying direct lookup
[2022-09-30T14:58:02.528074 137617] Found __DATE__ in main.c
[2022-09-30T14:58:02.528076 137617] Found __TIME__ in main.c
[2022-09-30T14:58:02.528078 137617] Disabling direct mode
[2022-09-30T14:58:02.528112 137617] Running preprocessor
[2022-09-30T14:58:02.528115 137617] Executing /usr/bin/cc -DBUILD_UUID="00000000-0000-0000-0000-000000000000" -E main.c
[2022-09-30T14:58:02.533521 137617] Include file generated_header.h too new
[2022-09-30T14:58:02.533552 137617] Got result key from preprocessor
[2022-09-30T14:58:02.533576 137617] No 33a3eri78s7c31pfvspaj5g3e4o9g2q78 in primary storage
[2022-09-30T14:58:02.533581 137617] Running real compiler
[2022-09-30T14:58:02.533600 137617] Executing /usr/bin/cc -DBUILD_UUID="00000000-0000-0000-0000-000000000000" -c -fdiagnostics-color -o main.o main.c
[2022-09-30T14:58:02.545265 137617] Using default compression level 1
[2022-09-30T14:58:02.545320 137617] Storing result file main.o
[2022-09-30T14:58:02.545324 137617] Storing embedded file #0 .o (1736 bytes) from main.o
[2022-09-30T14:58:02.545417 137617] Stored 33a3eri78s7c31pfvspaj5g3e4o9g2q78 in primary storage (/home/noah/.cache/ccache/3/3/a3eri78s7c31pfvspaj5g3e4o9g2q78R)
[2022-09-30T14:58:02.545438 137617] Result: cache_miss
[2022-09-30T14:58:02.545440 137617] Result: direct_cache_miss
[2022-09-30T14:58:02.545441 137617] Result: preprocessed_cache_miss
[2022-09-30T14:58:02.545443 137617] Result: primary_storage_miss
```

Some notable log messages:

1. `Found __DATE__ in main.c` : this disables "direct" cache mode
2. `Include file generated_header.h too new` : this also disables "direct" mode

Dealing with those problems (see below for the strategies) and re-running
results in a successful cache hit 🎉, with this debug log entry:

```log
5:04:39.975549 141290] Command line: ccache cc -DBUILD_UUID="00000000-0000-0000-0000-000000000000" -c main.c -o main.o
[2022-09-30T15:04:39.975554 141290] Hostname: thinkpad-x13
[2022-09-30T15:04:39.975555 141290] Working directory: /home/noah/dev/memfault/interrupt/example/ccache-debugging
[2022-09-30T15:04:39.975557 141290] Compiler type: gcc
[2022-09-30T15:04:39.975580 141290] Source file: main.c
[2022-09-30T15:04:39.975582 141290] Object file: main.o
[2022-09-30T15:04:39.975661 141290] Trying direct lookup
[2022-09-30T15:04:39.975692 141290] Retrieved 9727fqbvqefb749v8pop2k8o8a9tbsf78 from primary storage (/home/noah/.cache/ccache/9/7/27fqbvqefb749v8pop2k8o8a9tbsf78M)
[2022-09-30T15:04:39.975700 141290] Looking for result key in /home/noah/.cache/ccache/9/7/27fqbvqefb749v8pop2k8o8a9tbsf78M
[2022-09-30T15:04:39.976110 141290] Got result key from manifest
[2022-09-30T15:04:39.976128 141290] Retrieved 0425ht0hb4kn2n7ubfv5stesqrgc4m3e4 from primary storage (/home/noah/.cache/ccache/0/4/25ht0hb4kn2n7ubfv5stesqrgc4m3e4R)
[2022-09-30T15:04:39.976136 141290] Reading result /home/noah/.cache/ccache/0/4/25ht0hb4kn2n7ubfv5stesqrgc4m3e4R
[2022-09-30T15:04:39.976295 141290] Reading embedded entry #0 .o (1736 bytes)
[2022-09-30T15:04:39.976298 141290] Writing to main.o
[2022-09-30T15:04:39.976326 141290] Succeeded getting cached result
[2022-09-30T15:04:39.976330 141290] Result: direct_cache_hit
[2022-09-30T15:04:39.976332 141290] Result: primary_storage_hit
```

## Common Problems

### Using `__TIME__` / `__DATE__`

It can be pretty handy to have the build timestamp inserted into the final
binary. The downside is this precludes fully reproducible builds 😕.

However, if it's desirable to use these macros in the program, try to isolate
them to a single compilation unit, for example:

```c
// build_time.h

const char *get_build_time(void);
const char *get_build_date(void);
```

```c
// build_time.c
#include "build_time.h"

const char *get_build_time(void) {
  return __TIME__;
}
const char *get_build_date(void) {
  return __DATE__;
}
```

Now instead of a cache miss everywhere that macro is used, there's just a single
miss when compiling `build_time.c` 🥲

If it's undesirable to remove the `__TIME__` etc macros from a library or other
third party code being compiled, but the result is not important, an escape
hatch is to use one of the `ccache` Configuration Options to ignore the macros
when checking for a cache hit:

```bash
❯ CCACHE_SLOPPINESS=time_macros
```

### Generated header files

If a generated header file is used very widely in a build, it can cause a lot of
`ccache` misses. Either rework the system to avoid propagating a generated
header, or take advantage of the `ccache` Configuration Option escape hatch
(note that this one is a little more risky):

```bash
❯ CCACHE_SLOPPINESS=include_file_ctime,include_file_mtime
```

See
<https://ccache.dev/manual/4.6.3.html#_handling_of_newly_created_header_files>

### Build system injected command line options

In the example used in this article, the build system (Makefile) is passing a
random value when compiling `main.c`, which causes a cache miss on every build!

This problem is very situation-specific, but often requires some changes to the
project or build system infrastructure to deal with; in this case, the Makefile
was modified to permit overriding the offending variable when running:

```bash
# override the BUILD_UUID to be a fixed value, for reproducible builds
❯ BUILD_UUID=00000000-0000-0000-0000-000000000000 make
```

Other build systems may permit similar control over dynamic values- it can be
tricky to track them down, so the `ccache` debug log is very helpful here!

### Compiler mtime changes

This problem is pretty niche, but can occur when the compiler itself is freshly
installed, but is the same exact compiler as was used when the cache was last
populated.

For example, if a build is running inside a Docker container, and the GCC
compiler is freshly installed on every build, `ccache` will consider the
compiler to be different.

There are various ways to deal with the problem; probably the most obvious is to
keep the compiler install static, for example by including it inside the Docker
image rather than installing on each build:

```bash
# check the mtime, modified time, for the gcc compiler inside the 'gcc:latest'
# Docker image
❯ docker run --rm -i -t gcc bash -c 'stat $(which gcc)'
  File: /usr/local/bin/gcc
  Size: 1280424         Blocks: 2504       IO Block: 4096   regular file
Device: 2bh/43d Inode: 10651076    Links: 3
Access: (0755/-rwxr-xr-x)  Uid: (    0/    root)   Gid: (    0/    root)
Access: 2022-09-13 21:48:19.000000000 +0000
Modify: 2022-09-13 21:48:19.000000000 +0000
Change: 2022-09-30 19:17:16.755951197 +0000
 Birth: 2022-09-30 19:17:16.583945061 +0000
```

If that's not practical, luckily `ccache` has an option for dealing with this
scenario:

```bash
# have ccache use a hash of the compiler itself, instead of mtime of the
# compiler program
❯ CCACHE_COMPILERCHECK=content
```

More details here:

<https://ccache.dev/manual/4.6.3.html#config_compiler_check>

### `__has_include` False Hit

There's a known limitation with `ccache` when using `__has_include` to
optionally include header files- `ccache` is not able to track if the presence
of the optional file changes, and will fetch from the cache as usual. From the
[manual](https://ccache.dev/manual/4.9.1.html#_the_direct_mode):

> There is a catch with the direct mode: header files that were used by the
> compiler are recorded, but header files that were not used, but would have
> been used if they existed, are not. So, when ccache checks if a result can be
> taken from the cache, it currently can’t check if the existence of a new
> header file should invalidate the result. In practice, the direct mode is safe
> to use in the absolute majority of cases.

The workaround I recommend is to set the `CCACHE_NODIRECT=true` setting, which
forces the preprocessor to run before checking for a cache hit.

Since preprocessor invocation should typically be very fast, I just leave this
setting on globally on my development machine (in the relevant shell rc file),
and you can also set it in CI, for example for GitHub actions:

```yaml
env:
  # disable ccache Direct Mode for all jobs
  CCACHE_NODIRECT: true
```

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

- <https://ccache.dev/manual/4.6.3.html>
- <https://www.methodpark.de/blog/the-c-c-developers-guide-to-avoiding-office-swordfights-part-1-ccache/>

<!-- prettier-ignore-start -->
<!-- prettier-ignore-end -->
