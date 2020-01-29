---
title: "Rust for Digital Signal Processing"
description: "A DSP library written in Rust brings many advantages"
author: cyril
---

As you may know, the Rust programming language is gaining traction among firmware developers. James wrote [a great article](https://interrupt.memfault.com/blog/zero-to-main-rust-1) to help people start from scratch with Rust on embedded devices. 

Rust-based firmware won't be released into production in a day. This is mostly due to the SDK provided with ARM Cortex-M microcontrollers, which are all based on C/C++ and developers have been used for long time to write their whole program in C/C++. Although Rust is quite new, it deserves to be considered for production code and without a doubt, regarding the legacy code running, Rust will come in production code as external libraries first. 

One of the domain where Rust can be highly beneficial for the development process is in Digital Signal Processing (DSP). Developing for DSP on embedded targets has always been a pain in the ass. [I asked the reddit r/embedded community](https://www.reddit.com/r/embedded/comments/cmoiea/best_setup_to_develop_signal_processing_algorithm/) how they achieve to have a great development environment. The most upvoted answer was using Matlab and translating it to C code, which itself uses the well-known library developed by ARM for signal processing : [CMSIS-DSP](http://www.keil.com/pack/doc/CMSIS/DSP/html/index.html). To me, that library has always been the basics to develop algorithms for signal processing, as it is intended to be. I have to admit that my question was implying making use of CMSIS-DSP because I thought dropping that library, made explicitly for Cortex-M microcontrollers, would always lead to decreased performance.

<!-- excerpt start -->
We will see how Rust increases productivity when building a library called from legacy C code, and most importantly how it compares against the well-known CMSIS-DSP library.
<!-- excerpt end -->

{:.no_toc}

## Table of Contents

<!-- prettier-ignore -->
* auto-gen TOC:
{:toc}

## Creating the library

I assume [you have installed Rust](https://www.rust-lang.org/tools/install) and its components `rustc`, `cargo`, etc.

The goal here is to write a library that can be statically linked to my main C program. Creating a library in Rust proved to be quite easy[^2]. For the sake of that example, I'm going to use the nRF52832 microcontroller on a PCA10040 board. 

```bash
# Let's create a new module with cargo
# Notice how we can specify that we are creating a library using "--lib"
$ cargo new dsp_rs --lib
	Created library `dsp_rs` package

$ ls dsp_rs
  Cargo.toml src
```

`Cargo.toml` is the main file to configure our project. 

In our case, we want the library to be statically linked, which will create a `.a` file. To do so, the `crate-type` must be `staticlib`.

We also need a default panic behavior[^1] to be implemented. We will borrow one published as a crate: `panic-halt`.

```
[lib]
name = "dsp_rs"
crate-type = ["staticlib"] # Creates static lib

# Here, I specify the dependency only for thumbv7em-none-eabihf arch
[target.thumbv7em-none-eabihf.dependencies]
panic-halt = "0.2.0"
```

Now let's dive into `src/` where you can find the source file: `lib.rs`. That's where the magic will happen.

### A simple example

In order to keep everything as simple as possible, here is the algorithm that we are going to implement:

- use an array of 200 floating point numbers
- scale it to a dynamic factor
- offset the resulting array using an arbitrary number
- get the average of that new array

Those 3 functions (scale, offset, mean) taken individually are the most used for the projects I have been working for.
Here are the simple steps implemented in C using CMSIS-DSP:

```c
#define ARBITRARY_OFFSET   ((float32_t) 1.98765432)

static float32_t dsp_process_c(float32_t scaling_factor, float32_t * array, size_t size)
{
    float32_t mean = 0;
    
    arm_scale_f32(array, scaling_factor, array, TEST_ARRAY_SIZE);

    arm_offset_f32(array, ARBITRARY_OFFSET, array, TEST_ARRAY_SIZE);

    arm_mean_f32(array, TEST_ARRAY_SIZE, &mean);

    return mean;
}
```

#### The Rust implementation

I have to admit that I started by implementing the function using imperative programming: iterating into the array to add an offset for example. But, Rust has the advantage to have influences from *functional programming*, which will be used to see how it behaves against CMSIS-DSP. For your information and because I find it interesting: using imperative programming was at least 2 times slower for that example. 

Here is the equivalent of the C function, along with some attributes to make it work on our target that **cannot** use `std`.

```rust
#![no_std]
#![no_builtins]

#[cfg(target_arch = "arm")]
extern crate panic_halt;

/* Needed to perform the division */
use core::ops::Div;

#[no_mangle]
pub extern "C" fn dsp_process_rs(scaling: f32, ptr: *const f32, size: usize) -> f32 {
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

Now that we have the Rust function written, we need to compile the crate.

### Compile the library

We want to compile that library for our nRF52. In our case, we didn't specified anywhere the default target to be used so we will now specify it if we want to compile for Cortex-M4F core using the `--target` flag:

```bash
$ cargo build --target thumbv7em-none-eabihf

	Compiling panic-halt v0.2.0
	Compiling dsp_rs v0.1.0 (/Users/charlottemaurer/Documents/Cyril/rust/dsp_rs)
	Finished dev [unoptimized + debuginfo] target(s) in 0.62s
```

---

Tips: If you are wondering how to compile that library for the same target every time, create the file `.cargo/config` in the root directory, add the lines below while uncommenting the target you want:

```
[build]
# Pick ONE of these compilation targets by uncommenting the corresponding line:
# target = "thumbv6m-none-eabi"    # Cortex-M0 and Cortex-M0+
# target = "thumbv7m-none-eabi"    # Cortex-M3
# target = "thumbv7em-none-eabi"   # Cortex-M4 and Cortex-M7 (no FPU)
# target = "thumbv7em-none-eabihf" # Cortex-M4F and Cortex-M7F (with FPU)
```

---

Now, checking in the directory `target/thumbv7em-none-eabihf/debug/` , you will find `libdsp_rs.a`. 

### Call Rust from C

We now have the archive ready to be added to our nRF52 test program. But how do we call functions implemented in Rust from C? 

In C, we need to have at least the signature of the functions in a header file and the function declaration in a compilation unit. Here, the function is taken from the library file. 

In our case, we have only one function signature, which is pretty simple so I wrote it by hand. But, as specified on the embedded Rust book[^2]:

> There is a tool to automate this process, called [cbindgen](https://github.com/eqrion/cbindgen) which analyses your Rust code and then generates headers for your C and C++ projects from it.

Below is our header file, easy right?

```c
#ifndef HELLO_WORLD_DSP_RS_H
#define HELLO_WORLD_DSP_RS_H

#include "arm_math.h"

float32_t dsp_process_rs(float32_t scaling, float32_t const * array, size_t size);

#endif //HELLO_WORLD_DSP_RS_H
```

Below is a snippet of the actual `main.c` file used for that example, with the interesting parts:

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

### Link

We have two steps to do before being able to link our C program with our Rust library.

First, we need to make sure that the path to the header file is specified in the Makefile.

Second, we have to add both `libdsp_rs.a` and the CMSIS-DSP lib to be linked. The `.a` file is copy-pasted in the nRF52 project while the CMSIS-DSP file can be found in the nRF-SDK:

```
LIB_FILES += \
  $(PROJ_DIR)/lib/libdsp_rs.a \
  $(SDK_ROOT)/components/toolchain/cmsis/dsp/GCC/libarm_cortexM4lf_math.a 
```

We can now compile and flash our test program on the PCA10040 board and check if we did everything well. The first good thing is that both averages computed using Rust and C are the same. We then need to make sure that Rust doesn't spend too much time executing that function. 

## Is Rust really efficient?

In order to test extensively the C code against the Rust code, my first idea has been to add time tracking around the call to `dsp_process_rs` and `dsp_process_c`. Thus, we can measure the time spent in each function using the low frequency clock on the nRF52 (`app_timer` module). I added a loop to make the test a hundred times to make sure the results can be repeated. I will report the worst case scenario here. Below are the steps to measure the time spent in the function:

```c
uint32_t tick      = app_timer_cnt_get();

uint32_t mean      = dsp_process_rs((float32_t) i, array_from, TEST_ARRAY_SIZE);

uint32_t tock      = app_timer_cnt_get();
uint32_t diff_rust = app_timer_cnt_diff_compute(tock, tick);
```

Here are the results, using the default optimization, made for debugging the program (see the last line when compiling: "dev [unoptimized + debuginfo]"):

- Function written using Rust: **49 ticks**
- Function written using C: **2 ticks**

Around 25 times slower using Rust... It's a lot and doesn't seem to be a good indication for the future...

### Rust optimizations

Now, as in many programming languages, it is possible with Rust to optimize for speed[^3]. One basic optimization is to simply build the library with the `--release` flag. 

```bash
$ cargo build --target thumbv7em-none-eabihf --release
```

Once done, do not forget to copy-paste the `.a` file and clean the project (`make clean`) if there is no modification to the code because the Makefile won't detect that the lib file has changed (at least the simple Makefile I'm using). We should expect to gain some efficiency, let's see:

- Function written using Rust: **1 tick! 😮⚡️**
- Function written using C: **2 ticks**

It's getting **really** interesting. I now have a quicker program using Rust! 👏 

Using the low frequency clock doesn't yield accurate results regarding the actual efficiency of both implementation. In order to go further, I checked the Cycle Count register[^4] that can be read while debugging, before and after calling the functions:

Function written using Rust: **2 209** instructions counted

Function written using C: **3 930** instructions counted

The Rust function is about **1.8 times faster** than the CMSIS-DSP implementation. 

##   Rust portability, the real advantage

As you probably know: Rust can be compiled for different architectures: x86, ARM, etc. And as people who have been trying to develop DSP algorithms on embedded targets know: it's hard to test those algorithms on a massive amount of data, directly on the target. Some solutions include using Qemu with semihosting to fetch data from the host and pass it to the algorithms, or using Matlab to develop the algorithms and later generate C code. 

There are interesting discussions around the web, like [this one](https://news.ycombinator.com/item?id=9837342) that brings various elements to the table regarding correctness of floating point arithmetic. To make it short, we have to accept that results will be different, considering an error `epsilon`. Depending on the use case, `epsilon` can be more or less significant but we won't go into that discussion for now and admit that the error is low enough to consider that our processing algorithm yields "correct" results on any modern platform.

The least we can say is that progress has been made with the introduction of the IEEE-754 standard that brings the same representation accross languages and architectures for floating-point numbers. Rust and C are using that standard when using `f32`/`float` or `f64`/`double`.

Let's see how we can make use of the portability of Rust.

### Get into speed using your host computer

Having developed a processing crate that fits our needs, we can easily integrate that crate into another Rust project, using a few steps. Let's create that application:

```bash
$ cargo new hello_world --bin
    Created binary (application) `hello_world` package
```

Modifying `Cargo.toml` to use our DSP crate:

```
[dependencies]
dsp_rs = { path = "../dsp_rs" }
```

We want the `crate-type` to be `rlib`. Unfortunately, while I'm writing those lines, there is no way to specify a crate type depending on target at the moment[^6], so let's change it in `Cargo.toml`:

```
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

The power of Rust programs used in a CLI[^5] environment, along with the ease to develop crates for different targets, is a strong advantage to use Rust. Coupled with its standard library, Rust is able to talk to machines and fetch data from servers quite easily, then pass those data into your algorithms and gives results in seconds.

## Closing

We have seen that Rust keeps its promises regarding efficiency and portability, making it a great choice to develop your future (or old) DSP algorithms. Getting rid of CMSIS-DSP in order to enjoy the productivity gains brought with Rust is to be considered. 

I would love people who started developing DSP algorithms using Rust, for any target to join the discussion. 

{:.no_toc}

## References

[^1]: [Rust and the panic behavior](https://interrupt.memfault.com/blog/zero-to-main-rust-1#something-just-for-rust)
[^2]: [A little Rust with your C](https://rust-embedded.github.io/book/interoperability/rust-with-c.html)
[^3]: [Optimize Rust for speed](https://rust-embedded.github.io/book/unsorted/speed-vs-size.html#optimize-for-speed)
[^4]: [Cycle Count register](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0439b/BABJFFGJ.html)
[^5]: [Issue: Ability to set crate-type depending on target](https://github.com/rust-lang/cargo/issues/4881)
[^6]: [Command-line apps with Rust](https://www.rust-lang.org/what/cli)