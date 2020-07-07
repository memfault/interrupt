---
title: Test Automation with Renode
description: TODO
author: tyler
image:
---

Automated tested on real embedded hardware is what every hardware company
strives to build. The mission is to repeatedly verify that changes being merged
into master and shipped to customers will not experience bugs.

The problem with automated firmware testing with real hardware is that it's
**hard**. Not only does the system require a relatively stable firmware to build
on top of, it will also require extra hardware and software to orchestrate the
tests and capture results.

An alternative to building a complicated orchestration system for automated
tests is to use the popular hardware emulator project, Renode, and it's built-in
RobotFramework integration.

<!-- excerpt start -->

In this post, I cover what it takes to build a simple firmware for use with
Renode, how to build and administer tests with RobotFramework, and how to run
these tests within Github's continuous integration system.

<!-- excerpt end -->

_Like Interrupt? [Subscribe](http://eepurl.com/gpRedv) to get our latest posts
straight to your mailbox_

{:.no_toc}

## Table of Contents

<!-- prettier-ignore -->
* auto-gen TOC:
{:toc}

## What is Test Automation?

Test automation is the practice of testing software and/or hardware in a
repeatable and scalable way where expected values are compared against actual
values.

| Type of Testing            | Description                                                                                                            | Test Medium    | Difficulty  |
| -------------------------- | ---------------------------------------------------------------------------------------------------------------------- | -------------- | ----------- |
| Manual Device Testing      | Manual tests performed on real hardware or development boards by engineers or QA.                                      | Device         | None        |
| Unit Tests                 | Isolated tests which perform tests on a single module. Usually ran on the host machine and sometimes on a real device. | Host, Device   | Low         |
| Software Integration Tests | Tests which perform tests on a collection of modules that interact with each other.                                    | Host or Device | Medium      |
| Emulator Integration Tests | Tests which perform                                                                                                    | Emulator       | Medium\*    |
| Hardware "Unit Tests"      |                                                                                                                        | Device         | Large       |
| Hardware Integration Tests |                                                                                                                        | Device         | Large       |
| Hardware End-to-end Tests  |                                                                                                                        | Device         | Extra Large |

\* - Assuming an emulator for the platform is already available through Renode
or QEMU.

## Different Types of Test Automation

Since reading [Francois' introduction to
Renode]({% post_url 2020-03-23-intro-to-renode %}) post, I've been dying to try
it out. Many comments in response to this post revolved around whether Renode
would be a good fit for automated tests in CI. The answer is **absolutely**.

## Performing Tests on Device

Whether tests are being run within Renode or on real hardware, there should be a way to easily run tests on a device. 

A common approach is to use a CLI-based shell over serial, which we wrote about in our post
[Building a Tiny CLI Shell for Tiny
Firmware]({% post_url 2020-06-09-firmware-shell %}#integration--automated-tests).

The idea is relatively simple.

1. Create a test which will execute a series of shell commands on a device.
2. After each command or series of commands, the results will be compared against expected values.
3. At the end of the test run, the harness determines whether the test was successful or not.

Thankfully for us, Renode has a number of utilities and helpers built-in that we can take advantage of. 

## Setup

### Example Repo

All the code in this post was written for the STM32F429 MCU by ST Micro. While
the examples run fine on the STM32F429i discovery board, they were developed in
Renode, a popular MCU emulation platform.

You can find the complete code example for this blog post in a separate
[Github repository](https://github.com/memfault/interrupt-renode-test-automation)

### Toolchain

Running automated tests with Renode unfortunately requires Python 2.7. To keeps
things simple for me, I decided to use a Conda environment to keep this entire
environment isolated.

```
$ conda create -n renode
$ conda activate renode
$ conda install -c conda-forge -c memfault python=2.7 gcc-arm-none-eabi

$ which arm-none-eabi-gcc
/Users/tyler/miniconda3/envs/renode/bin/arm-none-eabi-gcc
```

Perfect, I now have `arm-none-eabi-gcc` and `python2.7` in my path.

> Want to learn more about Conda? Check out my previous post on using [Conda
> environments for embedded
> development]({% post_url 2020-01-07-conda-developer-environments %}).

### Renode

To install Renode, I've found the best instructions to be on the
[Renode Github Page](https://github.com/renode/renode#installation). In the
repo, I've created a `start.sh` script which points to my local Renode
installation on my Mac. If you are another platform, you'll have to update this
script.

```bash
#!/bin/sh

sh /Applications/Renode.app/Contents/MacOS/macos_run.command renode-config.resc
```

To run Renode with RobotFramework, we'll also need to clone the Renode
repository and all of its submodules, as well as installing all of the Python
dependencies.

```
$ git clone git@github.com:renode/renode.git --recurse-submodules
$ cd renode

# Install Robot Framework and other dependencies
$ pip install -r tools/requirements.txt
```

### Docker

To use Renode with Robot Framework in CI, it's best to use the official
[Renode Docker image](https://hub.docker.com/r/antmicro/renode) for running the
tests. If you haven't already, you'll want to install
[Docker](https://www.docker.com/products/docker-desktop).



## Github Actions & Renode

It's now time to plug things into a continuous integration system for automated testing! In a previous post, we wrote about [building firmware in CircleCI]({% post_url 2019-09-17-continuous-integration-for-firmware %}). This time, we are going to use Github Actions to build and test our firmware, as it's likely the easiest for most people to get up and running. 

To start, we'll want to start off by creating a file `.github/workflows/main.yml` in our repo. The `blank` example from Github's [starter-workflows](https://github.com/actions/starter-workflows/blob/master/ci/blank.yml) gives a quick introduction to how the system works. I've copied it down below for easy reference. 

```
# This is a basic workflow to help you get started with Actions

name: CI

# Controls when the action will run. Triggers the workflow on push or pull request
# events but only for the master branch
on:
  push:
    branches: [ master ]
  pull_request:
    branches: [ master ]

# A workflow run is made up of one or more jobs that can run sequentially or in parallel
jobs:
  # This workflow contains a single job called "build"
  build:
    # The type of runner that the job will run on
    runs-on: ubuntu-latest

    # Steps represent a sequence of tasks that will be executed as part of the job
    steps:
    # Checks-out your repository under $GITHUB_WORKSPACE, so your job can access it
    - uses: actions/checkout@v2

    # Runs a single command using the runners shell
    - name: Run a one-line script
      run: echo Hello, world!

    # Runs a set of commands using the runners shell
    - name: Run a multi-line script
      run: |
        echo Add other actions to build,
        echo test, and deploy your project.
```

### Building Firmware in CI

It's time to adapt the sample above to make it build our firmware. First, we'll need to make sure that the `checkout` routine clones submodules as well, since our example repo has a couple of them.

```
jobs:
  build_and_test:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v2
      with:                #
        submodules: true   # CHANGED
```

Next, we'll want to download and configure the ARM embedded toolchain since it isn't installed by default. Thankfully, there is already an Action, [`fiam/arm-none-eabi-gcc`](https://github.com/fiam/arm-none-eabi-gcc), that we can use to install the toolchain for us.

```
    steps:
    [...]

    # Get the arm-non-eabi-gcc toolchain   
    - name: Install arm-none-eabi-gcc
      uses: fiam/arm-none-eabi-gcc@v1
      with:
          release: '9-2019-q4' 
```

The last thing we need to do is to invoke `make` itself to start the build. 

```
    steps:
    [...]

    - name: Build Firmware
      run: make -j4
```


### Running Renode in CI

The next thing we need to do is to get Renode working in CI. For the example, this is as simple as running `./docker-test.sh` since the Github Actions Ubuntu runners have Docker already pre-installed.

```
    steps:
    [...]

    - name: Renode Tests
      run: ./docker-test.sh 
```

Since the script `docker-test.sh` was configured to output the test artifacts to the folder `test_results/`, we will want to capture that directory and save it as an artifact in the CI build.

```
    steps:
    [...]

    - name: Upload Output Directory
      uses: actions/upload-artifact@v2
      with:
        name: Renode Test Results
        path: test_results/
```


### Complete Github Action `main.yml`

```
name: Renode Automated Tests

on:
  push:
    branches: [ master ]
  pull_request:
    branches: [ master ]

jobs:
  build_and_test:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v2
      with:
        submodules: true

    # Get the arm-non-eabi-gcc toolchain   
    - name: Install arm-none-eabi-gcc
      uses: fiam/arm-none-eabi-gcc@v1
      with:
          # The arm-none-eabi-gcc release to use.
          release: '9-2019-q4' 

    - name: Build Firmware
      run: make -j4

    - name: Upload ELF
      uses: actions/upload-artifact@v2
      with:
        name: renode-example.elf
        path: build/renode-example.elf

    - name: Renode Tests
      run: ./docker-test.sh 

    - name: Upload Output Dir
      uses: actions/upload-artifact@v2
      with:
        name: Renode Test Results
        path: test_results/
```



## Final Thoughts

As an evangelist for developer productivity, I love finding repeated tasks and
automating the processes, whether that is in the firmware shell, a [project CLI
on the host
machine]({% post_url 2019-08-27-building-a-cli-for-firmware-projects %}), or in
[the
debugger]({% post_url 2019-07-02-automate-debugging-with-gdb-python-api %}).

I hope this post conveyed the reasons why having a developer-focused interface
into a firmware-based device is helpful. I'd love to hear about how you think
about command-line interfaces and also how you strike the balance between
features and code space usage in your shell.

You can find the examples shown in this post
[here](https://github.com/memfault/interrupt/tree/master/example/firmware-shell).

See anything you'd like to change? Submit a pull request or open an issue at
[Github](https://github.com/memfault/interrupt)

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^sdk_uart]: [Nordic nRF52 SDK - UART](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.3.0%2Fgroup__app__uart.html)
<!-- prettier-ignore-end -->
