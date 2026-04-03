---
title: "The Hidden Cost of Misalignment"
description:
  "Most embedded developers use #pragma pack(1) to create portable binary record
  formats. But pack(1) forces the compiler to byte-decompose every field access
  -- even perfectly aligned fields -- resulting in 7x more operations on RISC-V,
  ARM, and Xtensa. This post shows the problem, the fix (packed + aligned), and
  how to lint struct alignment as part of your development workflow."
author: chrismerck
tags: [c, toolchain, performance]
---

Let's suppose you're building an even smarter
[fishtank]({% post_url 2024-07-24-choosing-or-building-an-embedded-db %}).
You're adding temperature and salinity sensors, logging timestamped readings to
flash. The struct _is_ your binary record format -- every field at a fixed byte
offset, so you can read it back on any system that knows the layout.

You use fixed-width types from `stdint.h` and `#pragma pack(1)` to strip out
compiler-inserted padding. This is the advice I had always received and given,
and it's correct -- as far as it goes.

But `pack(1)` costs more than you think. It **destroys the compiler's ability to
generate efficient code for _every_ field access**, including fields that are
perfectly aligned.

On RISC-V, a single 32-bit store to an aligned field in a `pack(1)` struct
compiles to 7 instructions instead of 1!

<!-- excerpt start -->

In this post, we show how to fix using the `packed` and `aligned` attributes,
and how to avoid byte-decomposition even as the struct grows in the future.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## A Sensor Record

Here's a struct you might find in any embedded codebase -- a timestamped sensor
reading destined for flash storage or transmission over the wire:

```c
#pragma pack(push, 1)
typedef struct {
    int64_t  timestamp;       // offset 0,  8 bytes
    int32_t  temperature_mc;  // offset 8,  4 bytes (millidegrees C)
    int32_t  salinity_ppt;    // offset 12, 4 bytes (parts per thousand)
    uint8_t  status_flags;    // offset 16, 1 byte
    uint16_t battery_mv;      // offset 17, 2 bytes
} sensor_reading_t;           // 19 bytes, no padding
#pragma pack(pop)
```

The structure is packed to preserve offsets between platforms (so the same C
code can read it out on a PC, for example), and secondarily to eliminate padding
bytes that would waste space. This is reasonable and very common in embedded
code.

> **Why pack structs at all?** Without packing, the compiler inserts padding
> bytes between fields to satisfy alignment requirements. Historically, the size
> and placement of that padding differed quite a bit across
> architectures[^endianness] -- modern compilers are somewhat more consistent,
> but the padding bytes still waste space and can surprise you when designing a
> wire or storage format. Packing removes this instability: you get a fixed,
> deterministic byte layout regardless of where the code is compiled.

Here's what that looks like. The unpacked version of this struct is 24 bytes --
5 bytes of hidden padding:

<img src="{% img_url packed-aligned/padding-waste.svg %}" />

But you will see that we labeled most of the entries in the packed structure as
"byte-decomposed", even for fields that are aligned relative to the start of the
struct! This is a hidden performance bomb.

## What the Compiler Actually Emits

Look at `temperature_mc` at offset 8. It's a 4-byte integer sitting at a
4-byte-aligned offset. One would think the compiler would emit a single store
instruction for this, but take a look at what actually happens.

Here's the RISC-V disassembly (compiled with `riscv64-elf-gcc` at `-O2`):

```asm
;; entry->temperature_mc = value;
;; value in a1, entry pointer in a0

srli    a3, a1, 0x8       ; extract byte 1
srli    a4, a1, 0x10      ; extract byte 2
srli    a5, a1, 0x18      ; extract byte 3
sb      a1, 8(a0)         ; store byte 0
sb      a3, 9(a0)         ; store byte 1
sb      a4, 10(a0)        ; store byte 2
sb      a5, 11(a0)        ; store byte 3
```

