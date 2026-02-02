---
title: "The Hidden Cost of #pragma pack(1) on Embedded Targets"
description:
  "Most embedded developers use #pragma pack(1) to create portable binary
  record formats. But pack(1) forces the compiler to byte-decompose every
  field access -- even perfectly aligned fields -- resulting in 7x more
  instructions on RISC-V, ARM, and Xtensa. This post shows the problem,
  the fix (packed + aligned), and how to lint struct alignment as part of
  your development workflow."
author: chrismerck
tags: [c, toolchain, performance]
---

You're building a sensor that logs readings to flash. Temperature, salinity,
a timestamp, some status flags. The struct *is* your binary record format --
every field at a fixed byte offset, so you can read it back on any system
that knows the layout.

You use fixed-width types from `stdint.h` and `#pragma pack(1)` to strip out
compiler-inserted padding. This is the standard advice, and it's correct --
as far as it goes.

<!-- excerpt start -->

But `pack(1)` costs more than you think. It doesn't just control layout -- it
destroys the compiler's ability to generate efficient code for *every* field
access, including fields that are perfectly aligned. On RISC-V, a single
32-bit store to an aligned field in a `pack(1)` struct compiles to 7
instructions instead of 1. The fix is straightforward: use
`__attribute__((packed, aligned(4)))` instead. But keeping your structs
well-aligned as they evolve over time -- that's the harder part.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## A Sensor Record

Here's a struct you might find in any embedded codebase -- a timestamped
sensor reading destined for flash storage or transmission over the wire:

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

Nineteen bytes per reading, fields at predictable offsets. Exactly what you
wanted.

> **Why pack structs at all?** Without packing, the compiler inserts padding
> bytes between fields to satisfy alignment requirements. The size and
> placement of that padding depends on the target architecture -- the same
> struct compiled for a 32-bit MCU may have a different layout than on a
> 64-bit server parsing the same data. Packing removes this instability:
> you get a fixed, deterministic byte layout regardless of where the code
> is compiled.

Here's what that looks like. The unpacked version of this struct is 24 bytes --
5 bytes of padding that shift depending on the platform:

<img src="{% img_url packed-aligned/padding-waste.svg %}" />

Endianness is another portability concern when designing binary formats, but
that's a topic for another post. Here we'll focus on what `pack(1)` does to
your generated code.

## What the Compiler Actually Emits

Look at `temperature_mc` at offset 8. It's a 4-byte integer sitting at a
4-byte-aligned offset. The compiler should emit a single store instruction
for this.

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

Seven instructions -- 3 shifts and 4 byte-stores -- to write a single
`int32_t` to a perfectly aligned offset.

Without `pack(1)`:

```asm
sw      a1, 8(a0)         ; one instruction
```

Writing the `int64_t timestamp` at offset 0 is worse. That's 14 instructions
-- 6 shifts and 8 byte-stores -- instead of 2:

```asm
;; entry->timestamp = ts;
;; ts split across a1 (low) and a2 (high), entry pointer in a0

srli    t1, a1, 0x8       ; extract low byte 1
srli    a7, a1, 0x10      ; extract low byte 2
srli    a6, a1, 0x18      ; extract low byte 3
srli    a3, a2, 0x8       ; extract high byte 1
srli    a4, a2, 0x10      ; extract high byte 2
srli    a5, a2, 0x18      ; extract high byte 3
sb      a1, 0(a0)         ; store low byte 0
sb      a2, 4(a0)         ; store high byte 0
sb      t1, 1(a0)         ; store low byte 1
sb      a7, 2(a0)         ; store low byte 2
sb      a6, 3(a0)         ; store low byte 3
sb      a3, 5(a0)         ; store high byte 1
sb      a4, 6(a0)         ; store high byte 2
sb      a5, 7(a0)         ; store high byte 3
```

Every field in the struct gets this treatment. Not just the misaligned ones.
All of them.

## Why This Happens

