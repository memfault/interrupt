---
title: C Structure Padding Initialization
description: Details on initialization of padding bits in C language structures
author: noah
image: img/c-stuct-padding-initialization/c-struct-padding.png
tags: [c, security, better-firmware]
---

<!-- excerpt start -->

This article takes a look at a few different aspects of C structure
initialization. In particular, we'll look at when it matters, the current state
of things in Clang and GCC, recommendations, and the ✨ future ✨.

<!-- excerpt end -->

Time to dive into this very niche, but occasionally hazardous corner of the C
language!

{% include newsletter.html %}

{% include toc.html %}

## C Structure Padding

Recently I was reading this
[_excellent_ post](https://thephd.dev/ever-closer-c23-improvements#consistent-warningless-and-intuitive-initialization-with--)
on some of the upcoming features in C23, and it inspired me to do a little
exploration and documentation around the current state of initialization of
padding in structures in the C-language.

For background, let's take this example C structure:

```c
#include <stdint.h>

struct foo {
  uint32_t i;
  uint8_t b;
};
```

By default, padding will be inserted at the end of this structure, to align it
to the largest member size. We can use the
[`pahole`](https://linux.die.net/man/1/pahole) tool to examine structure holes
after compiling (with debug symbols enabled, `-g`):

```c
struct foo {
        uint32_t                   i;                    /*     0     4 */
        uint8_t                    b;                    /*     4     1 */

        /* size: 8, cachelines: 1, members: 2 */
        /* padding: 3 */
        /* last cacheline: 8 bytes */
};
```

My understanding is this is done so if the structure is addressed as part of an
array, the first member of each element in the array will have the same
alignment:

```c
struct foo foo_array[2];
// if &foo_array[0] has 4 byte alignment, we want &foo_array[1]
// to also have 4 byte alignment. This is because on most architectures it is more efficient to access data along boundaries aligned with their size.
```

The other (more commonly encountered) case is where padding is inserted between
structure members:

```c
struct foo_internal_padding {
  uint8_t b;
  uint32_t i;
};
```

Running `pahole` as above shows the padding inserted after the first element:

```c
struct foo {
        uint8_t                    b;                    /*     0     1 */

        /* XXX 3 bytes hole, try to pack */

        uint32_t                   i;                    /*     4     4 */

        /* size: 8, cachelines: 1, members: 2 */
        /* sum members: 5, holes: 1, sum holes: 3 */
        /* last cacheline: 8 bytes */
};
```

> Note! the _excellent_ guide here (which we've linked before), is a great
> reference on structure padding and artisanal hand-packing:
> <http://www.catb.org/esr/structure-packing/>

Of course, to prevent padding, we can force the compiler to pack the structure:

```c
struct __attribute__((packed)) foo {
  uint8_t b;
  uint32_t i;
};
```

Now we have no padding inside the structure:

```c
struct foo {
        uint8_t                    b;                    /*     0     1 */
        uint32_t                   i;                    /*     1     4 */

        /* size: 5, cachelines: 1, members: 2 */
        /* last cacheline: 5 bytes */
} __attribute__((__packed__));
```

Similarly, a structure that would normally have padding at the end will no
longer have it:

```c
struct foo {
        uint32_t                   i;                    /*     0     4 */
        uint8_t                    b;                    /*     4     1 */

        /* size: 5, cachelines: 1, members: 2 */
        /* last cacheline: 5 bytes */
} __attribute__((__packed__));
```

_Note that accessing members of compiler-packed structs often can add compute
overhead; the CPU may need to do bytewise loads and stores depending on
alignment requirements of the architecture._

For completeness, note that arrays of packed structures by default will also be
packed (no trailing padding inserted between array elements; example
[here](https://godbolt.org/z/hhzf9zW4E)). Usually the compiler will do the
correct thing, but you might run into unexpected cases when type aliasing (this
is undefined behavior anyway, and there be dragons here 🐉!).

**Bitfields** follow similar rules when it comes to packing, with the added
complexity where the type holding the bitfield is undefined, with this somewhat
horrifying language in the C11 specification §6.7.2.1/11:

> An implementation may allocate any addressable storage unit large enough to
> hold a bit-field. If enough space remains, a bit-field that immediately
> follows another bit-field in a structure shall be packed into adjacent bits of
> the same unit. If insufficient space remains, whether a bit-field that does
> not fit is put into the next unit or overlaps adjacent units is
> implementation-defined. The order of allocation of bit-fields within a unit
> (high-order to low-order or low-order to high-order) is
> implementation-defined. The alignment of the addressable storage unit is
> unspecified.

As Mitch Johnson over at theinterrupt.slack.com pointed out, there are other
subtleties to consider with bitfields that can have architecture-specific
implications:

> ... some architectures (ARM in particular) require compilers to represent
> volatile bitfield layout and accesses in a well-defined fashion in order to
> comply with their procedure call standard. This allows use of volatile
> bitfields to properly represent access to memory-mapped peripherals.
> https://developer.arm.com/documentation/ihi0042/j/?lang=en This can still be
> fraught with danger. GCC's had a number of bugs around volatile bitfield
> usage, and ARM's own clang derivative has had varyingly non-compliant behavior
> over time: https://developer.arm.com/documentation/ka004594/latest

## When does this matter?

Alright! Now that we've got a description of struct padding, let's describe some
cases where it makes a difference.

### Comparing padded structs

It's tempting to compare structs by using `memcmp`, as in the following:

```c
struct foo {
  uint32_t i;
  uint8_t b;
};

// Check 2 foos for equality
bool foo_are_equal(struct foo a, struct foo b) {
  const int result = memcmp(&a, &b, sizeof(a));
  return result == 0;
}

// Check if a foo matches a reference
bool foo_is_reference(struct foo a) {
  static const struct foo reference = {
    .i = 1234,
    .b = 56,
  };
  return foo_are_equal(a, reference);
}
```

HOWEVER, this may give incorrect results if the padding is not accounted for!

### Serializing structs outside the application

For example, writing a C struct into non-volatile storage:

```c
struct device_config {
  uint8_t device_config_version;
  uint64_t manufacture_date;
  uint8_t hardware_version;
  uint8_t serial_number[16];
};
```

If that data needs to be read by another piece of software, or if it potentially
will be migrated to a different struct layout (eg, a new field is added), it
might be prudent to pack the struct (either by hand or with
`__attribute__((packed))`), to simplify reasoning about the data structure.

### Security issues

The values in the padding space can potentially leak sensitive information if
the data structures are crossing trust boundaries. Specifically, the padding
space can contain data from objects that were previously allocated on the stack
(for example, an encryption key used to perform some cryptographic operation).

See the following articles for information on that subject:

- <https://lwn.net/Articles/417989/>
- <https://wiki.sei.cmu.edu/confluence/display/c/DCL39-C.+Avoid+information+leakage+when+passing+a+structure+across+a+trust+boundary>

### When it doesn't matter

If the structure is only ever accessed on a per-member basis, the padding
probably won't cause problems:

```c
struct foo {
  uint32_t i;
  uint8_t b;
};

// Check 2 foos for equality
bool foo_are_equal(struct foo a, struct foo b) {
  return (a.i == b.i) && (a.b == b.b);
}
```

Specifically, if the structure is only ever _internally_ used in the application
(never crosses a library or trust boundary, and is never serialized out to
external storage or over a communications interface), issues related to padding
may not be a problem.

Note that other languages may have different layout implementations for
composite types (see
[Rust](https://doc.rust-lang.org/1.59.0/reference/types/struct.html)), which may
complicate matters when moving raw C structs between different pieces of
software.

## Structure (Zero) Initialization

Given the above, it seems convenient to zero-initialize structures before using
them. With C99 or later, it is common to make use of the following patterns with
"designated initializers" for structure initialization:

```c
struct foo {
  uint32_t i;
  uint8_t b;
};

// Initialize members of 'a' to specific values. Members not specifically
// initialized will be initialized per the 'static storage duration'
// initialization rules (eg pointers go to NULL, integers go to 0, floats go to
// 0.0, etc)
struct foo a = {
  .i = 1,
  // .b will be set to 0
};

// Initialize 'b' to all zeros. This is a common idiom that specifies a '0'
// constant as the initial value for the first member of the structure, then
// relies on the above rule to initialize the rest of the structure.
struct foo b = { 0 };
```

This looks great! However, it's not obvious (from looking at those snippets)
what the value loaded into the padding region will be.

The unfortunate answer is: _it depends_

The C11 standard, chapter §6.2.6.1/6 says this:

> When a value is stored in an object of structure or union type, including in a
> member object, the bytes of the object representation that correspond to any
> padding bytes take unspecified values.

See also <https://stackoverflow.com/a/37642061>

Objects with '_static_ storage duration' (`static` keyword, or external linkage
(defined at the outermost scope in a compilation unit)), **padding bits will be
initialized to 0**!

Objects with '_automatic_ storage duration' (locally-scoped objects) have
**undefined behavior** when it comes to padding bit initialization!

This means that there is no constraint on what values are set to those bits when
the object is initialized.

## The Current State

Let's consider the following 4 zero-initialization strategies for this
structure:

```c
struct foo {
  uint32_t i;
  uint8_t b;
};
```

1. `memset` to zeros:

   ```c
   struct foo a;
   memset(&a, 0, sizeof(a));
   ```

2. individually set all members to 0:

   ```c
   struct foo a = {
     .i = 0,
     .b = 0,
   };
   ```

3. use `{ 0 }` zero-initializer

   ```c
   struct foo a = { 0 };
   ```

4. use `{}` GCC extension zero-initializer (_Note: this is quite
   poorly/non-documented for C - it IS valid C++ - but works in C on both GCC
   and clang. See [here](https://stackoverflow.com/a/54409687) and
   [here](https://gcc.gnu.org/legacy-ml/gcc/2019-07/msg00065.html)_)

   ```c
   struct foo a = {};
   ```

It turns out, the results for these vary between GCC and Clang and optimization
levels.

For the record, I'm testing using these compiler versions, on Ubuntu Linux 21.10
on 2022-02-28:

```bash
❯ clang --version
Ubuntu clang version 13.0.0-2

❯ gcc --version
gcc (Ubuntu 11.2.0-7ubuntu2) 11.2.0

# specific package versions:
❯ apt list clang gcc
Listing... Done
clang/impish,now 1:13.0-53~exp1 amd64 [installed]
gcc/impish,now 4:11.2.0-1ubuntu1 amd64 [installed]
```

Padding values under each strategy, optimization level, and compiler (_warning
boring tables below!_):

### Strategy 1, `memset`

| Strategy    | Optimization Level | Clang 13 | GCC 11 |
| ----------- | ------------------ | -------- | ------ |
| 1, `memset` | `0`                | zero     | zero   |
| 1, `memset` | `1`                | zero     | zero   |
| 1, `memset` | `2`                | zero     | zero   |
| 1, `memset` | `3`                | zero     | zero   |
| 1, `memset` | `s`                | zero     | zero   |

### Strategy 2, explicitly setting each struct member

| Strategy    | Optimization Level | Clang 13  | GCC 11    |
| ----------- | ------------------ | --------- | --------- |
| 2, explicit | `0`                | zero      | **unset** |
| 2, explicit | `1`                | **unset** | **unset** |
| 2, explicit | `2`                | zero      | **unset** |
| 2, explicit | `3`                | zero      | **unset** |
| 2, explicit | `s`                | **unset** | **unset** |

### Strategy 3, `{ 0 }`

| Strategy   | Optimization Level | Clang 13  | GCC 11 |
| ---------- | ------------------ | --------- | ------ |
| 3, `{ 0 }` | `0`                | zero      | zero   |
| 3, `{ 0 }` | `1`                | **unset** | zero   |
| 3, `{ 0 }` | `2`                | zero      | zero   |
| 3, `{ 0 }` | `3`                | zero      | zero   |
| 3, `{ 0 }` | `s`                | zero      | zero   |

### Strategy 4, `{ }` GCC extension

| Strategy | Optimization Level | Clang 13  | GCC 11 |
| -------- | ------------------ | --------- | ------ |
| 4, `{ }` | `0`                | zero      | zero   |
| 4, `{ }` | `1`                | **unset** | zero   |
| 4, `{ }` | `2`                | zero      | zero   |
| 4, `{ }` | `3`                | zero      | zero   |
| 4, `{ }` | `s`                | **unset** | zero   |

The main point is that it's not particularly consistent across compilers and
optimization levels 😱!

> You can find the example application used to generate the above data here on
> [Github](https://github.com/memfault/interrupt/tree/master/example/c-struct-padding-initialization/).

> I've also uploaded it to the wonderful Compiler Explorer if you want to take a
> look and quickly play around: <https://godbolt.org/z/b985G4ejT>

## Best Practice?

It's tricky to recommend a one-size-fits-all option here, because different
software will have different constraints. However, some general purpose advice
follows.

### Avoid relying on structure layout

The simplest option to avoid padding issues is to avoid the padding fields
altogether:

> Access data directly via each member, do not alias structures or use `memcmp`
> etc.

This only works if the padding data can be safely ignored in all use cases for
the data structures in question.

_Note however that packed structs can be safely `memcmp`'d, see below_

Another approach is to avoid structure holes entirely!

For example, you can use the `-Wpadded` compiler warning in
[GCC](https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#index-Wpadded) and
[Clang](https://releases.llvm.org/15.0.0/tools/clang/docs/DiagnosticsReference.html#wpadded) to detect
padding, and with `-Werror` or `-Werror=padded`, you can trigger compilation
errors if padding is detected. To address the warnings, you can add placeholders
to fill unused space:

```c
struct foo {
  uint32_t i;
  uint8_t b;
  uint8_t padding_[3];
};
```

(Note that GCC will emit a warning on _declaration_, where Clang will only warn
when the violating struct is actually used in a definition. Similar, but subtly
different as usual 🌈).

Alternatively, the `pahole` tool could be used to detect any structures with
padding bits (for example, as a linter pass on the generated binary), and they
can be corrected by either reordering structure members to eliminate padding, or
adding `uint8_t padding_[n]` fields to explicitly address the holes. See also
[The Lost Art of Structure Packing](http://www.catb.org/esr/structure-packing/)
linked previously.

It's generally preferable to strive for padding to only be present at end of
struct.

### Use `memset` to zero-initialize padding bits

`memset` reliably sets the entire memory space of an object, including the
padding bits.

Must be manually done, though, so can be error-prone.

Be sure to set the size argument directly from the object in question:

```c
// error prone! if the type of 'a' changes, we might get unexpected results
memset(&a, 0, sizeof(struct foo));

// much better
memset(&a, 0, sizeof(a));
```

### Last resort, `__attribute__((packed))`

This will eliminate structure padding, but there can be considerable compute
overhead (and with code doing unusual type aliasing, you may find yourself in an
Unaligned Access fault 😓).

On the plus side, the structure fields should have easily predictable offsets in
memory, for example if it needs to be serialized out.

Additionally, packed structs can be safely `memcmp`'d, since there are no
"ghost" bits hiding in between explicitly allocated members 👻

## The Future 🌞

I gave this away
[right at the start of the article](https://thephd.dev/ever-closer-c23-improvements#consistent-warningless-and-intuitive-initialization-with--),
but somewhat unusually for C, there is change coming on this topic!

The proposed change for C23 is that the `= {}` (functionally equivalent to
`= { 0 }` on modern compilers) _will also initialize padding bits to 0_ 😎.

You can see the gory details in the following links:

- <http://www.open-std.org/jtc1/sc22/wg14/www/docs/n2900.htm>
- <https://github.com/ThePhD/future_cxx/issues/37>

This seems like a nice update to the standard that doesn't appear to impact
backwards-compatibility and just makes things better!

## Update 2022-05-19

GCC 12 has added a new flag, `-ftrivial-auto-var-init=choice` which enables
zero-initialization of struct padding by the compiler:

<https://gcc.gnu.org/onlinedocs/gcc-12.2.0/gcc/Optimize-Options.html#index-ftrivial-auto-var-init>

This flag has been present in Clang for a while, but it seems like it may be
removed in the future; it requires setting this somewhat amusingly named flag to
use:

```plaintext
-enable-trivial-auto-var-init-zero-knowing-it-will-be-removed-from-clang
```

The flag is documented
[here](https://releases.llvm.org/14.0.0/tools/clang/docs/ClangCommandLineReference.html#cmdoption-clang-enable-trivial-auto-var-init-zero-knowing-it-will-be-removed-from-clang).

This flag definitely incurs runtime penalties, so it might make sense to enable
it when compiling some sensitive subset of your application (eg a security
library). Definitely an interesting compiler feature!

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->

- <https://web.archive.org/web/20181230041359if_/http://www.open-std.org/jtc1/sc22/wg14/www/abq/c17_updated_proposed_fdis.pdf>
  C17 specification
- <https://thephd.dev/ever-closer-c23-improvements> Roundup of some upcoming C23
  improvements
- <https://linux.die.net/man/1/pahole> The `pahole` tool
- <http://www.catb.org/esr/structure-packing/> The Lost Art of Structure Packing
- <https://lwn.net/Articles/417989/> Structure holes and information leaks
- <https://wiki.sei.cmu.edu/confluence/display/c/DCL39-C.+Avoid+information+leakage+when+passing+a+structure+across+a+trust+boundary>
  Avoid information leakage when passing a structure across a trust boundary
- <https://stackoverflow.com/a/37642061> Discussion on zero-initializing C
structure padding
<!-- prettier-ignore-end -->
