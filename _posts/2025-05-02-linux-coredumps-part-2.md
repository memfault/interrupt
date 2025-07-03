---
title: Linux Coredumps (Part 2) － Shrinking the Core
description:
  "Limiting the size of Linux coredumps by collecting stacks and metadata for
  gdb"
author: blake
tags: [linux, coredumps, memfault, debugging]
---

In [our previous
article]({% link _posts/2025-02-14-linux-coredumps-part-1.md %}), we outlined
what a Linux coredump is and how they work under the hood. One common constraint
we see in embedded Linux, however, is a limited amount of storage space. Whether
we're trying to limit writes to disk, or need to reserve most of the disk space
available to a device for other data, sometimes we just don't have enough space
to store coredumps.

<!-- excerpt start -->

In this article, we'll take a look at what comprises a coredump, why they can be
so large, and what we can strip away to make them smaller while retaining
critical debugging information.

<!-- excerpt end -->

> 🎥 Listen to a recording of Blake's talk from Open Source Summit North America
> 2025,
> ["Efficient On-Device Core Dump Processing for IoT: A Rusty Implementation,"](https://www.youtube.com/watch?v=fDwDXg7T4K8)
> for an even deeper diver into the techniques explored in this series.

{% include newsletter.html %}

{% include toc.html %}

## Defining The Minimum Coredump

If we want to reduce the size of a coredump, we must first define the minimum
set of information we need to still get a stacktrace. At a high level this is
fairly obvious. For a majority of programs, most of the mapped memory will
consumed by the heap. If we can cut this out, we should already see the size of
our coredump go down quite a bit. Outside of this, it's possible for individual
thread stacks to be quite large. If we could only capture the top N bytes of
each stack, we would have not only a smaller corecump, but a more predictable
size. Finally, to use a coredump effectively in `gdb`, we'll need to provide it
some metadata for unpacking the coredump. We'll dive more into this later!

If we condense this down to a list we can see the following requirements for our
slim coredump:

- Limit each stack to the top N bytes
- Remove heap allocations
- Capture metadata needed for debuggers.

By fullfilling the above requirements, we'll have a coredump that stripped down
to just the essentials. This does, however, have two major tradeoffs. For one,
any heap-allocated values on the stack will not be resolved if they're on the
stack, and obviously, we will not be able to look at heap-allocated values at
all. We have found this to be an acceptable tradeoff, however, as most crashes
can be debugged with just the stacktrace and stack allocated variables.

The second tradeoff is that we're limiting the size of each stack. This does
limit the depth of the stacktrace, but in general we have observed two important
characteristics of most coredumps. Typically, the stacks are not that deep, and
for those that are, only the top few frames are actually of any interest. This
allows us to limit the size here, without affecting the debugging in the vast
majority of cases.

### Finding The Stacks

If we want to both only capture the stacks, as well as limit the number of bytes
we capture from each stack, we must find them. Recall from our previous article
how a coredump represents mapped memory as a collection of `PT_LOAD` segments.
If we want to find the `PT_LOAD` segment that contains the stack of the thread
we're looking at, we need to know its PC. With the current PC of the thread, we
can search through all segments and see which segment it falls into.

But how do we get the current PC for each thread? Recall in our last article
[we briefly touched on all the different types of `PT_NOTE` segments that existed](https://interrupt.memfault.com/blog/linux-coredumps-part-1#program-headers-and-segments).
One note in particular, `NT_PRSTATUS` is of interest here. This note contains
information about each process. Here[^prstatus] is the layout of the note
description:

```c
struct elf_prstatus_common
{
 struct elf_siginfo pr_info; /* Info associated with signal */
 short pr_cursig;  /* Current signal */
 unsigned long pr_sigpend; /* Set of pending signals */
 unsigned long pr_sighold; /* Set of held signals */
 pid_t pr_pid;
 pid_t pr_ppid;
 pid_t pr_pgrp;
 pid_t pr_sid;
 struct __kernel_old_timeval pr_utime; /* User time */
 struct __kernel_old_timeval pr_stime; /* System time */
 struct __kernel_old_timeval pr_cutime; /* Cumulative user time */
 struct __kernel_old_timeval pr_cstime; /* Cumulative system time */
};

struct elf_prstatus
{
 struct elf_prstatus_common common;
 elf_gregset_t pr_reg; /* GP registers */
 int pr_fpvalid;  /* True if math co-processor being used.  */
};
```

Of interest here is the `pr_reg` struct member. This has all of the general
purpose registers at the time of the crash. From here we can grab the PC of the
thread, and search through all of the mapped memory regions.

To search through all mapped memory regions we can look at every entry in
`/proc/<pid>/maps`[^proc_pid_maps]. Looking at a single entry, we can see there
is a start and end:

```bash
77837a402000-77837a404000 rw-p 00000000 00:00 0
```

If the PC is inside the range, we've found our stack region! Now, we simply copy
from the PC down to either the start of our stack or until we've reached the max
N bytes we wish to copy!

Now, we only have the memory from each thread! If we tried to load this into
GDB, however, it wouldn't work. That's because we're missing some vital bits of
debug information. Let's take a look at what these sections are, and how we can
identify them.

### `r_debug` - Debug Rendevous Structure

Dynamic linking allows programs to call code from shared libraries on the
system. For example, this can allow a single `libopenssl` binary to handle
cryptography for a variety of applications without the full library binary
needing to be included in each one and many other common bits of functionality.
Since our call stack could eventually enter these linked libraries, we need to
have some information on them if we want to properly reconstruct our stacktrace.

One of the key bits of information we need to know is the translation between
the compile-time address and the addresses at runtime. As a security feature
Linux loads every program into a randomized address at runtime, with a feature
called Address Space Layout Randomization(ASLR). This prevents malicious actors
from sniffing stack address values and attempting to redirect the flow of a
program. You can probably already see how this presents a problem for us. If the
address is randomized at every load, how do we translate the fixed address in
the debug info to the random address at load time?

This is where the `r_debug`[^r_debug], or debug rendevous structure comes in.
While it has many functions, the main reason we need it here is the mapping it
provides from compile time address to runtime address. Below, we can see the
layout of the `r_debug` structure:

```c
struct r_debug
  {
    /* Version number for this protocol.  It should be greater than 0.  */
    int r_version;

    struct link_map *r_map; /* Head of the chain of loaded objects.  */

    /* This is the address of a function internal to the run-time linker,
       that will always be called when the linker begins to map in a
       library or unmap it, and again when the mapping change is complete.
       The debugger can set a breakpoint at this address if it wants to
       notice shared object mapping changes.  */
    ElfW(Addr) r_brk;
    enum
      {
        /* This state value describes the mapping change taking place when
           the `r_brk' address is called.  */
        RT_CONSISTENT,  /* Mapping change is complete.  */
        RT_ADD,   /* Beginning to add a new object.  */
        RT_DELETE  /* Beginning to remove an object mapping.  */
      } r_state;

    ElfW(Addr) r_ldbase; /* Base address the linker is loaded at.  */
  };

