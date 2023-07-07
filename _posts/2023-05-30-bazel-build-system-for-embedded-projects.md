---
title: "Bazel Build System for Embedded Projects"
author: blaise
tags: [bazel, build-system]
---

<!-- excerpt start -->

Selecting a build system is an essential decision when creating a project. Changing is always painful, especially in a mature repository. Therefore the choice should be made carefully. With this article, I will try to describe a few advantages of what Bazel can provide in the context of an embedded repository and show how to set up a build environment with a cross compiler from scratch.

<!-- excerpt end -->

# Bazel Build System for Embedded Projects

{% include toc.html %}

## Why Bazel?

Setting up a build system on a repository is long and fastidious, it’s a step most would want to go through only once. This is one of the many reasons why monolithic repositories (or monorepos) are so popular. While it is often a straightforward choice for most software projects, it also comes with several challenges for embedded projects: how to deal with multiple cross-compilation? How multiple toolchains can coexist in the same repository? How to reuse software components between different architectures? etc.

Fortunately, there is a great tool to solve these issues! Bazel is a build system that has multi-platform support at its core, a very powerful concept that I will try to introduce in the next chapter. In addition to what most build systems provide, Bazel also ensures the reproducibility of builds making every build action hermetic from the development environment. It is a multi-language build system that supports many languages out of the box (C, C++, Rust, Python…) and can be extended in Starlark (Bazel scripting language which is very similar to Python). Finally, it is designed to scale, from small to large repositories, making it a perfect candidate for monorepos.

## Introduction to Bazel

Before jumping into the subject, let's first cover a short introduction to Bazel.

A Bazel workspace is a directory tree that contains the source files for the software you want to build and it is defined by a `WORKSPACE` file at its root. This file may contain instructions for Bazel to create additional repositories that will be used during the build phase, such as fetching external tools, libraries, etc.

