---
title: "From Zero to main(): Bare metal Rust"
description: "Bringing an embedded Rust application from zero to main"
author: james
tags: [zero-to-main, rust, bare-metal]
---

For the past thirty years or so, the choice of languages for embedded systems developers has been relatively slim. Languages like C++ and Ada have found a home in some niche areas, such as telecommunications and safety critical fields, while higher level languages like Lua, Python, and JavaScript have found a home for scripting and prototyping.

Despite these options, most developers working on bare metal systems have been using the same two languages as long as I can remember: Assembly and C.

But not for no reason! Languages often make trade-offs to fit the needs of the developers working with them: an interpreter to allow for rapid iteration, a heap for ease of memory management, exceptions for simplifying control flow, etc. These trade-offs usually come with a price: whether it is code size, RAM usage, low level control, power usage, latency, or determinism.

<!-- excerpt start -->

Since 2015, Rust has been redefining what it means to combine the best-in-class aspects of performance, correctness, and developer convenience into one language, without compromise. In this post, we'll bootstrap a Rust environment on a Cortex-M microcontroller from scratch, and explain a few of the language concepts you might not have seen before.

<!-- excerpt end -->

As a compiled systems language (based on LLVM), it is also capable of reaching down to the lowest levels of embedded programming as well, without losing built-in features that feel more at home in higher level languages, like a package manager, helpful compile time diagnostics, correctness through powerful static analysis, or documentation tooling.

This post is meant as a complement to the original [Zero to main()] post on the Interrupt blog, and will elide some of the explanations of hardware level concepts. If you're new to embedded development, or haven't seen that post, go read it now!

[Zero to Main()]: {% post_url 2019-05-14-zero-to-main-1 %}

## Setting the stage

As with the original post, most of the concepts and code presented in this series should work for all Cortex-M series MCUs, though these examples target the nRF52 processor by Nordic. This is a Cortex-M4F chip found on several affordable development boards.

Specifically, we are using:

* Decawave's [DWM1001-DEV] as our development board
* The built-in JLink capabilities of the board
* Segger's JLinkGDBServer for programming

[DWM1001-DEV]: https://www.decawave.com/product/dwm1001-development-board/

Software wise, we will be using:

* The 1.39.0 version of Rust, though any stable version 1.31.0 or newer should work
* We'll also use some of the `arm-none-eabi` binutils, such as `arm-none-eabi-gdb` and `arm-none-eabi-objdump`, which are compatible with the binaries produced by Rust

We'll also be implementing a simple blinking LED application. The full Rust source used for this blog post is available [here, on GitHub]. This is what the source code looks like for our application:

[here, on GitHub]: https://github.com/ferrous-systems/zero-to-main/blob/master/from-scratch/src/main.rs

```rust
#![no_std]
#![no_main]

use nrf52::gpio::{Level, Pins};

fn main() -> ! {
    let gpios = Pins::take();
    let mut led = gpios.p0_31;

    led.set_push_pull_output(Level::Low);

    loop {
        led.set_high();
        delay(2_000_000);

        led.set_low();
        delay(6_000_000);
    }
}
```

## Power on!

Let's build our Rust application, and see what the binary contains:

```text
cargo build --release
   Compiling from-scratch v0.1.0 (/home/james/memfault/blog-1/examples/from-scratch)
    Finished release [optimized] target(s) in 0.62s

arm-none-eabi-objcopy -O binary target/thumbv7em-none-eabihf/release/from-scratch  target/thumbv7em-none-eabihf/release/from-scratch.bin

xxd target/thumbv7em-none-eabihf/release/from-scratch.bin | head -n 5
00000000: 0000 0120 dd00 0000 0000 0000 0000 0000  ... ............
00000010: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000020: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000030: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000040: 0000 0000 0000 0000 0000 0000 0000 0000  ................
```

Reading this, our initial stack pointer is `0x20010000`, and our start address pointer is `0x000000dd`. Let's see what symbol is there. We will also pass `-C` to objdump, which will demangle our symbols (we'll explain demangling a bit more later):

```
arm-none-eabi-objdump -Ct target/thumbv7em-none-eabihf/release/from-scratch | sort
...
00000004 g     O .vector_table  00000004 __RESET_VECTOR
000000dc g       .vector_table  00000000 _stext
000000dc l     F .text  0000005c from_scratch::reset_handler
00000138 l     F .text  0000006a from_scratch::main
000001a4 g       *ABS*  00000000 __sidata
...
```

