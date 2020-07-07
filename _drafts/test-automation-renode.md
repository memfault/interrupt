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
Robot Framework integration.

<!-- excerpt start -->

In this post, I cover what it takes to build a simple firmware for use with
Renode, how to build and administer tests with Robot Framework, and how to run
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

There are various forms of test automation, and they are usually run within or controlled by a continuous integration system, such as Jenkins, Github Actions, or CircleCI.

- Software Unit Tests - Isolated tests which perform tests on a single module. Usually ran on the host machine and sometimes on an emulator or real device.
- Integration Tests - Tests which perform tests on a collection of modules that interact with each other
- End-to-end Tests - Tests which exercise an entire technical stack end-to-end. These tests would usually communicate with some sort of backend in the cloud or mobile phone.

These types of testing are not limited entirely to software, as hardware can be tested in similar ways. "Hardware unit testing" is a concept where individual pieces of the larger hardware board are put under test to ensure the chip itself behaves correctly. Integration tests and end-to-end tests can also be performed on hardware, but the complexity increases exponentially.

A middle ground between host-run tests and hardware tests is testing on an emulator which emulates the real hardware as much as possible. Renode has become our favorite emulator at Memfault, and it's list of support boards is growing[^renode_boards]. Renode will emulate many peripherals of the board, including the UART, SPI, I2C, RAM, ROM, and GPIO's. 


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
[Github repository](https://github.com/memfault/interrupt-renode-test-automation).

```
$ git clone https://github.com/memfault/interrupt-renode-test-automation.git --recurse-submodules
```

### Toolchain

I used the following tools to build my firmware:

- GCC 8.3.1 / GNU Arm Embedded Toolchain as our compiler[^gnu_toolchain]
- GNU Make 4.2.1 as the build system

Running automated tests with Renode unfortunately requires Python 2.7. To keeps
things simple for me, I decided to use a Conda environment to keep this entire
environment isolated.

```
$ conda create -n renode
$ conda activate renode
$ conda install -c conda-forge -c memfault \
      python=2.7 gcc-arm-none-eabi make=4.2.1

$ which arm-none-eabi-gcc
/Users/tyler/miniconda3/envs/renode/bin/arm-none-eabi-gcc
```

Perfect, I now have `arm-none-eabi-gcc` and `python2.7` in my path. 

> Want to learn more about Conda? Check out my previous post on using [Conda
> environments for embedded
> development]({% post_url 2020-01-07-conda-developer-environments %}).

It turns out the Renode team packages the application in a [Renode Conda package](https://anaconda.org/antmicro/renode), but at the time of writing this post, not all platforms had the latest version released (v1.9, released in March). For that reason, I chose to install it locally for this post.

### Renode & Robot Framework

To install Renode, I've found the best instructions to be on the
[Renode Github Page](https://github.com/renode/renode#installation). In the
example repo, I've created a `start.sh` script which points to my local Renode
installation on my Mac. If you are using another platform, you'll have to update this
script to point to your own Renode start script.

```bash
#!/bin/sh

sh /Applications/Renode.app/Contents/MacOS/macos_run.command renode-config.resc
```

To run Robot Framework alongside Renode, we'll also need to clone the Renode
repository and all of its submodules, as well as install all of the Python
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

## Example Firmware Overview

This post builds upon previous ideas and examples written on Interrupt. If you find yourself missing some context, the following posts would be useful:

* [Cortex-M MCU Emulation with Renode]({% post_url 2020-03-23-intro-to-renode %})
* [Building a Tiny CLI Shell for Tiny Firmware]({% post_url
  2020-06-09-firmware-shell %})

Rather than the STM32Cube HAL, I used an open-source MCU HAL called `libopencm3`
with excellent support for the STM32. It is included as a submodule in the repo.

The primary way we'll control the firmware is through a CLI shell written previously and linked above. 

With the shell in place, this means that our main loop of the firmware is simply:

```c
int main(void) {
    clock_setup();
    gpio_setup();
    usart_setup();

    printf("App STARTED\n");

    // Configure shell
    sShellImpl shell_impl = {
      .send_char = usart_putc,
    };
    shell_boot(&shell_impl);

    char c;
    while (1) {
        c = usart_getc();
        shell_receive_char(c);
    }

    return 0;
}
```

The supporting `*_setup()` calls and their implementations can be found in the other `.c` files in [`src/`](https://github.com/memfault/interrupt-renode-test-automation/tree/master/src).


We can build our example firmware by invoking the build system. 

```
$ make -j4
Building libopencm3
  GENHDR  include/libopencm3/stm32/f4/irq.json
  [...]
  AR      libopencm3_stm32f4.a
  LD    build/renode-example.elf
```

Now we have our `renode-example.elf` file which we can load into Renode. 

```
$ ./start.sh
```

In the Renode Monitor window, we type `start`.

```
(STM32F4_Discovery) start
```

And then we see our firmware's shell in the UART window of Renode. 

![](img/test-automation-renode/renode-start.png)

We can interact with it exactly as if it was connected to our computer over USB serial! This would have made writing and testing the firmware for my [Tiny Shell]({% post_url 2020-06-09-firmware-shell %}) post **much** easier. I'm glad I took the time now to learn Renode for the next time. 

## Anatomy of a Robot Framework Test

Now that we've verified that the firmware works within Renode, it's now time to think about how we are going to test our firmware. We have two shell commands, `help` and `ping` which are probably useful "sanity" checks that we can test, so let's start with those. 

If we write `ping` as a command into our shell, the following is printed:

```
shell> ping
PONG
```

It makes logical sense to test that this exact text is printed. Although incredibly basic, this is the code that we are testing in our firmware:

```c
int cli_command_ping(int argc, char *argv[]) {
    shell_put_line("PONG");
    return 0;
}
```

The following is a Robot Framework test file that calls the `ping` shell command and expects that `PONG` is printed.

```robot
# test_basic.py

*** Settings ***
Suite Setup       Setup
Suite Teardown    Teardown
Test Setup        Reset Emulation
Resource          ${RENODEKEYWORDS}

*** Variables ***
${SHELL_PROMPT}    shell>

*** Keywords ***
Start Test
    Execute Command             mach create
    Execute Command             machine LoadPlatformDescription @platforms/boards/stm32f4_discovery-kit.repl
    Execute Command             machine LoadPlatformDescription @${PWD_PATH}/add-ccm.repl
    Execute Command             sysbus LoadELF @${PWD_PATH}/build/renode-example.elf
    Create Terminal Tester      sysbus.uart2
    Start Emulation

*** Test Cases ***
Ping
    [Documentation]             Prints help menu of the command prompt
    [Tags]                      non_critical

    Start Test

    Wait For Prompt On Uart     ${SHELL_PROMPT}
    Write Line To Uart          ping
    Wait For Line On Uart       PONG        timeout=2
```

To help run this test locally, I've created a few extra helpers. First is a script, `run_tests.sh`, which contains the following:

```
#!/bin/bash -e

RENODE_CHECKOUT=${RENODE_CHECKOUT:-~/code/renode}

${RENODE_CHECKOUT}/test.sh -t "${PWD}/tests/tests.yaml" --variable PWD_PATH:"${PWD}" -r "${PWD}/test_results"
```

`RENODE_CHECKOUT` is the Renode repo that we cloned (with all submodules initialized!). The second line calls Renode's `test.sh` helper with the test manifest file and tells it where to output the results.

The test manifest file, `tests.yaml` contains a list of tests that `test.sh` should iterate over, and it can be used if we want to add more than a single Robot test file.

```
# tests.yaml
- robot:
    - tests/test-basic.robot
```

If we run our script, let's see what we get!

```
# Ensure our Python2.7 environment is activated
$ conda activate renode

# Go!
$ ./run_tests.sh
Preparing suites
Started Renode instance on port 9999; pid 82717
Starting suites
Running tests/test-basic.robot
+++++ Starting test 'test-basic.Ping'
+++++ Finished test 'test-basic.Ping' in 1.55 seconds with status OK
Cleaning up suites
Closing Renode pid 82717
Aggregating all robot results
Output:  /Users/tyler/dev/interrupt-renode-test-automation/test_results/robot_output.xml
Log:     /Users/tyler/dev/interrupt-renode-test-automation/test_results/log.html
Report:  /Users/tyler/dev/interrupt-renode-test-automation/test_results/report.html
Tests finished successfully :)
```

Awesome. Our simple test was able to launch the firmware, execute the `ping` command, receive a `PONG` response, and generate a report. 

We can also add another test to this same file to test our `help` command, and we can use multiple `Wait For Line On Uart`, one after the other. We can append the following to the end of `test_basic.robot`:

```robot
*** Test Cases ***
Help Menu
    Start Test

    Wait For Prompt On Uart     ${SHELL_PROMPT}
    Write Line To Uart          help

    # Expect two lines
    Wait For Line On Uart       help: Lists all commands      timeout=2
    Wait For Line On Uart       ping: Prints PONG             timeout=2
```

The best place to start for inspiration of the various features of Renode's integration with Robot Framework is to search around the Internet. I've search for "Create Terminal Tester" on [GitHub](https://github.com/search?q=%22Create+Terminal+Tester%22&type=Code) and [grep.app](https://grep.app/search?q=Create%20Terminal%20Tester) and found great examples.

## More Robot Framework Tips & Tricks

### UART Timeouts

To keep tests failing quickly, make sure not to forget adding `timeout=<seconds>` to UART expectations.

```
    Wait For Line On Uart       help: Lists all commands      timeout=2
```

### Regular Expressions on UART

A cool feature that Renode added was the ability to expect regular expressions from the UART, which allows developers to read in values that change between invocations of the test. For example, we can test the following shell command:

```c
int cli_command_greet(int argc, char *argv[]) {
    char buf[64];
    snprintf(buf, sizeof(buf), "Hello %s!", argv[1]);
    shell_put_line(buf);
    return 0;
}
```

By using `treatAsRegex=True` and saving this value to `p`, we can then read it out by using `p.groups[0]` and comparing it to the name provided in the test. 

```robot
*** Test Cases ***
Greet
    Start Test

    Wait For Prompt On Uart         ${SHELL_PROMPT}
    Write Line To Uart              greet Tyler

    ${p}=  Wait For Line On Uart    Hello (\\w+)!     treatAsRegex=true     timeout=2
    Should Be True                  'Tyler' == """${p.groups[0]}"""
```

### Comparison Operations

At Pebble, we used dynamic memory allocations. To help us sleep at night and confirm that we always had enough heap available on every release we shipped, we added tests ensuring our high-water-marks were within acceptable limits. A test we might have written at the time would measure the available heap space after all of the tests had finished. 

```robot
*** Test Cases ***
High Water Mark
    Start Test

    Wait For Prompt On Uart         ${SHELL_PROMPT}
    Write Line To Uart              heap_free

    ${p}=  Wait For Line On Uart    (\\d+)     treatAsRegex=true  timeout=2
    ${i}=  Convert To Integer       ${p.groups[0]}
    
    # Ensure we have 5000 bytes free at the end of the tests
    Should Be True                  5000 < ${i}
```

### Line Endings

If your shell uses carriage returns as line endings, make sure to add the following option when creating the tester:

```
    Create Terminal Tester    ${UART}  endLineOption=TreatCarriageReturnAsEndLine
```

### Tags & Documentation

You can add a **Tags** and **Documentation** attribute to each test, which will help bucket your tests up into logical groups and help developers know what each test is doing. 

```robot
*** Test Cases ***
Command
    [Documentation]             Some command
    [Tags]                      critical  uart  factory
```

![](/img/test-automation-renode/robot-tags.png)

> Adding the tag `non_critical` or `skipped` on a test will allow the test to fail but not mark the entire test run as a failure. You can use this for work-in-progress or flaky tests.

## Github Actions CI & Renode

It's now time to plug things into a continuous integration system for automated testing! In a previous post, we wrote about [building firmware in CircleCI]({% post_url 2019-09-17-continuous-integration-for-firmware %}). This time, we are going to use Github Actions to build and test our firmware, as it's likely the easiest for most people to get up and running. 

To start, we'll want to start off by creating a file `.github/workflows/main.yml` in our repo. The `blank` example from Github's [starter-workflows](https://github.com/actions/starter-workflows/blob/master/ci/blank.yml) gives a quick introduction to how the system works. I've copied it down below for easy reference. 

```yaml
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

```yaml
jobs:
  build_and_test:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v2
      with:                #
        submodules: true   # CHANGED
```

Next, we'll want to download and configure the ARM embedded toolchain since it isn't installed by default. Thankfully, there is already an Action, [`fiam/arm-none-eabi-gcc`](https://github.com/fiam/arm-none-eabi-gcc), that we can use to install the toolchain for us.

```yaml
    steps:
    [...]

    # Get the arm-non-eabi-gcc toolchain   
    - name: Install arm-none-eabi-gcc
      uses: fiam/arm-none-eabi-gcc@v1
      with:
          release: '9-2019-q4' 
```

The last thing we need to do is to invoke `make` itself to start the build. 

```yaml
    steps:
    [...]

    - name: Build Firmware
      run: make -j4
```


### Running Renode in CI

The next thing we need to do is to get Renode working in CI. For the example, this is as simple as running `./docker-test.sh` since the Github Actions Ubuntu runners have Docker already pre-installed.

```yaml
    steps:
    [...]

    - name: Renode Tests
      run: ./docker-test.sh 
```

Since the script `docker-test.sh` was configured to output the test artifacts to the folder `test_results/`, we will want to capture that directory and save it as an artifact in the CI build.

```yaml
    steps:
    [...]

    - name: Upload Output Directory
      uses: actions/upload-artifact@v2
      with:
        name: Renode Test Results
        path: test_results/
```


### Complete Github Action `main.yml`

```yaml
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

Now, when we publish a pull-request on Github, we'll immediately see that the build is triggered and our job starts. 

![](/img/test-automation-renode/github-pr-building.png)

If we click on "Details", we can watch each job complete in real time! Below we see that all jobs are successful and that we have two artifacts. One is the ELF file that was built during the job, and the other is a ZIP archive of the Robot Framework test results.

![](/img/test-automation-renode/github-pr-build.png)

The best part about using the Robot Framework integration of Renode is that it generates pretty HTML-based reports, as we saw in the [Introduction to Renode post]({% post_url 2020-03-23-intro-to-renode %}#renode--integration-tests).

> I've included the HTML report for the above test. [Click here to view it](/misc/test-automation-renode/log.html).



## Final Thoughts



You can find the examples shown in this post
[here](https://github.com/memfault/interrupt-renode-test-automation).

See anything you'd like to change? Submit a pull request or open an issue at
[Github](https://github.com/memfault/interrupt)

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^gnu_toolchain]: [GNU ARM Embedded toolchain for download](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
[^renode_boards]: [Renode Supported Boards](https://renode.readthedocs.io/en/latest/introduction/supported-boards.html)
<!-- prettier-ignore-end -->