Seven ops! Three shifts and 4 byte-stores -- to write a single `int32_t` to a
perfectly aligned offset. Without `pack(1)`:

```asm
sw      a1, 8(a0)         ; one op
```

Every field in the struct gets this treatment. Writing the 64-bit `timestamp` at
offset 0 is even worse -- 14 ops instead of 2. Not just the misaligned fields.
All of them.

> **Why this happens.** `#pragma pack(1)` sets the _type's_ maximum alignment
> to 1. That means a pointer to this struct could legally point to any byte
> address. The compiler can't look at your code and determine that in practice,
> these structs always live in `malloc`'d memory (aligned), on the stack
> (aligned), or in larger aligned structures. The type system says alignment is
> 1, so every access gets byte-decomposed. This isn't a bug. You told the
> compiler "trust nothing about this struct's alignment." The compiler took you
> at your word.

## The Fix: `packed` + `aligned`

GCC and Clang support combining `__attribute__((packed))` with
`__attribute__((aligned(N)))`. The `packed` attribute controls internal layout
(no padding between fields), while `aligned` sets the struct's own alignment
requirement:

```c
typedef struct __attribute__((packed, aligned(4))) {
    int64_t  timestamp;       // offset 0  -- native access
    int32_t  temperature_mc;  // offset 8  -- native access
    int32_t  salinity_ppt;    // offset 12 -- native access
    uint8_t  status_flags;    // offset 16 -- byte access either way
    uint16_t battery_mv;      // offset 17 -- still byte-decomposed
} sensor_reading_t;           // 20 bytes (19 content + 1 byte tail padding)
```

The field offsets are identical. The struct gains 1 byte of tail padding (so its
size is a multiple of 4), but the binary layout of the fields themselves doesn't
change. What changes is what the compiler knows: the base pointer is always
4-byte aligned.

That lets the compiler do the math:

- `timestamp` at offset 0: 4-aligned base + 0 = aligned. **Native store (2
  ops).**[^aligned8]
- `temperature_mc` at offset 8: 4-aligned + 8 = aligned. **Native store (1
  op).**
- `salinity_ppt` at offset 12: 4-aligned + 12 = aligned. **Native store.**
- `status_flags` at offset 16: byte access regardless. **No change.**
- `battery_mv` at offset 17: 4-aligned + 17 is odd. **Still byte-decomposed** --
  correctly.

<img src="{% img_url packed-aligned/field-access.svg %}" />

The genuinely misaligned field (`battery_mv` at offset 17) is still handled
safely.[^bonus] But the aligned fields -- which are the majority of this struct
-- go from 7 ops to 1.

> **When you can't switch: arrays of packed structs.** If your packed struct is
> already used in a contiguous array -- in flash, in a file format, over the
> wire -- switching to `packed, aligned(4)` changes the array stride. Elements
> that were at offsets 0, 19, 38, 57... would now be spaced 20 bytes apart. This
> breaks binary compatibility with existing data.
>
> For existing formats with arrays of packed structs, you're stuck with
> `pack(1)` unless you can migrate the data. For _new_ formats, use
> `packed, aligned(4)` from the start.

Here's what that looks like in practice. With `pack(1)`, elements pack tightly
at 19-byte intervals. With `packed, aligned(4)`, each element is padded to 20
bytes[^stride] so the next element starts at a 4-byte-aligned offset:

<img src="{% img_url packed-aligned/array-stride.svg %}" />

## Struct Evolution

Your struct is well-designed. The aligned fields get native access, the
misaligned ones are handled correctly. You ship it.

Six months later, someone extends the struct in a pull request. They append a
`uint32_t error_code`:

```c
typedef struct __attribute__((packed, aligned(4))) {
    int64_t  timestamp;
    int32_t  temperature_mc;
    int32_t  salinity_ppt;
    uint8_t  status_flags;
    uint16_t battery_mv;
    uint32_t error_code;      // NEW -- offset 19
} sensor_reading_t;
```