Same as in the original post, our compiler has set the lowest bit of our reset handler to one to indicate a thumb2 function, so `from_scratch::reset_handler` is what we're looking for.

## Writing a Reset_Handler

The Cortex-M processor on our board doesn't actually know the difference between C and Rust, so our responsibilities for starting the firmware are the same when we build a Rust program, we need to:

1. Provide an entry point, stored in the second word of flash
2. Zero-initialize the `.bss` section
3. Set items with static storage duration to their initial values. In Rust the destination (in RAM) is referred to as `.data`, and the source values (in Flash) is referred to as `.rodata`

Let's go through the code required to write this reset handler, one chunk at a time.

### Starting at the top

The first line in many embedded applications and libraries in Rust will look like this:

```rust
#![no_std]
```

This is called a "global attribute", and it is stating that this Rust code will not be using the Rust Standard Library. Attributes in Rust are sometimes used similarly to `#pragma` in C, in that they can change certain behaviors of the compiler.

Rust provides a number of built-in library components, but the two main ones are:

1. The Rust Standard Library
2. The Rust Core Library

While the Standard Library contains a number of useful components, such as data structures, and interfaces for opening files and sockets, it generally requires your target to have these things! For bare metal applications, we can instead forego this library and only use the Rust Core Library, which does not have these requirements.

Rust as a language has a concept of "modules", which can be used to organize and provide namespaces for your code. Libraries or applications in Rust are called "Crates", and each has its own namespace. This is why we saw the symbol `from_scratch::reset_handler` in our linker script: It was referring to the `reset_handler` function in the `from_scratch` crate (which is the application crate in this example).

To use items from another crate, including the [`core` library], you can import these items in using the `use` syntax:

[`core` library]: https://doc.rust-lang.org/core/index.html

```rust
use core::{
    mem::zeroed,
    panic::PanicInfo,
    ptr::{read, write_volatile},
};
```

This imports the symbols into the current context so that they can be used. Most symbols in Rust are not available in a global namespace, which helps to avoid naming collisions.

However in some cases, it is important to have a globally defined symbol. As part of the ABI of the Cortex-M platform, we need to provide the address of the reset handler in a very specific location. Let's look at how we do that in Rust:

### Setting the Reset Vector

```rust
#[link_section = ".vector_table.reset_vector"]
#[no_mangle]
pub static __RESET_VECTOR: fn() -> ! = reset_handler;
```

Let's unpack that from the bottom up:

```rust
pub static __RESET_VECTOR: fn() -> ! = reset_handler;
```

This line defines a symbol called `__RESET_VECTOR` at static scope. The type of this symbol is `fn() -> !`, which is a pointer to a function that takes no arguments and that never returns, or that "diverges". The value of this symbol is `reset_handler`, which is the name of a function in our program. Functions are a first class items in Rust (similar to Python), so we can use the names of functions as a value that represents a function pointer.

```rust
#[no_mangle]
```

This is another attribute, like our `#![no_std]`. By starting with `#[` instead of `#![`, we can tell this is a local attribute instead of a global attribute, which means it only applies to the next item, instead of the whole module.

The `#[no_mangle]` attribute tells the compiler **not** to mangle this symbol, so it will show up as `__RESET_VECTOR`, not `from_scratch::__RESET_VECTOR`. [Name mangling] is a technique used by languages like C++ and Rust to emit unique names for things like functions, generic data type parameters, or symbols for data at static scope, no matter where or how often they are used in the resulting binary.

[Name mangling]: https://en.wikipedia.org/wiki/Name_mangling

```rust
#[link_section = ".vector_table.reset_vector"]
```

This is another attribute that is informing the compiler to place this symbol in the `.vector_table.reset_vector` section of our linker script, which will place it right where we need it. This is similar to gcc's `__attribute__((section(...)))`.

### The Reset Handler, for real

Now let's look at our actual reset handler, from top to bottom:

