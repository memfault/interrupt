---
title: "Rust on Yocto: A Seamless Integration"
description: "A look at the various ways to use Rust in Yocto"
author: tsarlandie
tags: [rust, yocto, linux, embedded, cross-compilation]
---

<!-- excerpt start -->

At Memfault, our love affair with Rust began in late 2022. What drew us to Rust? Well, the typical allure of a modern programming language: an impressive type-system, memory safety without the constant jitters, efficient concurrency management, a thriving package ecosystem, and overwhelming support from our engineering team. To put it simply, we're smitten. Our journey with Rust has been nothing short of transformative, enabling rapid progress and leading us to conquer challenges we previously deemed ... non-trivial.

<!-- excerpt end -->

Our recent migration of the [elf-coredump processor](https://github.com/memfault/memfault-linux-sdk/blob/kirkstone/meta-memfault/recipes-memfault/memfaultd/files/memfaultd/src/cli/memfault_core_handler/mod.rs) to Rust serves as testament to its readability and maintainability. Plus, Rust's seamless compatibility with binary builds for a plethora of systems, sometimes even facilitated by cross-compilation tools like [cross-rs](https://github.com/cross-rs/cross), has made our development journey smoother. An unexpected perk we discovered along the way? Rust expertise is quite the magnet for top-notch talent.

With many of our clientele relying on Yocto for crafting Custom Embedded System Images, we naturally explored ways to deliver impeccable Rust program support for Yocto-built systems. This blog post unravels our findings, spotlighting three distinct Rust providers for Yocto because, as they say, variety is the spice of life!

{% include newsletter.html %}

{% include toc.html %}

---

> 📺 **Watch the Webinar Recordings**
>
> Check out Thomas' previous webinars for more Embedded Linux discussions: [Over-the-Air Updates for Embedded Linux Devices](https://go.memfault.com/over-the-air-updates-embedded-linux-devices) and [Wrangling Penguins: Better Embedded Linux Monitoring and Debugging](https://go.memfault.com/better-embedded-linux-monitoring-debugging-memfault).

## Introducing meta-rust

[Cody Shafer pioneered the integration of Rust](https://github.com/meta-rust/meta-rust/commit/f7801cf999f7fecde6d5564e72db6d94216978df) into Yocto with the creation of the meta-rust layer back in 2014. This layer is still maintained, and its [`LAYERSERIES_COMPAT` variable](https://github.com/meta-rust/meta-rust/blob/master/conf/layer.conf#L12) signals its compatibility spanning from dunfell (3.1 - April 2020) to Mickledore (4.2 - May 2023).

Let's delve into the fundamental tenets that underpin `meta-rust`:

### Building Rust from Source

In the spirit of absolute reproducibility, the team behind meta-rust chose to compile the Rust compiler directly from its source. This approach aligns with Yocto's philosophical stance favoring source-built components. An intriguing challenge, however, is that the Rust compiler itself is crafted in Rust: A pre-existing "trusted" Rust binary is required as a starting point.

For the curious minds at Interrupt, no rabbit hole is too deep. Let's embark on a brief exploration of how this intricate setup functions within meta-rust. The rust devtool recipe employs a [rust-snapshot to seed the build environment](https://github.com/meta-rust/meta-rust/blob/master/recipes-devtools/rust/rust.inc#L43-L60). This snapshot is [downloaded](https://github.com/meta-rust/meta-rust/blob/master/recipes-devtools/rust/rust-snapshot-1.72.0.inc#L24-L28) from the `rust-lang.org` servers.

> Peeking into the [meta-rust source code](https://github.com/meta-rust/meta-rust/tree/master/recipes-devtools/rust) quickly triggers another aside. Though I've had my fair share of encounters with cross-compilers, and despite sharing many maple syrup-laden breakfasts with Canadian experts in cross-compilation, I was initially stumped by the concept of gcc-cross-canadian.
> Thankfully, a [brief educational detour](https://docs.yoctoproject.org/4.2.3/overview-manual/concepts.html#cross-development-toolchain-generation) clarified that in the realm of Yocto, multiple toolchains are meticulously crafted. The `-cross` toolchain runs on the build machine and builds binaries for the target machine, while the `-cross-canadian` toolchain, also building binaries for the target machine, is tailored to operate on your `SDKMACHINE`. If this piques your curiosity further, [Wikipedia explains](https://en.wikipedia.org/wiki/Cross_compiler#Canadian_Cross) that it's called Canadian because Canada used to have three political parties.

### Explicit Dependency Management

The second pillar of `meta-rust` emphasizes the imperative of freezing project dependencies in time. All your Rust dependencies (all the packages listed in `Cargo.lock`) must be meticulously listed as `crate://` sources within your recipe `SRC_URI`.

These dependencies are fetched by the [Crate Fetcher](https://docs.yoctoproject.org/bitbake/2.4/bitbake-user-manual/bitbake-user-manual-fetching.html#crate-fetcher-crate) which downloads dependencies from [crates.io](https://crates.io/), the Rust package registry, during the `fetch` phase of the build.

Listing the dependencies would be tedious to do manually, so the team created [cargo-bitbake](https://github.com/meta-rust/cargo-bitbake), a tool to generate the recipe, with the proper `SRC_URI` from your `Cargo.lock` file. We will look into this shortly.

## Making it official

Randy MacLeod of WindRiver Systems deserves significant credit for leading the efforts to formalize Rust support in Yocto—much appreciated! For those interested in diving deeper into the intricacies of this journey, [this presentation from Yocto Summit 2020](https://pretalx.com/media/yocto-project-summit-2020/submissions/THCHFZ/resources/Yocto_Project_DevDay_EU2020_Rust_USeJwUC.pdf) sheds light on the detailed process before completion. The culmination of these efforts was realized when it was [merged in 2021](https://github.com/meta-rust/meta-rust/issues/251#issuecomment-906752507) and subsequently released with Yocto 3.4 “Honister”.

Like `meta-rust`, the official Rust support introduces a `cargo` class, making building a Rust package directly from a `Cargo.toml` file straightforward. The approach remains the same: building the Rust compiler from source using a snapshotted version of Rust and necessitating listing all dependencies in the recipe under the format `crate://` sources.

One limitation to note with this integration is anchoring the Rust version to the corresponding Yocto release.

In practice, in the official open-embedded Rust support:
- Honister is tethered to Rust 1.54,
- [Kirkstone](https://github.com/yoctoproject/poky/blob/kirkstone/meta/recipes-devtools/rust/rust_1.59.0.bb) to Rust 1.59,
- Langdale to 1.63,
- and [Mickledore](https://github.com/yoctoproject/poky/blob/mickledore/meta/recipes-devtools/rust/rust_1.68.2.bb) to Rust 1.68.2.

A potential workaround for those not content with these versions is to override built-in rust support with the `meta-rust` layer.

An intriguing development post the official Rust endorsement in open-embedded was Randy MacLeod's [PR submission](https://github.com/meta-rust/meta-rust/pull/364) to meta-rust, proposing the deprecation of this layer in favor of the newly integrated support. Interestingly, this PR has yet to be merged, leaving a layer of ambiguity regarding the delineation between "official" and recommended. The continued existence of meta-rust undeniably offers some benefits, such as extending an updated Rust environment to older Yocto versions.

The progress on meta-rust was further discussed in a [November 2022 presentation](https://www.youtube.com/watch?v=7uCzL2ZwRMU), which highlighted the maintainers' current focus on enhancing testing, reproducibility, and minimizing build times.

## Packaging a Rust Program with the cargo Class

Regardless of whether one opts for the official Rust support or relies on `meta-rust`, the packaging steps for a Rust program remain consistent. (Obviously, if you're using a Yocto version without built-in Rust support or want a newer Rust version, you'd need to clone the `meta-rust` layer and include it in your `conf/bblayers.conf` file.)

Consider the creation of a rudimentary Rust project with an added dependency:

```shell
$ cargo init .
$ cat src/main.rs
fn main() {
    println!("Hello, world!");
}
$ cargo add serde
```

Though we only added one package, it brings along its dependencies, reflected in the six dependencies now listed in `Cargo.lock`. As discussed before, they will all need to be listed in the recipe.

The utility [`cargo-bitbake`](https://github.com/meta-rust/cargo-bitbake) comes in handy when formulating a recipe for our program. It peruses the `Cargo.toml` and `Cargo.lock` files, generating a Yocto recipe based on the provided information.

```
$ cargo install --locked cargo-bitbake
$ cargo bitbake
```

The generated recipe includes all the essential elements required for a Yocto build, with a comprehensive listing of the dependencies and their respective versions.

```
# Auto-Generated by cargo-bitbake 0.3.16
# (a few lines removed for conciseness)

inherit cargo

# how to get helloworld could be as easy as but default to a git checkout:
# SRC_URI += "crate://crates.io/helloworld/0.1.0"
SRC_URI += "gitsm://git@github.com/exampleorg/helloworld.git;protocol=ssh;nobranch=1;branch=main"
SRCREV = "3680398c2c48377f12c7fe79ae6f70bd6881924f"
S = "${WORKDIR}/git"
CARGO_SRC_DIR = ""

# please note if you have entries that do not begin with crate://
# you must change them to how that package can be fetched
SRC_URI += " \
    crate://crates.io/proc-macro2/1.0.69 \
    crate://crates.io/quote/1.0.33 \
    crate://crates.io/serde/1.0.189 \
    crate://crates.io/serde_derive/1.0.189 \
    crate://crates.io/syn/2.0.38 \
    crate://crates.io/unicode-ident/1.0.12 \
"
```

The most important line here is `inherit cargo` which automatically handles cross-building a Rust project with a `Cargo.toml` file, even accommodating the building of C dependencies if a `build.rs` exists.

Embedding this recipe within an existing layer and initiating `bitbake hellorust` builds the rust cross compiler, and our program is built for the target.

## meta-rust-bin

[`meta-rust-bin`](https://github.com/rust-embedded/meta-rust-bin/tree/master), a project of the rust-embedded group, emerges as a sought-after alternative for packaging Rust programs within Yocto. Its main differentiators are:

1. It uses pre-built binaries of `rust`, `cargo` and `libstd-rs`
2. It uses the `Cargo.lock` file to pin the dependencies and `cargo` to download them.

Among its other merits are its frequent updates aligning with the latest Rust versions and its support for older Yocto versions, ranging [from dunfell to langdale](https://github.com/rust-embedded/meta-rust-bin/blob/ef9a765b0785ed9e964691ac1a5a302dd0a0ea62/conf/layer.conf#L13-L20).

Previously, `meta-rust-bin` presented a `cargo` class, rendering it incompatible with `meta-rust` or recent version of Yocto. The recommendation was to use a `BBMASK` statement to "hide" the built-in `cargo` and `rust` classes. Fortunately, this stumbling block was recently addressed with [the class name undergoing a change to `cargo_bin`](https://github.com/rust-embedded/meta-rust-bin/pull/136), enabling seamless usage of both the official Rust support and meta-rust-bin within the same Yocto project.

Unlike `meta-rust`, `meta-rust-bin` does not require pinning all the dependencies in the recipe. However, this poses a challenge since dependencies cannot be downloaded prior to the build's commencement, mandating network access for cargo during the build. Such an approach, [not permitted by default since kirkstone](https://docs.yoctoproject.org/migration-guides/release-notes-4.0.html#new-features-enhancements-in-4-0), necessitates special network-access permission in the recipe with: `do_compile[network] = "1"`.

To employ `meta-rust-bin`:

 - Clone `meta-rust-bin` alongside other Yocto layers.
 - Incorporate meta-rust-bin within your `conf/layers.conf` file.
 - Write a recipe leveraging the `cargo_bin` class:

```
SUMMARY = "GPIO Utilities"
HOMEPAGE = "git://github.com/rust-embedded/gpio-utils"
LICENSE = "MIT"

inherit cargo_bin

# Enable network for the compile task allowing cargo to download dependencies
do_compile[network] = "1"

SRC_URI = "git://github.com/rust-embedded/gpio-utils.git;protocol=https;branch=master"
SRCREV="02b0658cd7e13e46f6b1a5de3fd9655711749759"
S = "${WORKDIR}/git"
LIC_FILES_CHKSUM = "file://LICENSE-MIT;md5=935a9b2a57ae70704d8125b9c0e39059"
```

A notable advantage of this approach is the noticeable reduction in build time. Preliminary evaluations on a medium-sized AWS instance suggest a time-saving in the order of 15 minutes compared to `meta-rust`.

## TLDR

After a comprehensive exploration, we identified three primary avenues to incorporate `rust` into a Yocto project:

1. **The Original `meta-rust` layer**: This method is as a beacon for compatibility with older Yocto releases and furnishes a Rust compiler for many Yocto versions. Nonetheless, it bears its own set of challenges:
    - It mandates the inclusion of an additional layer to your project.
    - The process of rebuilding the Rust compiler from source can considerably slow down the build.
    - Utilizing `cargo bitbake` is essential for recipe preparation, and the list of dependencies needs to be maintained when the `Cargo.lock` dependencies change.

2. **The Open-Embedded `cargo` and `rust` classes**: Functioning as the official Rust endorsement for Yocto, it's seamlessly integrated into open-embedded. However, certain limitations might overshadow its benefits:
    - Rust versions are anchored to specific Yocto releases, which might render them outdated for some projects.
    - Reconstructing the Rust compiler from its source can be a time-consuming affair.
    - Maintaining the recipe is tedious with the necessity to enumerate the complete list of dependencies.

3. **The `meta-rust-bin` layer**: This alternative champions using pre-constructed binaries of Rust, Cargo, and libstd. And the recipes are more concise, eliminating the need to itemize all dependencies. Nevertheless, it has its nuances:
    - It introduces an additional layer to your project.
    - Its recipe blueprint diverges from the `meta-rust` recipes.

At Memfault, our inclination leans towards `meta-rust-bin`. Its compatibility with a broader range of Yocto versions, and its efficiency in reducing build time, strikes a chord with us. That said, drafting a recipe compatible with the official Rust support **and** `meta-rust` isn't a complex undertaking with the assistance of `cargo bitbake`. We keep one at our disposal, catering to clients who favor `meta-rust`.

## Another Important Consideration: Binaries Size

When delving into Rust for embedded systems, certain facets remain untouched in our discussion but merit attention in future deliberations.

- **The Rust Standard Library**: A conspicuous drawback in the methods discussed is the static linking of the Rust standard library into each binary file. This addition augments the file by several hundred kilobytes. An ingenious workaround might be the construction of `libstd-rs` as a dynamic library, allowing all the rust binaries within the system to share it. This factor will gain prominence as Rust binaries become an integral part of your system image. None of the solutions presented today support this, but they all mention it as a future improvement. Our tactic involves symlinking (filesystem links) our three binaries (`memfaultd`, `memfaultctl`, and `memfault-core-handler`) to a singular binary. This unique binary alters its function based on the name used in the invocation, a strategy we fondly refer to as the `busybox` approach.

- **Minimizing Binaries Size**: Optimizing the size of binaries is crucial, an excellent topic for a future post. For now, we will leave those seeking guidance with [this article](https://github.com/johnthagen/min-sized-rust) that offers invaluable insights.

We are eager to gain insights into aspects we might have overlooked and address any queries regarding Rust on embedded systems. Engage with us in the comments, or connect with us on the interrupt Slack community for a more extensive discussion!


>📚**Recommended Reading**
>
>Fan of Rust? Check out our other Rust articles: [Asynchronous Rust on Cortex-M Microcontrollers](https://interrupt.memfault.com/blog/embedded-async-rust), [Rust for Low Power Digital Signal Processing](https://interrupt.memfault.com/blog/rust-for-digital-signal-processing), [From Zero to main(): Bare metal Rust](https://interrupt.memfault.com/blog/zero-to-main-rust-1).