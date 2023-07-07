---
title: "Rust for Low Power Digital Signal Processing"
description: "A step by step guide on how to use Rust for Digital Signal
Processing (DSP) in a firmware project"
author: cyril
image: /img/rust-dsp/rust-dsp-code.png
tags: [rust]
---

Like many firmware developers, I've been curious about the potential Rust can
have in the embedded space. After reading James Munns' [great
article]({% post_url 2019-12-17-zero-to-main-rust-1 %}), I decided to
find a project I could use Rust for to learn.

We're still in the early days of Rust on embedded. Given all the chipset SDKs
provided in C, the dominance of K&R's language is assured for years to come. I
believe Rust will creep into projects in the form of libraries that get included
into firmware.

One area I quickly honed in on is DSP programming. The current tooling and
process in that area is very poor, and I suspected that Rust could help.
[I had asked the reddit r/embedded community](https://www.reddit.com/r/embedded/comments/cmoiea/best_setup_to_develop_signal_processing_algorithm/)
what their favorite setup was like. The most upvoted answer surprised me: folks
were using Matlab for development, then translating their algorithms to C code
based on the ubiquitous
[CMSIS-DSP](http://www.keil.com/pack/doc/CMSIS/DSP/html/index.html) library.

<!-- excerpt start -->

In this post, I go over how Rust can be used to implement DSP algorithms for
firmware today, and compare the process and performance to the equivalent code
    written with CMSIS-DSP.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Building a Rust library

Before anything, make sure you have installed Rust on your system. There are
great instructions on
[the Rust website](https://www.rust-lang.org/tools/install) so I won't repeat
them here. You should have `rustc` and `cargo` executables on your path.

For the sake of this example, I'm using the nRF52832 microcontroller on a
PCA10040 development board.

Our goal here isn't to write a full firmware, but instead to write a library in
Rust which can be called from our C firmware. This allows us to side-step the
BSP and other pieces often provided in C by suppliers.

There are good instructions on how to do this online[^2], I've summarized them
below.

First, we use the `cargo` package manager to create a new project. Cargo is the
default for Rust packages, which are usually called "crates". We can pass
Cargo the `--lib` flag to specify that the project is a library.

```bash
$ cargo new dsp_rs --lib
    Created library `dsp_rs` package

$ ls dsp_rs
    Cargo.toml src
```

Our project contains source code under `src` as well as a `Cargo.toml` file.
This is the main project configuration file used by Cargo. We will need to
modify this file slightly:

1. Since we want the library to be static rather than dynamic, we must indicate
   it under `crate-type`.
2. We will add a dependency on another crate, `panic-halt`, which we use as the
   default panic behavior for our project. `panic`[^1] is one of the few functions
   required by the Rust runtime on a given platform. Conveniently, `panic-halt` has
   an implementation we can use.

Here's our `Cargo.toml`:

```toml
[lib]
name = "dsp_rs"
crate-type = ["staticlib"] # Creates static lib

# Here, I specify the dependency only for thumbv7em-none-eabihf arch
[target.thumbv7em-none-eabihf.dependencies]
panic-halt = "0.2.0"
```

Now that that's out of the way, we can start implementing our library.

### A simple example

Let's start with a simple algorithm:

Given an array of 200 floating point numbers:
1. scale it to a dynamic factor
2. offset the resulting array using an arbitrary number
3. get the average of that new array

While this is a bit artificial, it relies on three operations (scale, offset,
mean) which are used heavily in all of my DSP projects.

Here is a simple implementation of this algorithm using CMSIS-DSP and C:

```c
#define ARBITRARY_OFFSET   ((float32_t) 1.98765432)

static float32_t dsp_process_c(float32_t scaling_factor,
                               float32_t * array,
                               size_t size)
{
    float32_t mean = 0;
    arm_scale_f32(array, scaling_factor, array, TEST_ARRAY_SIZE);
    arm_offset_f32(array, ARBITRARY_OFFSET, array, TEST_ARRAY_SIZE);
    arm_mean_f32(array, TEST_ARRAY_SIZE, &mean);
    return mean;
}
```

#### The Rust implementation

As a C programmer, I was tempted to write the Rust code using an imperative
style. Instead, I decided to leverage Rust's syntax for functional programming,
as it maps neatly to digital signal processing concepts. While this is beyond
the scope of this article, I found that using a functional rather than
itterative approach yielded a ~2x improvement in performance in this case.

We filled in `src/lib.rs`, the file cargo auto-generated for us. It creates a
function `dsp_process_rs` which we will expose to our C code.

```rust
/* Standard library not supported in our firmware project */
#![no_std]
#![no_builtins]

extern crate panic_halt;

/* Needed to perform the division */
use core::ops::Div;

#[no_mangle]
pub extern "C" fn dsp_process_rs(scaling: f32,
                                 ptr: *const f32,
                                 size: usize) -> f32
{
    /* Create a slice out of the C array using pointer to first element */
    let array;
    unsafe {
        array = core::slice::from_raw_parts(ptr, size);
    }

    let mean_func = array.iter()
        .map(|x| *x * scaling)
        .map(|x| x + 1.98765432)
        .fold(0_f32, |sum, x| sum + x)
        .div(array.len() as f32);

    mean_func
}
```

A few things to note:

1. Per James' article, we disable the standard library and builtins via `no_std`
   and `no_builtins`.
2. We must mark our function as `pub` to export it, `extern "C"` to use the C
   ABI, and `no_mangle` to make sure its name does not get mangled by the Rust
   compiler (like it would with C++).

Now that we have the Rust function written, we need to compile the crate.

### Compiling the library

This is where the Rust tooling really starts to shine. More than a package
manager, Cargo is also a build system. To compile our library, all we need to do
is call `cargo build` and specify the correct target architecture. Like most
compilers, `rustc` will otherwise default to compiling a program for the host
architecture.

```bash
$ cargo build --target thumbv7em-none-eabihf

	Compiling panic-halt v0.2.0
	Compiling dsp_rs v0.1.0 (/Users/charlottemaurer/Documents/Cyril/rust/dsp_rs)
	Finished dev [unoptimized + debuginfo] target(s) in 0.62s
```

You can see that Cargo has pulled all the dependencies, compiled them, compiled
our program, and generated debug & program output. In the directory
`target/thumbv7em-none-eabihf/debug/`, we can find our static library:
`libdsp_rs.a`. Impressive!

>  Tip: To compile our code for the same target every time, we can create the
>  file `.cargo/config` in the root directory and add the following lines:
> ```
> [build]
> # Pick ONE of these compilation targets by uncommenting the corresponding line:
> # target = "thumbv6m-none-eabi"    # Cortex-M0 and Cortex-M0+
> # target = "thumbv7m-none-eabi"    # Cortex-M3
> # target = "thumbv7em-none-eabi"   # Cortex-M4 and Cortex-M7 (no FPU)
> # target = "thumbv7em-none-eabihf" # Cortex-M4F and Cortex-M7F (with FPU)
> ```

### Calling Rust from C

We now have an archive ready to be added to our nRF52 test program. But how do
we call functions implemented in Rust from C?

In C, we need to have at least the declaration of the function in a header file
and its definition in a compilation unit. Here, the function is taken from the
library file. Since we've already got our definition in Rust, all we need is a
function declaration in a header file.

Since we have a single function, I wrote a header file by hand. The embedded
Rust book[^2] has a better option for us:

> There is a tool to automate this process, called
> [cbindgen](https://github.com/eqrion/cbindgen) which analyses your Rust code
> and then generates headers for your C and C++ projects from it.

Below is our header file, easy right?

```c
#ifndef HELLO_WORLD_DSP_RS_H
#define HELLO_WORLD_DSP_RS_H

#include "arm_math.h"

float32_t dsp_process_rs(float32_t scaling,
                         float32_t const * array,
                         size_t size);

#endif //HELLO_WORLD_DSP_RS_H
```

We can now call this function from C file as if it were another C API!

Below is a snippet of the `main.c` file we use to run both our C and Rust
implementations.

```c
#include "cmsis_dsp_rs.h"
/* ... */

#define TEST_ARRAY_SIZE		200
static float32_t array_from[TEST_ARRAY_SIZE] = {...};

#define SCALING_FACTOR    (1.78478f)

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    /* Copy needed in another array when calling the C function
     * not to change the original data
     */
    float32_t array_copy[TEST_ARRAY_SIZE];
    arm_copy_f32(array_from, array_copy, TEST_ARRAY_SIZE);

    /* Call Rust function */
    float32_t mean_rs = dsp_process_rs((float32_t) SCALING_FACTOR, array_from, TEST_ARRAY_SIZE);

    /* Call C function */
    float32_t mean_c  = dsp_process_c((float32_t) SCALING_FACTOR, array_copy, TEST_ARRAY_SIZE);

    /* Make sure results are the same */
    APP_ERROR_CHECK_BOOL(mean_rs == mean_c);

    while(1);
}
```

### Linking it all into a program

We have two steps to do before being able to link our C program with our Rust
library.

First, we need to make sure that the path to the header file is specified in the
Makefile.

Second, we have to add both `libdsp_rs.a` and the CMSIS-DSP lib to be linked.
The `.a` file is copy-pasted in the nRF52 project while the CMSIS-DSP file can
be found in the nRF-SDK:

```make
LIB_FILES += \
  $(PROJ_DIR)/lib/libdsp_rs.a \
  $(SDK_ROOT)/components/toolchain/cmsis/dsp/GCC/libarm_cortexM4lf_math.a
```

We can now compile and flash our test program on the PCA10040 board and check if
we did everything right. On first look, things are promising: both averages
computed using Rust and C are the same. Let's find out about performance...

## How did Rust do?

In order to test performance, I added time tracking around the call to
`dsp_process_rs` and `dsp_process_c`. Thus, we can measure the time spent in
each function using the low frequency clock on the nRF52 (using the `app_timer`
module). I also added a loop to run the test a hundred times to make sure the
results can be repeated. Here is what it looks like:

```c
uint32_t tick      = app_timer_cnt_get();

uint32_t mean      = dsp_process_rs((float32_t) i, array_from, TEST_ARRAY_SIZE);

uint32_t tock      = app_timer_cnt_get();
uint32_t diff_rust = app_timer_cnt_diff_compute(tock, tick);
```

Here are the results with the default build configuration, "unoptimized debug
mode":

- Function written using Rust: **49 ticks**
- Function written using C: **2 ticks**

So Rust is 25 times slower. That doesn't bode well!

### Optimizing Rust

Like most compilers, `rustc` has multiple optimization levels[^3].

#### Speed

Here, the C project has been compiled using GCC's `-03` optimization level which
turns on all optimizations for performance, no matter the code size or compilation time[^4].

As for Rust, we have been compiling in debug mode, which disables all optimizations. One simple
improvement we can make is to simply build the library with the `--release` flag.

```bash
$ cargo build --target thumbv7em-none-eabihf --release
```

> Do not forget to copy the resulting `.a` file over to your project, and to call
> (`make clean`) if there is no modification to the code because the Makefile
> won't detect that the lib file has changed (at least the simple Makefile I'm
> using).

Let's look at the results:

- Function written using Rust: **1 tick! 😮⚡️**
- Function written using C: **2 ticks**

It's getting **really** interesting. I now have a quicker program using Rust! 👏

Using the low frequency clock doesn't yield accurate results regarding the
actual efficiency of both implementation. In order to get more accurate results,
we need a better timer. A good option on Cortex-M4 is to use the Cycle Count
register[^6]. Reading it before and after calling our function will tell us how
many CPU cycles have been spent executing it.

Function written using Rust: **2 209** instructions counted

Function written using C: **3 930** instructions counted

The Rust function is about **1.8 times faster** than the CMSIS-DSP
implementation.

#### Code size

Firmware is often constrained by code size just as much as it is by performance.
We must make sure that Rust performance does not come at an unreasonable code
size cost.

To compare code size, we compile 4 binaries:
- one calling C function and Rust function using the Debug library
- one calling C function and Rust function using the Release library
- one calling only our Rust function,
- one calling only our C function.

The table below summarizes the results:

| Functions called          | Binary size | Comments                                         |
| ------------------------- | ----------- | ------------------------------------------------ |
| Both C and Rust (debug)   | 18.712kB    | Our baseline                                     |
| Both C and Rust (release) | 16.456kB    | A ~2kB improvement with release optimizations    |
| C only                    | 16.024kB    | The Rust library takes 432 bytes                 |
| Rust only (release)       | 15.944kB    | The C library takes 512 bytes,                   |

Far from being unreasonable, there is in fact no real code size tradeoff between
C & Rust for our example. This is exciting!

## Where Rust shines

As you probably know: Rust can be compiled for different architectures (x86,
ARM, etc). And as people who have been trying to develop DSP algorithms on
embedded targets know: it's hard to test those algorithms on a massive amount of
data, directly on the target. Some solutions include using QEMU with semihosting
to fetch data from the host and pass it to the algorithms, or using Matlab to
develop the algorithms and later generate C code.

Rust and Cargo make it trivially easy to run our algorithm in different
environments. Our program can be developed on an x86 host, against large corpus
of data and with good debugging tools, then be easily recompiled for our
Cortex-M4 target. Let's check this out.

> Note on floating point portability: Despite IEEE standardization, you may find
> that different architectures produce different results for the same floating
> point calculations. There are
> [interesting](https://gafferongames.com/post/floating_point_determinism/)
> [discussions](https://news.ycombinator.com/item?id=9837342) on this topic
> online. Long story short, expect small variations in the results across
> platforms.

### Developing cross-platform algorithms on PC with Cargo

Having developed a processing crate that fits our needs, we can easily integrate
that crate into another Rust project. For example, let's create a simple Rust
program we can run on our PC.

```bash
$ cargo new hello_world --bin
    Created binary (application) `hello_world` package
```

Modifying `Cargo.toml` to use our DSP crate:

```
[dependencies]
dsp_rs = { path = "../dsp_rs" }
```

We want the `crate-type` to be `rlib`. Unfortunately, while I'm writing those
lines, there is no way to specify a crate type depending on target at the
moment[^5], so let's change it in `Cargo.toml`:

```toml
[lib]
name = "dsp_rs"
crate-type = ["rlib"]
```

And making use of our crate in `main.rs`, for example:

```rust
extern crate dsp_rs;
use dsp_rs::*;

fn main() {
    let array_to_process = [...];

    let mean = dsp_process_rs(1.324_f32, array_to_process.as_ptr(), array_to_process.len());

    println!("Average: {}", mean);
}
```

Compiling and running that example takes milliseconds.

The power of Rust programs used in a CLI[^7] environment, along with the ease to
develop crates for different targets, is a strong advantage for Rust. Coupled
with its standard library, Rust is able to talk to machines and fetch data from
servers quite easily, to test your algorithm against a corpus hosted online.

### Rust has modern language features

While Algorithm designers today prefer Matlab or Python, I believe Rust strikes
a great balance between expressivity and performance which may allow it to win
in the end.

This starts with the syntax itself: Rust offers functional programming idioms,
option types, powerful macros, and a powerful type system. This will be familiar
to software engineers who may find C arcane and difficult to use.

Moreover, the tooling around Rust is fantastic. `cargo` is a solid build system
and package manager and `rustfmt` helps format your code. By bundling these
tools with the language, Rust has enabled a vibrant ecosystem to spring. Rather
than reinvent the wheel, Rust programmers rely on third party libraries
throughout their projects.

Many of the safety features implemented in the compiler make
Rust a much more forgiving language than C. Rather than crashes, Rust developers
are faced with verbose error messages emitted at compile time.

Last but not least, Rust is free and open-source! Between the low cost,
expressive syntax, solid ecosystem, and the fact that it does not need to be
translated to C, Rust is a real alternative to Matlab for algorithm development.

## Closing

We have seen that Rust keeps its promises regarding efficiency and portability,
making it a great choice to develop DSP algorithms. Getting rid of CMSIS-DSP in
order to enjoy the productivity gains brought with Rust is an exciting prospect.

Have you built digital signal processing pipelines in Rust? I'd love to hear
your experience in the comments!

{% include newsletter.html %}

{:.no_toc}

## References

[^1]: [Rust and the panic behavior]({% post_url 2019-12-17-zero-to-main-rust-1 %}#something-just-for-rust)
[^2]: [A little Rust with your C](https://rust-embedded.github.io/book/interoperability/rust-with-c.html)
[^3]: [Optimize Rust for speed](https://rust-embedded.github.io/book/unsorted/speed-vs-size.html#optimize-for-speed)
[^4]: [Code Size Optimization: GCC Compiler Flags]({% post_url 2019-08-20-code-size-optimization-gcc-flags %}#changing-the-optimization-level)
[^5]: [Cycle Count register](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0439b/BABJFFGJ.html)
[^6]: [Issue: Ability to set crate-type depending on target](https://github.com/rust-lang/cargo/issues/4881)
[^7]: [Command-line apps with Rust](https://www.rust-lang.org/what/cli)