```rust
pub fn reset_handler() -> ! {
    extern "C" {
        // These symbols come from `linker.ld`
        static mut __sbss: u32; // Start of .bss section
        static mut __ebss: u32; // End of .bss section
        static mut __sdata: u32; // Start of .data section
        static mut __edata: u32; // End of .data section
        static __sidata: u32; // Start of .rodata section
    }

    // Initialize (Zero) BSS
    unsafe {
        let mut sbss: *mut u32 = &mut __sbss;
        let ebss: *mut u32 = &mut __ebss;

        while sbss < ebss {
            write_volatile(sbss, zeroed());
            sbss = sbss.offset(1);
        }
    }

    // Initialize Data
    unsafe {
        let mut sdata: *mut u32 = &mut __sdata;
        let edata: *mut u32 = &mut __edata;
        let mut sidata: *const u32 = &__sidata;

        while sdata < edata {
            write_volatile(sdata, read(sidata));
            sdata = sdata.offset(1);
            sidata = sidata.offset(1);
        }
    }

    // Call user's main function
    main()
}
```

Phew! That was a lot at once, especially if you aren't familiar with Rust! Let's break that down one chunk at a time to explain the concepts in a little more detail:

### Defining a function in Rust

```rust
pub fn reset_handler() -> ! {
```

This defines a function that is public, named `reset_handler`, that takes no arguments `()`, and that never returns `-> !`.

In Rust, functions normally either don't return a value like this:

```rust
/// Returns nothing
fn foo() { /* ... */ }
```

Or do return a value like this:

```rust
/// Returns a 32-bit unsigned integer
fn bar() -> u32 { /* ... */ }
```

The `!` type, called the "Never type", means that this function will never return, or diverges. Since our reset handler never will return (where would it go?) we can tell Rust this, which may allow it to make certain optimizations at compile time.

### A little help from the linker

```rust
extern "C" {
    // These symbols come from `linker.ld`
    static mut __sbss: u32; // Start of .bss section
    static mut __ebss: u32; // End of .bss section
    static mut __sdata: u32; // Start of .data section
    static mut __edata: u32; // End of .data section
    static __sidata: u32; // Start of .rodata section
}
```

This section defines a number of static symbols which will be provided by our linker, namely the start and end of the sections that are important for our reset handler to know about. These symbols are defined in an `extern "C"` scope, which means two things:

1. They will be provided sometime later, by another piece of code, or in this case, the linker itself
2. They will be defined using the "C" style of ABI and naming conventions, which means they are implicitly `#[no_mangle]`

Some of these symbols are also declared as `mut`, or "mutable". By default in Rust, all variables are immutable, or read-only. To make a variable mutable in Rust, you must explicitly mark it as `mut`. This is the opposite of languages like C and C++, where variables are by default mutable, and must be marked with `const` to prevent them from being modified.

### Zeroing the BSS section

```rust
// Initialize (Zero) BSS
unsafe {
    let mut sbss: *mut u32 = &mut __sbss;
    let ebss: *mut u32 = &mut __ebss;

    while sbss < ebss {
        write_volatile(sbss, zeroed());
        sbss = sbss.offset(1);
    }
}
```

As a language, Rust makes some pretty strong guarantees around memory safety, correctness, and freedom from Undefined Behavior. However, when working directly with the hardware, which has no knowledge of Rust's guarantees, it is necessary to work in Rust's `unsafe` mode, which allows some additional behaviors, but requires the developer to uphold certain correctness guarantees manually.

Rust has two ways of referring to data by reference:

1. References
2. Raw Pointers

In most Rust code, we use references, which can be statically guaranteed for correctness and memory safety. However in this case, we are given the raw integers, which we want to treat as pointers.

In this code, we take a mutable reference to the `__sbss` and `__ebss` symbols provided by the linker, and convert these Rust references into raw pointers.

We then use these pointers to make volatile writes of zero across the range, one 32-bit word at a time.

This section zeros our entire `.bss` section, as defined by the linker.

### Initializing static data

```rust
// Initialize Data
unsafe {
    let mut sdata: *mut u32 = &mut __sdata;
    let edata: *mut u32 = &mut __edata;
    let mut sidata: *const u32 = &__sidata;

    while sdata < edata {
        write_volatile(sdata, read(sidata));
        sdata = sdata.offset(1);
        sidata = sidata.offset(1);
    }
}
```

This section of code initializes our `.data` section, copying directly from the `.rodata` section. This is similar to the code above, however we also walk the pointer in the initializer section as well as the pointer in the destination section.

### Ready for launch

Finally, at the end of our reset handler, we get to call main!

```rust
// Call user's main function
main()
```