> The concept of workspace is being replaced by [modules](https://bazel.build/external/module). Their function and syntax are very similar, since this is a recent concept and does not add to this article, I will not cover it in this post.

A workspace is a collection of `packages` that are defined by a `BUILD` file. These files contain declarations of build `rules` that are uniquely identified through their relative path to the root and a unique name within this file. This is called the `target name`. Each of these rules defines build `actions`, for example, a `cc_binary` rule defines actions such as compilation and linking.

When Bazel is invoked, it will operate in 3 distinct phases, it will first load all `BUILD` files, then analyze all the rules and their dependencies to create an `action graph` and finally execute all the actions until the final outputs are produced.

The behavior and configuration of Bazel can be changed through command line arguments (also called flags). A file named `.bazelrc` can be used at the root of the workspace to collect all flags that should be added when invoking Bazel.

## Using Bazel

Bazel is used as a command line tool. It has many commands and flags so I will not go through all, but here is a short overview of the most important ones.

Building a target is done as follows:
```bash
bazel build //<package>:<target>
```
Where `<package>` is the path of the package to be built and `<target>` specifies one or more targets. For example `bazel build //my_dir:hello`, will build the target named `hello`, within the directory `my_dir`, or `bazel build //my_dir/...` will build all targets within the directory `my_dir`. Various declinations of this syntax make it less verbose, for example, the leading `//` can be omitted, and if the target name is the same as the package name, it can also be omited.

Similarly, all tests targets can be tested with:
```bash
bazel test //my_dir/...
```
A test target is a Bazel rule that implements a `test` executable. Such rules are commonly named such as `cc_test`, `py_test`, etc.

An executable target can run with:
```bash
bazel run //my_dir:hello
```
An executable target is any Bazel rule that can be executed, so it also includes test targets. Executables that are not test targets are commonly named such as `cc_binary`, `py_binary`, etc.

Note that running `bazel test` or `bazel run` will also run `bazel build` if the target was not built or if any of the input files of the target have been altered.

Flags can also be used to change the behavior of a command. These flags can be passed as command line arguments, such as `bazel test //:my_target --runs_per_test=10` to execute a specific test 10 times, or `bazel build /:my_trarget --platforms=//platforms:esp32` to cross-compile a target for esp32.

These flags can be combined and can add up to a very verbose command line which makes it difficult to remember, type or even share. That is where the `.bazelrc` file comes in handy. Here is an example of a `.bazelrc` file:

```bash
# All options comming from this file will be announced on the terminal.
common --announce_rc

# If the file `.bazelrc.local`, exists it will be imported.
try-import .bazelrc.local

# Add timestamps to message.
common --show_timestamps

# If a command fails, print out the full command line.
build --verbose_failures

# Print the full error of the test that fails.
test --test_output=errors

# Configuration for "stress".
test:stress --runs_per_test=10
test:stress --notest_keep_going
```

Each flag are preceded by a command, which corresponds to the Bazel command to which it applies, or some may have specific meaning such as `common`, which applies to all Bazel commands, or `try-import`, which tries to import a file if it exists.

In addition, commands can be grouped into configs. This is represented here with `test:stress`. It means that when the user executes `bazel test --config=stress //:my_target`, the flags `--runs_per_test=10` and `--notest_keep_going` will be added to the command line.

In a mature project, this file can be very large, hence it is always a good practice to add at the beginning `common --announce_rc` to ensure that all the added commands are also announced on the standard output when the command is executed.

Now that we know the basis of what Bazel is and how to use it, let's deep dive into the subject.

## Platforms

Platforms is a Bazel concept to deal with build variants. This concept addresses many of the challenges in embedded development, for example when dealing with different hardware, having multiple products with different features, handling different product versions, using different compilers, sharing software components, testing on different architectures, and many more...

To understand how platforms are used with Bazel, there are a few fundamentals to understand first.

A platform is a way for Bazel to model the environment of the build, it is a very abstract notion that can describe many things such as hardware, a CPU, a software feature, etc. There are 3 kinds of platforms:

- `host`: the platform on which Bazel itself runs (your Linux machine for example).
- `execution`: the platform on which the tools invoked by Bazel runs to generate build artifacts (your Linux machine or an Amazon server where the build is taking place with remote execution for example).
- `target`: the platform on which the final output resides and executes (a Raspberry PI or a software version or just a feature such as allowing runtime exceptions for testing for example).

In a cross-compilation context, the **host** and **execution** platforms are often the same. So for the following, we will focus on this scenario.

> Note that Bazel will generate the host and execution platform by automatically detecting the type of machine it is running on.

A platform is simply a collection of **constraints** (you can see a constraint as an identifier for now). To choose a compiler, Bazel will try to match the constraints defined by the compiler for the execution platform and the target platform, this step is called the **toolchain resolution**.

For example, imagine we defined a platform called `esp32` that defines the constraint `@platforms//cpu:xtensa`. When Bazel is invoked with the argument `--platforms=esp32`, the execution platform is automatically generated by Bazel (it contains constraints that define your current machine, for example, `@platforms//cpu:x86_64` while the target platform is the one passed into an argument which defines the constraints `@platforms//cpu:xtensa`).

With this pair of constraints, during the toolchain resolution process, Bazel will try to find the appropriate toolchain to compile your code.

This process is transparent to the user and therefore, if multiple toolchains are available for various execution platforms (for example, one for Linux, one for Windows), the user can seamlessly compile the same code on different hosts running the same command without even thinking about it.

## Toolchains

One of the key features that Bazel solves with platforms is the ability to deal with different toolchains.
Out of the box, Bazel will try to figure out which tools are available locally and use them transparently to make it easy to get started.

For example, given the following `BUILD` file:

```python
cc_binary(
    name = "main",
    srcs = ["main.cc"],
)
```

When running, `bazel run //:main`, Bazel will use your locally installed C++ toolchain (gcc for example) and compile and link with gcc using a default recipe. The code above is all you need with Bazel to get started with a C or C++ project.
This is convenient but not when you need a reproducible build or need to cross-compile for example. For such use cases, you will need to use a custom toolchain.

### Custom

For this example, we will integrate a pre-compiled ARM C++ toolchain into Bazel (taken from the official [ARM website](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)). This will create a semi-hermetic toolchain ("semi" because it will still rely on some of the libraries of the host such as libc, etc. There are ways to make it fully hermetic but this is not the topic of this article.)

For this process, we will follow specific steps that can be re-used to integrate any toolchain.

#### Fetch the binaries

The first step is to integrate the pre-compiled binaries into Bazel, so that it can be accessed by a toolchain rule. The binaries can be downloaded with the Bazel repository rule http_archive. This will simply download a binary from a URL, unpack it and make it accessible as an external resource from Bazel. Note that you must provide a `BUILD` file in addition to exposing the interface from this newly created repository.

As this is a repository rule, it must be called from the `WORKSPACE` file.
For example:

```python
http_archive(
    name = "arm64_gcc_linux_x86_64",
    urls = ["https://developer.arm.com/-/media/Files/downloads/gnu/12.2.rel1/binrel/arm-gnu-toolchain-12.2.rel1-x86_64-aarch64-none-linux-gnu.tar.xz"],
    strip_prefix = "arm-gnu-toolchain-12.2.rel1-x86_64-aarch64-none-linux-gnu",
    sha256 = "6e8112dce0d4334d93bd3193815f16abe6a2dd5e7872697987a0b12308f876a4",
    build_file = "//toolchain_arm64_gcc:arm64_gcc_linux_x86_64.BUILD",
)
```

and the toolchain_arm64_gcc/arm64_gcc_linux_x86_64.BUILD file looks like:

```python
filegroup(
    name = "all",
    srcs = glob(["**/*",]),
)
```

After this step, the binaries will be accessible from the Bazel target `@arm64_gcc_linux_x86_64//:all`.

#### Create a CC toolchain

Now we need to create a `cc_toolchain`, this will instruct Bazel how to use the C++ toolchain just fetched previously.
The `cc_toolchain` rule, takes as parameters the set of files that need to be added to the sandbox for the different toolchain actions (compiling, linking, etc). Since we already exposed all the files from the package before with a `filegroup` named `all`, we can re-use it for all these arguments.

> Note that it is recommended to use the smallest set of files possible to avoid pulling all files every time, which could slow down the process on some execution platforms.

The `cc_toolchain` also requires a `toolchain_config` argument, which we will create in the next step. This is how the `cc_toolchain` invocation could look like in `toolchain_arm64_gcc/arm64_gcc_linux_x86_64.BUILD`:

```python
cc_toolchain(
    name = "cc_toolchain",
    all_files = ":all",
    ar_files = ":all",
    as_files = ":all",
    compiler_files = ":all",
    dwp_files = ":all",
    linker_files = ":all",
    objcopy_files = ":all",
    strip_files = ":all",
    static_runtime_lib = ":all",
    toolchain_config = ":my_cc_toolchain_config",
)
```

To create the `toolchain_config` that we named `my_cc_toolchain_config` in this example, we can use the `cc_toolchain_config` rule provided by the `@bazel_tools//` repository.

With this rule, we can tell Bazel how to use the toolchain binaries (which one to call and with which arguments).

We need to provide the path of all the binaries needed for the different toolchain actions, and the set of flags that need to be provided to the tools when compiling and linking. There are more options, but these are the most important to get started.

Without going too much into detail here is how this could look like in `toolchain_arm64_gcc/arm64_gcc_linux_x86_64.BUILD`:

```python
load("@bazel_tools//tools/cpp:unix_cc_toolchain_config.bzl", "cc_toolchain_config")

cc_toolchain_config(
    name = "my_cc_toolchain_config",
    cpu = "arm64",
    compiler = "gcc",
    toolchain_identifier = "arm64_gcc",
    host_system_name = "local",
    target_system_name = "local",
    target_libc = "unknown",
    abi_version = "unknown",
    abi_libc_version = "unknown",
    tool_paths = {
        "gcc": "bin/aarch64-none-linux-gnu-g++",
        "cpp": "bin/aarch64-none-linux-gnu-cpp",
        "ar": "bin/aarch64-none-linux-gnu-ar",
        "nm": "bin/aarch64-none-linux-gnu-nm",
        "ld": "bin/aarch64-none-linux-gnu-ld",
        "as": "bin/aarch64-none-linux-gnu-as",
        "objcopy": "bin/aarch64-none-linux-gnu-objcopy",
        "objdump": "bin/aarch64-none-linux-gnu-objdump",
        "gcov": "bin/aarch64-none-linux-gnu-gcov",
        "strip": "bin/aarch64-none-linux-gnu-strip",
        "llvm-cov": "/bin/false",
    },
    compile_flags = [
        "-isystem", "external/arm64_gcc_linux_x86_64/aarch64-none-linux-gnu/include/c++/12.2.1/aarch64-none-linux-gnu",
        "-isystem", "external/arm64_gcc_linux_x86_64/aarch64-none-linux-gnu/include/c++/12.2.1",
        "-isystem", "external/arm64_gcc_linux_x86_64/aarch64-none-linux-gnu/include",
        "-isystem", "external/arm64_gcc_linux_x86_64/aarch64-none-linux-gnu/libc/usr/include",
        "-isystem", "external/arm64_gcc_linux_x86_64/lib/gcc/aarch64-none-linux-gnu/12.2.1/include",
    ],
    link_flags = [],
)
```

At this point, we instructed Bazel on how to use the toolchain, now we need to tell Bazel when to use it and for which language.

#### Create a toolchain

A toolchain is created with the toolchain rule and associates a language-specific toolchain (like the `cc_toolchain` we previously created) with a toolchain type (that describes the C++ language in our case) and a set of constraints that need to be matched to be selected by the automatic toolchain resolution process we mentioned before.

In our case, the constraints for the execution platform will describe the host on which the toolchain we downloaded is built to run. While the constraints for the target platform will describe the ARM CPU.

Here is what the toolchain rule can look like in `toolchain_arm64_gcc/BUILD`:

```python
toolchain(
    name = "arm64_gcc_linux_x86_64",
    exec_compatible_with = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        "@platforms//cpu:arm64",
    ],
    toolchain_type = "@rules_cc//cc:toolchain_type",
    toolchain = "@arm64_gcc_linux_x86_64//:cc_toolchain",
)
```

#### Register the toolchain

Now we need to register the toolchain so that Bazel knows about its existence. This is done in the `WORKSPACE` file as follows:

```python
register_toolchains(
    "//toolchain_arm64_gcc:arm64_gcc_linux_x86_64",
)
```

#### Create a platform

At this point, the toolchain is fully integrated into Bazel and ready to be used. We only need a way to select it, this is done by creating a platform that will set the constraints we defined previously in the toolchain rule.

We need to create a platform that will be used as the target platform. Note the execution platform is the same as the host platform and is auto-generated by Bazel and it matches your current host environment.

This target platform only needs to set constraints to define the ARM CPU, it can be done as following in `toolchain_arm64_gcc/BUILD`:

```python
platform(
    name = "toolchain_arm64_gcc",
    constraint_values = [
        "@platforms//cpu:arm64",
    ],
)
```

#### Use the toolchain

We now have all the pieces to build a target with our new toolchain. To invoke this toolchain, we need to tell Bazel that we want to build for the previously create target platform. This is done as follow:

```bash
bazel build … --platforms=//toolchain_arm64_gcc --incompatible_enable_cc_toolchain_resolution
```

This is it! You have integrated a semi-hermetic toolchain into Bazel!

Note, `--incompatible_enable_cc_toolchain_resolution` is required to enable toolchain resolution with platforms for C++ toolchains. This flag should be enabled by default starting from Bazel 7.0.0, so in future releases, it will not be needed anymore.

The code described in this section can be found here: [](https://github.com/blaizard/bazel-examples/tree/main/toolchain-arm64-gcc)

### Dockerized

Often when dealing with cross-compilation, you don’t get a self-contained binary or package that you can just unzip and use directly, but you need to install the toolchain on your machine, which makes it very difficult to use or integrate into CI for example or might conflict with an existing installation you already have. Or just by simplicity you might have your environment already set up in a Docker image and would like to be able to integrate it seamlessly into the build process.

For such a situation, Bazel can use a toolchain (or any tool in general) hosted on a docker image. This is done by defining an execution platform that makes use of the docker image.

#### Create an execution platform

First, we need to create the platform that will be used as the execution platform.
The constraint value associated with this platform will be used to select the toolchain, so we can create an arbitrary constraint value.

Here is an example:

```python
constraint_setting(
    name = "docker_image"
)

constraint_value(
    name = "docker_image_linux_arm64",
    constraint_setting = ":docker_image",
)

platform(
    name = "execution_platform",
    constraint_values = [
        ":docker_image_linux_arm64",
    ],
    exec_properties = {
        "container-image": "docker://docker.io/dockcross/linux-arm64:20230421-a0eaff4",
    },
)
```

The important piece is to point to the docker image URL with the `container-image` key of the `exec_properties` attribute.

#### Register the execution platform

Then we need to register the execution platform in the `WORKSPACE` file:

```python
register_execution_platforms(
    "//toolchain_arm64_gcc_docker:execution_platform",
)
```

This step makes the execution platform known by Bazel available for selection during the automatic toolchain resolution process. In Bazel point of view, all the execution platforms registered as such, are available from this host machine to execute an action.

#### Create the toolchain

In the same way as a toolchain was defined in the previous section, the `toolchain`, `cc_toolchain`, and `cc_toolchain_config` rules need to be created and point to the binaries within the Docker container.

The only difference is that now the toolchain rule takes as `exec_compatible_with` constraint, the one we used to create the execution platform.

#### Update the sandbox strategy

The last step is to tell Bazel to prioritize Docker sandboxing over the default strategy. That way, Bazel will check if the execution platform selected is compatible with the Docker strategy and use it, if not, the default sandboxing will be used.

This is done by adding these 2 lines in the `.bazelrc` file:

```bash
build --experimental_enable_docker_sandbox
build --spawn_strategy=docker,sandboxed
```

That’s it, you can now run a toolchain or any tool from within a docker image!

The code described in this section can be found here: [](https://github.com/blaizard/bazel-examples/tree/main/toolchain-arm64-gcc-docker)

### Remote execution

Another interesting use case that Bazel can solve is remote execution. This can be useful in the domain of embedded development for the following scenarios for example:

- The toolchain only runs on Windows but the developers have Linux machines
- There is a limited number of licenses for a tool
- A hardware key is required for a specific tool
- ...

For all these use cases, it is hard to integrate such tools in the developer workflow as it requires building on a specific machine or has other accessibility constraints.

By setting up a remote execution server (here is a list of available options: [](https://bazel.build/community/remote-execution-services)), Bazel can execute some of these actions on a remote machine. This is done by changing the default strategy (as we did in the previous section) to remote and pointing to the address of this server, like in this example:

```bash
build --remote_executor=grpc://my/server:8980
build --spawn_strategy=remote,sandboxed
```

The selection of the remote runners is done by matching the set of `exec_properties` from the selected execution platform, similar to how the Docker image was selected in the previous section.

The rest of the configuration is specific to the remote execution server you selected, so please refer to their documentation if you want to explore this option.

## Target selection

In addition selecting the correct toolchain, a platform can also be used to conditionally select targets during the build.
For example, let’s imagine we have a library called `my_library` that relies on different backends depending on the architecture used.

We can do the selection at build time as follow:

```python
cc_library(
    name = “my_library“,
    srcs = ["my_library.cc"],
    deps = select({
        ":arm_build": [":arm_lib"],
        ":x86_build": [":x86_lib"],
        "//conditions:default": [":generic_lib"],
    }),
)
```

The `select` built-in can be used for anything, such as compilation options, defines, dependencies, source files, etc. It lets you deal with variance at build time.
Conditions can also be associative with the use of `selects.with_or` and `selects.config_setting_group` to achieve OR and AND boolean operations.

## Target

Finally, when building all targets in the repository with a specific target platform, the incompatible ones will be skipped.

Compatibility or incompatibility of a target is described with the `target_compatible_with` attribute of any rule. Note that this attribute propagates from its dependencies as well. In other words, if a target depends on a Windows library for example, and the latter is described with the appropriate `target_compatible_with` values, then if built on for a Linux platform, this all its parent target(s) will be skipped (not built).

Here is for example how this could look like:

```python
cc_library(
    name = "windows_lib",
    srcs = ["windows_lib.cc"],
    target_compatible_with = [
        "@platforms//os:windows",
    ],
)
```

> Note that targets will be skipped only if all targets are requested to be built, using `bazel build //...` or `bazel test //...` for example. If a specific target is built and is incompatible, the build will fail.

This is very useful when one wants to test all targets that are compatible with a specific platform either locally after a change or in CI for example.

## Conclusion

Bazel is a powerful and effective build tool for embedded systems, owing to its unique features and concepts. While this brief overview has provided an insight into its capabilities, there is much more to explore and uncover.

However, Bazel has also a steep learning curve, which may pose a challenge for beginners. The effort put into mastering its main concepts is worthwhile, as it offers numerous advantages over traditional build systems such as Makefile and CMake.

By leveraging Bazel's capabilities, developers can achieve faster build times, better reproducibility, and higher levels of code optimization, ultimately leading to more efficient and reliable embedded systems. Therefore, it is worth investing the time and resources necessary to gain a deep understanding of Bazel and incorporate it into your development workflow.

## References

Here is a list of useful links for further reading.

- [Bazel official documentation](https://bazel.build/docs)
- [Bazel CLI](https://bazel.build/reference/command-line-reference): CLI reference for Bazel.
- [Bazel Central Registry](https://registry.bazel.build/): Bazel modules ready to be used.
- [Bazel examples on various topics](https://github.com/bazelbuild/examples)
- [A collection of Bazel Arm Cortex-M toolchains](https://github.com/bazelembedded/bazel-embedded)
- [A curated list of Bazel rules, tooling and resources](https://awesomebazel.com/)