The struct was 19 bytes of content before, so `error_code` lands at offset 19 --
not 4-aligned. No compiler warning. No test failure. The code works fine. But
that field gets the full byte-decomposition treatment:

```asm
;; entry->error_code = code;
srli    a3, a1, 0x8
srli    a4, a1, 0x10
srli    a5, a1, 0x18
sb      a1, 19(a0)
sb      a3, 20(a0)
sb      a4, 21(a0)
sb      a5, 22(a0)
```

Seven ops for what should be one.

<img src="{% img_url packed-aligned/struct-evolution.svg %}" />

> **The bigger risk.** _Inserting_ fields into a packed struct breaks binary
> compatibility with anything already using the old layout -- devices in the
> field, stored records, protocol peers. Appending fields is safe (old readers
> simply ignore the extra bytes), but insertion shifts every subsequent offset.
> I recommend using defense in depth where CI rejects field insertion (by
> comparing struct layouts against a baseline) _and_ integration tests decode
> packets or records from old firmware versions. That's a topic for a future
> post.

The fix for this particular struct would be to insert explicit padding before
`error_code` so it lands at offset 20:

```c
    uint16_t battery_mv;
    uint8_t  _reserved;       // explicit padding to maintain alignment
    uint32_t error_code;      // now at offset 20 -- native access
```

This is easy to miss in code review, whether human or automated. This is why you
lint.