`#pragma pack(1)` sets the *type's* maximum alignment to 1. That means a
pointer to this struct could legally point to any byte address. The compiler
can't look at your code and determine that in practice, these structs always
live in `malloc`'d memory (aligned), on the stack (aligned), or in larger
aligned structures. The type system says alignment is 1, so every access gets
byte-decomposed.

This isn't a bug. You told the compiler "trust nothing about this struct's
alignment." The compiler took you at your word.

## The Fix: `packed` + `aligned`

GCC and Clang support combining `__attribute__((packed))` with
`__attribute__((aligned(N)))`. The `packed` attribute controls internal
layout (no padding between fields), while `aligned` sets the struct's own
alignment requirement:

```c
typedef struct __attribute__((packed, aligned(4))) {
    int64_t  timestamp;       // offset 0  -- native access
    int32_t  temperature_mc;  // offset 8  -- native access
    int32_t  salinity_ppt;    // offset 12 -- native access
    uint8_t  status_flags;    // offset 16 -- byte access either way
    uint16_t battery_mv;      // offset 17 -- still byte-decomposed
} sensor_reading_t;           // 20 bytes (19 content + 1 byte tail padding)
```

The field offsets are identical. The struct gains 1 byte of tail padding
(so its size is a multiple of 4), but the binary layout of the fields
themselves doesn't change. What changes is what the compiler knows: the
base pointer is always 4-byte aligned.

That lets the compiler do the math:

- `timestamp` at offset 0: 4-aligned base + 0 = aligned. **Native store (2 instructions).**
- `temperature_mc` at offset 8: 4-aligned + 8 = aligned. **Native store (1 instruction).**
- `salinity_ppt` at offset 12: 4-aligned + 12 = aligned. **Native store.**
- `status_flags` at offset 16: byte access regardless. **No change.**
- `battery_mv` at offset 17: 4-aligned + 17 is odd. **Still byte-decomposed** -- correctly.

<img src="{% img_url packed-aligned/field-access.svg %}" />

The genuinely misaligned field (`battery_mv` at offset 17) is still handled
safely. But the aligned fields -- which are the majority of this struct -- go
from 7 instructions to 1.

There's a bonus, too. When *reading* the misaligned `battery_mv`, the
compiler gets creative. With `pack(1)`, it does two byte-loads:

```asm
;; pack(1): read battery_mv at offset 17
lbu     a5, 18(a0)        ; load high byte
lbu     a0, 17(a0)        ; load low byte
```

With `packed, aligned(4)`, the compiler knows offset 16 is 4-aligned, so it
loads a full 32-bit word from there and lets the caller extract the bytes it
needs:

```asm
;; packed+aligned(4): read battery_mv at offset 17
lw      a0, 16(a0)        ; load full word from aligned offset 16
```

One instruction instead of two. The compiler can find a nearby aligned
address and use a wider load because it knows where aligned boundaries are.

> **When you can't switch: arrays of packed structs.** If your packed struct
> is already used in a contiguous array -- in flash, in a file format, over
> the wire -- switching to `packed, aligned(4)` changes the array stride.
> Elements that were at offsets 0, 19, 38, 57... would now be spaced 20
> bytes apart. This breaks binary compatibility with existing data.
>
> For existing formats with arrays of packed structs, you're stuck with
> `pack(1)` unless you can migrate the data. For *new* formats, use
> `packed, aligned(4)` from the start.

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

The struct was 19 bytes of content before, so `error_code` lands at offset
19 -- not 4-aligned. No compiler warning. No test failure. The code works
fine. But that field gets the full byte-decomposition treatment:

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

Seven instructions for what should be one.

<img src="{% img_url packed-aligned/struct-evolution.svg %}" />

> **The bigger risk.** Appending fields to a packed struct also breaks binary
> compatibility with anything already using the old layout -- devices in the
> field, stored records, protocol peers. Catching that requires saved test
> fixtures from previous format versions. But catching the *alignment* issue
> at PR time is a cheap first line of defense.

The fix for this particular struct would be to insert explicit padding before
`error_code` so it lands at offset 20:

```c
    uint16_t battery_mv;
    uint8_t  _reserved;       // explicit padding to maintain alignment
    uint32_t error_code;      // now at offset 20 -- native access
```