struct link_map
  {
    /* These first few members are part of the protocol with the debugger.
       This is the same format used in SVR4.  */

    ElfW(Addr) l_addr;    /* Difference between the address in the ELF
                             file and the addresses in memory.  */
    char *l_name;         /* Absolute file name object was found in.  */
    ElfW(Dyn) *l_ld;      /* Dynamic section of the shared object.  */
    struct link_map *l_next, *l_prev; /* Chain of loaded objects.  */
  };
```

There is a lot going on here, but we're only going to concern ourselves with the
`l_addr` member of `link_map`. This tells GDB how to map the addresses we'll see
at runtime to the addresses present in the compiled ELF.

Now that we know how it's useful, how do we find it? To do that we need to look
at the `PT_DYNAMIC` program header. The `PT_DYNAMIC` structure contains a list
of tags and values. Here is its layout for a 64-bit system:

```c
typedef struct {
  Elf64_Sxword    d_tag;
  union {
      Elf64_Xword d_val;
      Elf64_Addr  d_ptr;
  } d_un;
} Elf64_Dyn;
extern Elf64_Dyn _DYNAMIC[];
```

The `r_debug` structure is in this list of dynamic tags[^dynamic_entries]. It
can be found under the `DT_DEBUG` tag. We can iterate through all the tags,
extract the `r_debug` struct memory range for each, and add it to our coredump.

### Finding Build IDs and Headers For Dynamic Libs

To properly fetch the needed symbols for both our main executable and all
dynamic libraries, we need to collect some metadata on them. We need to get
three things for each mapped file:

1. The ELF header
2. All program headers
3. The build ID note

This will give GDB the ability to properly fetch symbols for any parts of the
stacktrace that may have landed in the dynamic mapped sections of the program.

So how do we find this info? For that we'll have to take a peek into the values
inside `/proc/pid/maps`[^proc_pid_maps]. This contains all mapped memory for the
given PID (Process Identifier). This contains the mapped addresses of stacks,
heaps, and, most importantly, the addresses of dynamically linked libraries.

For every mapped ELF file in the list, we can then extract the three bits of
information we mentioned above.

## Overall Size Savings

Now that we've done all this work, what's the payoff? Let's take a look at a
coredump produced by
`memfaultctl trigger-coredump`[^memfaulctl_trigger_coredump].

For a default coredump with none of the modifications, we see the following
decompressed size:

```bash
ls -la --block-size=k /media/memfault/mar/9d51df03-ba86-43aa-a245-a77a7ea33777/core-fdedc559-3b92-4a39-9156-fe575104b947.elf
-rw-r--r-- 1 root root 2625K May  1 14:59 /media/memfault/mar/9d51df03-ba86-43aa-a245-a77a7ea33777/core-fdedc559-3b92-4a39-9156-fe575104b947.elf
```

That's pretty large! At 2.6MB this could take up a significant amount of space
on devices with smaller flash parts. Now let's take a look at what we get with
our changes:

```bash
ls -la --block-size=k /media/memfault/mar/3bfd1399-659a-4aed-b0d9-540418a6c51d/core-e8afead6-782d-4f22-bb23-8789b4390f1c.elf
-rw-r--r-- 1 root root 75K May  1 14:56 /media/memfault/mar/3bfd1399-659a-4aed-b0d9-540418a6c51d/core-e8afead6-782d-4f22-bb23-8789b4390f1c.elf
```

Only 75k! That's a 35x size reduction! While it will vary from program to
program, this is a pretty huge storage savings. For older and more constrained
devices this opens up a world where we could save several coredumps, and still
be under the size of a single unmodified coredump.

## Conclusion

We've now seen that we can trim down the overall size of our ELF coredump by
stripping it down to the bare minimum needed to construct a human-readable
stacktrace. This does come with some tradeoffs, but if you're trying to save on
disk space, they are worth it.

In the next, and final, part of this article series we'll cover a further
evolution of the coredump handler. We'll look at doing on-device stack unwinding
to accomplish two distinct goals: preventing potentially sensitive customer data
from leaving the device, and reducing the size of the coredump even further by
only providing a list of PCs and any related debug info.

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^r_debug]: [`r_debug`](https://sourceware.org/git/?p=glibc.git;a=blob;f=elf/link.h;h=3b5954d9818e8ea9f35638c55961f861f6ae6057)
[^prstatus]: [`prstatus`](https://elixir.bootlin.com/linux/v6.14.4/source/include/linux/elfcore.h#L48)
[^proc_pid_maps]: [`proc_pid_maps`](https://man7.org/linux/man-pages/man5/proc_pid_maps.5.html)
[^dynamic_entries]: [`dynamic_entries`](https://refspecs.linuxfoundation.org/LSB_1.2.0/gLSB/dynamicsection.html)
[^memfaulctl_trigger_coredump]: [`memfaultctl_trigger_coredump](https://github.com/memfault/memfault-linux-sdk/blob/kirkstone/meta-memfault/recipes-memfault/memfaultd/files/memfaultd/src/cli/memfaultctl/coredump.rs#L28)
<!-- prettier-ignore-end -->