Since the main function we defined above is also divergent (`fn main() -> !`), we can simply call the function. If we had called a non-divergent function, we would get a compile error here!

### Something just for Rust

Rust does have one additional requirement for a bare metal program: You must define the panic handler.

```rust
/// This function is called on panic.
#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    // On a panic, loop forever
    loop {
        continue;
    }
}
```

Rust has a concept of a `panic`, which is like failing an assert in C. This happens when the program has hit an unrecoverable error case, and must be stopped in some way.

Unlike Exceptions in C++, panics are usually not designed to be recovered from gracefully, and therefore do not require the overhead necessary to unwind.

Still, we must define a "panic handler" in case our program ever panics. For this example, we go into an endless loop, though you could choose to do something different, like logging the error to flash, or soft-rebooting the system immediately.

### Programming without Compromise

Earlier I mentioned that Rust brings convenience without compromise. To demonstrate this, let's take a quick look at the size and total contents of our code once we compile for `opt-level = "s"`, which is equivalent to `-Os` in C or C++:

```
arm-none-eabi-size target/thumbv7em-none-eabihf/release/from-scratch
   text    data     bss     dec     hex filename
    420       0       8     428     1ac target/thumbv7em-none-eabihf/release/from-scratch

arm-none-eabi-nm -nSC target/thumbv7em-none-eabihf/release/from-scratch
00000004 00000004 R __RESET_VECTOR
00000008 R __reset_vector
000000dc R _stext
000000dc 0000005c t from_scratch::reset_handler
00000138 0000006a t from_scratch::main
000001a4 T __erodata
000001a4 T __etext
000001a4 A __sidata
20000000 T __edata
20000000 B __sbss
20000000 T __sdata
20000000 00000004 b from_scratch::delay::DUMMY
20000004 00000001 b from_scratch::nrf52::gpio::Pins::take::TAKEN
20000008 B __ebss
20000008 B __sheap
20010000 A _stack_start
```

This is 420 bytes of `.text`, which boils down to 220 bytes for the vector table, and 198 bytes of actual code.

## Wrapping up

Although we spent this post talking about how to write support for scratch in Rust, we almost never need to actually do this in practice!

Instead we can leverage Cargo, the package manager and build system for Rust, to use existing libraries that support Cortex-M and Nordic components, a board support crate that handles configuration for our specific development board, and provide a panic handler.

These libraries include an initialization runtime with pre-init hooks, a template linker script that can be modified and extended, access to common Cortex-M components like the NVIC, and more, without having to copy and paste boilerplate reference code into our project.

Now we instead end up with a program that looks like this:

```rust
#![no_std]
#![no_main]

// Panic provider crate
use panic_reset as _;

// Provides definitions for our development board
use dwm1001::{
    cortex_m_rt::entry,
    nrf52832_hal::prelude::*,
    DWM1001
};

#[entry]
fn main() -> ! {
    // Set up the board, initializing the LEDs on the board
    let mut board = DWM1001::take().unwrap();

    // Obtain a microsecond precision timer
    let mut timer = board.TIMER0.constrain();

    loop {
        // board.leds.D10 - Bottom LED BLUE
        board.leds.D10.enable();
        timer.delay(2_000_000);

        board.leds.D10.disable();
        timer.delay(6_000_000);
    }
}
```

All of the code used in this blog post is available [on GitHub]. If you're looking for information on how to get started with embedded Rust, check out the [Embedded Working Group]'s [bookshelf] for documentation on how to install Rust, connect to your device, and build and run your first application.

[on GitHub]: https://github.com/ferrous-systems/zero-to-main
[Embedded Working Group]: https://github.com/rust-embedded/wg
[bookshelf]: https://docs.rust-embedded.org/

In future posts we'll talk about how libraries like [`r0`], [`cortex-m`], and [`cortex-m-rt`] provide common functionality when writing embedded programs, and how libraries like [`nrf52832-pac`], [`nrf52832-hal`], and [`dwm1001`] provide compile-time safe abstractions over hardware interfaces!

[`r0`]: https://docs.rs/r0
[`cortex-m`]: https://docs.rs/cortex-m
[`cortex-m-rt`]: https://docs.rs/cortex-m-rt
[`nrf52832-pac`]: https://docs.rs/nrf52832-pac
[`nrf52832-hal`]: https://docs.rs/nrf52832-hal
[`dwm1001`]: https://docs.rs/dwm1001