But nobody thinks about that in a typical code review. The diff looks
innocuous -- just one new field. This is why you lint.

## Catching Misalignment with struct-lint

[struct-lint](https://github.com/chrismerck/struct-lint) reads DWARF debug
info from ELF binaries and reports fields that aren't naturally aligned
within packed structs:

```
$ struct-lint firmware.elf

sensor_reading_t.battery_mv (uint16_t, 2 bytes) at offset 17 not naturally aligned (needs 2)
sensor_reading_t.error_code (uint32_t, 4 bytes) at offset 19 not naturally aligned (needs 4)

2 issues in 1 struct across 1 file (2 alignment, 0 missing pack)
```

It catches exactly the kind of silent misalignment that happens when structs
evolve over time. Run it against your build artifacts and you'll know which
fields are paying the byte-decomposition penalty.

## Measured Results

Here's the full comparison across packing strategies, compiled for RISC-V
32-bit at `-O2`:

| Packing strategy | Write `int64_t` @ 0 | Write `int32_t` @ 8 | Write `uint16_t` @ 17 | Read `uint16_t` @ 17 |
|---|---|---|---|---|
| `pack(1)` | 14 insns | 7 insns | 3 insns | 2 insns |
| `packed, aligned(4)` | 2 insns | 1 insn | 3 insns | 1 insn |
| Unpacked | 2 insns | 1 insn | 1 insn (offset 18) | 1 insn (offset 18) |

For the aligned fields (`timestamp`, `temperature_mc`), `packed, aligned(4)`
matches the unpacked performance exactly. For the genuinely misaligned field
(`battery_mv` at offset 17), writes are the same as `pack(1)`, but reads are
actually *better* -- the compiler exploits the known base alignment to use a
wider aligned load.

## This Isn't Platform-Specific

We've shown RISC-V, but the same pattern appears on every architecture where
unaligned access is expensive. Here's the same 32-bit write
(`temperature_mc` at offset 8) across three architectures:

| Architecture | `pack(1)` | `packed, aligned(4)` | Ratio |
|---|---|---|---|
| **RISC-V 32** (rv32imac) | 7 insns (3 `srli` + 4 `sb`) | 1 insn (`sw`) | 7x |
| **Xtensa** (ESP32) | 7 insns (3 `extui` + 4 `s8i`) | 1 insn (`s32i.n`) | 7x |
| **ARM Cortex-M0** | 8 insns (3 `lsrs` + `lsls` + 4 `strb`) | 1 insn (`str`) | 8x |

Different instruction sets, same pattern. 7-8x more instructions for a
32-bit store to a perfectly aligned offset. The compiler has no choice --
when alignment is 1, every store must be byte-decomposed.

## Conclusion

`#pragma pack(1)` is a blunt instrument. It tells the compiler "nothing about
this struct's alignment can be trusted." In most embedded codebases, that's
far more pessimistic than reality -- you packed the struct to get a stable
binary layout for serialization, not because you actually access it through
misaligned pointers.

Use `__attribute__((packed, aligned(4)))` to say what you actually mean: no
padding between fields, but the struct itself lives at an aligned address.
Your code gets smaller and faster, and the compiler still correctly
byte-decomposes the fields that genuinely need it.

As your structs evolve, alignment can silently degrade. A new field appended
at the wrong offset, and you're back to 7x the instructions with no warning
from the compiler. Lint it like you lint everything else.

Individually, these are small wins -- single-digit percent improvements in
code size and throughput. But embedded mastery is the accumulation of these
details. Getting the small things right so the compiler can do its job is how
you build systems that are fast, small, and correct.

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
- [struct-lint: DWARF-based struct alignment linter](https://github.com/chrismerck/struct-lint)
- [GCC Type Attributes: packed, aligned](https://gcc.gnu.org/onlinedocs/gcc/Common-Type-Attributes.html)
- [ARM Cortex-M0 Technical Reference Manual: Unaligned Access](https://developer.arm.com/documentation/ddi0432/c/programmers-model/unaligned-access-support)
<!-- prettier-ignore-end -->