> **Catching misalignment with struct-lint.**
> [struct-lint](https://github.com/chrismerck/struct-lint) reads DWARF debug
> info from ELF binaries and reports fields that aren't naturally aligned within
> packed structs:
>
> ```
> $ struct-lint firmware.elf
>
> sensor_reading_evolved_t.battery_mv (uint16_t, 2 bytes) at offset 17 not naturally aligned (needs 2)
> sensor_reading_evolved_t.error_code (uint32_t, 4 bytes) at offset 19 not naturally aligned (needs 4)
>
> 2 issues in 1 structs across 1 file (2 alignment, 0 missing pack)
> ```
>
> It catches exactly the kind of silent misalignment that happens when structs
> evolve over time. Run it against your build artifacts and you'll know which
> fields are paying the byte-decomposition penalty.
>
> ---
>
> There is also a tool called [pahole](https://linux.die.net/man/1/pahole) which
> can inspect structs based on the DWARF info. Pahole has been around for almost
> 20 years and is even integrated into the Linux kernel build system. Output
> from `pahole` for the same example `sensor_reading_evolved_t` struct is:
>
> ```c
> type`def struct {
>       int64_t                    timestamp;            /*     0     8 */
>       int32_t                    temperature_mc;       /*     8     4 */
>       int32_t                    salinity_ppt;         /*    12     4 */
>       uint8_t                    status_flags;         /*    16     1 */
>       uint16_t                   battery_mv;           /*    17     2 */
>       uint32_t                   error_code;           /*    19     4 */
>
>       /* size: 24, cachelines: 1, members: 6 */
>       /* padding: 1 */
>       /* last cacheline: 24 bytes */
> } sensor_reading_evolved_t __attribute__((__aligned__(4)));>
> ```
>
> where you can see the offsets and sizes of each member of the structure. This
> output may be helpful for debugging and manual inspection of structures.
> `pahole` also offers flags where it can recommend restructuring for
> performance. `struct-lint` on the other hand is intended as a linter for your
> CI pipeline, and to provide feedback on stdout that looks like compiler errors
> so that it's maximally actionable for devs and coding agents.

## Not Just RISC-V

The examples above are RISC-V, but this affects all embedded architectures.
Here's the same 32-bit write (`temperature_mc` at offset 8) across seven
targets:

| Architecture             | `pack(1)`                            | `packed, aligned(4)` | Ratio |
| ------------------------ | ------------------------------------ | -------------------- | ----- |
| **RISC-V 32** (rv32imac) | 7 ops (3 `srli` + 4 `sb`)            | 1 op (`sw`)          | 7x    |
| **Xtensa** (ESP32)       | 7 ops (3 `extui` + 4 `s8i`)          | 1 op (`s32i.n`)      | 7x    |
| **ARM Cortex-M0**        | 7 ops (2 `lsrs` + `lsls` + 4 `strb`) | 1 op (`str`)         | 7x    |
| **MIPS32**               | 2 ops (`swl` + `swr`)                | 1 op (`sw`)          | 2x    |
| **macOS arm64**          | 1 op (`str`)                         | 1 op (`str`)         | 1x    |
| **i686**                 | 1 op (`movl`)                        | 1 op (`movl`)        | 1x    |
| **x86_64**               | 1 op (`movl`)                        | 1 op (`movl`)        | 1x    |

Most embedded targets we tested show the same 7× byte-decomposition overhead --
different ISAs, different compilers, same pattern. The exception is MIPS32,
which has dedicated unaligned store instructions (`swl` / `swr`) that reduce the
overhead to 2×.

Desktop architectures (arm64, x86) produce identical code regardless of packing.
They handle unaligned access in hardware and may pay a microarchitectural
penalty (cache line split, extra cycles) -- but thankfully on embedded targets
we can just inspect the disassembly to know exactly what we're paying.

## Conclusion

`#pragma pack(1)` is a blunt instrument. It tells the compiler "nothing about
this struct's alignment can be trusted." In most embedded codebases, that's far
more pessimistic than reality -- you packed the struct to get a stable binary
layout for serialization, not because you actually access it through misaligned
pointers.

Use `__attribute__((packed, aligned(4)))` to say what you actually mean: no
padding between fields, but the struct itself lives at an aligned address. Your
code gets smaller and faster, and the compiler still correctly byte-decomposes
the fields that genuinely need it.

As your structs evolve, alignment can silently degrade. A new field appended at
the wrong offset, and you're back to 7x the ops with no warning from the
compiler. Lint it like you lint everything else.

Individually, these are small wins -- single-digit percent improvements in code
size and throughput. But embedded mastery is the accumulation of these details.
Getting the small things right so the compiler can do its job is how you build
systems that are fast, small, and correct.

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->

[^endianness]:
    Endianness is another portability concern when designing binary formats --
    see the [portability
    aside]({% post_url 2024-07-24-choosing-or-building-an-embedded-db %}#aside-portability-of-records)
    in the embedded database post. Here we'll focus on what `pack(1)` does to
    your generated code.

[^aligned8]:
    On a 32-bit ISA there is no single 64-bit store instruction, so two 32-bit
    stores is already optimal and `aligned(4)` is sufficient. If you target a
    64-bit core (e.g. RV64, AArch64) and want a single `sd` or `str`, use
    `aligned(8)`.

[^stride]:
    A small bonus: array indexing is faster with a 20-byte stride than a 19-byte
    stride. Multiplying by 20 decomposes into simple shifts and adds, while
    multiplying by 19 requires an extra subtraction. The difference is minor but
    it's free.

[^bonus]:
    There's a bonus for misaligned reads, too. With `pack(1)`, reading
    `battery_mv` requires two byte-loads and a reassembly (4 ops). With
    `packed, aligned(4)`, the compiler knows offset 16 is 4-aligned, so it does
    a single word-load from there and shifts out the uint16_t (3 ops). Knowing
    where aligned boundaries are lets the compiler use wider loads even for
    misaligned fields.

- [struct-lint: DWARF-based struct alignment linter](https://github.com/chrismerck/struct-lint)
- [GCC Type Attributes: packed, aligned](https://gcc.gnu.org/onlinedocs/gcc/Common-Type-Attributes.html)
- [ARM Cortex-M0 Technical Reference Manual: Unaligned Access](https://developer.arm.com/documentation/ddi0432/c/programmers-model/unaligned-access-support)
<!-- prettier-ignore-end -->
