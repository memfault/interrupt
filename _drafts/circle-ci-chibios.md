---
title: Build Firmware Projects with CircleCI
description: TODO
author: francois
image: /img/project-cli/project-cli.png
tags: [ci]
---

<!-- excerpt start -->

TODO

<!-- excerpt end -->

{:.no_toc}

## Table of Contents

<!-- prettier-ignore -->
* auto-gen TOC:
{:toc}

## Why Continuous Integration

_Like Interrupt? [Subscribe](http://eepurl.com/gpRedv) to get our latest posts
straight to your mailbox_

## Why CircleCI

- One free runner
- Docker and Linux
- Github first
- Insanely fast, 32 processes at once

## Steps to getting set up

- We fork ChibiOS into the Memfault org
- Login to CircleCI
- Click "Add Projects"
- Search for ChibiOS, click "Set Up Project"
- Follow instructions


![Searching for ChibiOS](img/circle-ci/search_chibios.png)
![Setting up CircleCI Project](img/circle-ci/setup_project.png)
![Sample Build Started](img/circle-ci/sample_build_started.png)
![Sample Build Ended](img/circle-ci/sample_build_ended.png)

![ChibiOS Build Ended](img/circle-ci/real_build_ended.png)
![ChiboOS .elf Artifact Uploaded](img/circle-ci/chibios_artifact.png)

![Github Pull Request Created](img/circle-ci/github_pull_request_created.png)
![Github Pull Request Success](img/circle-ci/github_pull_request_success.png)

![Recommended CircleCI Settings](img/circle-ci/recommended_settings.png)

```
# ~/dev/junk/ChibiOS on git:master x [19:49:13]
$ git commit -m 'Add sample .yml CircleCI file'
[master c347b244e] Add sample .yml CircleCI file
 1 file changed, 16 insertions(+)
 create mode 100644 .circleci/config.yml

# ~/dev/junk/ChibiOS on git:master o [19:49:27]
$ git push memfault master
Enumerating objects: 5, done.
Counting objects: 100% (5/5), done.
Delta compression using up to 8 threads
Compressing objects: 100% (3/3), done.
Writing objects: 100% (4/4), 473 bytes | 473.00 KiB/s, done.
Total 4 (delta 1), reused 0 (delta 0)
remote: Resolving deltas: 100% (1/1), completed with 1 local object.
To github.com:memfault/ChibiOS.git
   fd444de36..c347b244e  master -> master
```


- Copy the sample .yml into repo master and push

```yaml
version: 2
jobs:
  build:
    docker:
      - image: debian:stretch

    steps:
      - checkout

      - run:
          name: Greeting
          command: echo Hello, world.

      - run:
          name: Print the Current Time
          command: date
```

- Click "Start building"
- Screenshot of it building
- Success!
- Now let's go over how to build ChibiOS from the git repo
- We'll use python:3.6.9 image as it has all the necessary packages installed such as git, curl, etc.
- 

```
- run:
    name: Install apt dependencies
    command: sudo apt install p7zip-full gcc-arm-none-eabi binutils-arm-none-eabi
```

```
- run:
    name: Build Demo
    command: |
      cd ext
      7za x fatfs-0.13c_patched.7z
      cd ../demos/STM32/RT-STM32F103-STM3210E_EVAL-FATFS-USB
      make -j 
```


```
- store_artifacts:
    path: ./demos/STM32/RT-STM32F103-STM3210E_EVAL-FATFS-USB/build/ch.elf
```


Full Example
```
version: 2
jobs:
  build:
    docker:
      - image: circleci/python:3.6.9-stretch
    steps:
      - checkout
      - run:
          name: Install apt packages
          command: 'sudo apt install p7zip-full gcc-arm-none-eabi binutils-arm-none-eabi'
      - run:
          name: Build Demo
          command: |
            cd ext
            7za x fatfs-0.13c_patched.7z
            cd ../demos/STM32/RT-STM32F103-STM3210E_EVAL-FATFS-USB
            make -j 
      - store_artifacts:
          path: ./demos/STM32/RT-STM32F103-STM3210E_EVAL-FATFS-USB/build/ch.elf
```

## Configuring our Project

- Go to settings
- Under Build Settings / Advanced Settings
  - "Only build pull requests" -> On
  - "Auto-cancel redundant builds" -> On
  - "Enable pipelines" -> optional, but suggested

## Testing Locally

```
$ brew install circleci
$ circleci config validate
$ circleci local execute
```


## Updating with paralell make build

```
# ~/dev/junk/ChibiOS on git:test-pr x [20:16:48]
$ git add .

# ~/dev/junk/ChibiOS on git:test-pr x [20:16:54]
$ git commit -m 'Enable parallel builds with make -j'
[test-pr 2ee687647] Enable parallel builds with make -j
 1 file changed, 1 insertion(+), 1 deletion(-)

# ~/dev/junk/ChibiOS on git:test-pr o [20:16:55]
$ gb -M 'enable-parallel-builds'

# ~/dev/junk/ChibiOS on git:enable-parallel-builds o [20:17:06]
$ gst
On branch enable-parallel-builds
nothing to commit, working tree clean

# ~/dev/junk/ChibiOS on git:enable-parallel-builds o [20:17:07]
$ git push memfault enable-parallel-builds
Enumerating objects: 7, done.
Counting objects: 100% (7/7), done.
Delta compression using up to 8 threads
Compressing objects: 100% (3/3), done.
Writing objects: 100% (4/4), 374 bytes | 374.00 KiB/s, done.
Total 4 (delta 2), reused 0 (delta 0)
remote: Resolving deltas: 100% (2/2), completed with 2 local objects.
remote:
remote: Create a pull request for 'enable-parallel-builds' on GitHub by visiting:
remote:      https://github.com/memfault/ChibiOS/pull/new/enable-parallel-builds
remote:
To github.com:memfault/ChibiOS.git
 * [new branch]          enable-parallel-builds -> enable-parallel-builds
```


## Final Thoughts

I've thoroughly enjoyed using Invoke in my previous and current job. In
Memfault's codebase, we have around 100 tasks, most of which are namespaced
under general top-level modules. This provides our team with a centralized place
for all of the common tasks, such as building the firmware SDK, publishing
documentation, running automated tests on our devices, pushing new versions of
our service, performing database migrations...everything. The self-documenting
nature of the commands is indispensable.

_All the code used in this blog post is available on
[Github](https://github.com/memfault/interrupt/tree/master/example/invoke-basic/).
See anything you'd like to change? Submit a pull request!_

{:.no_toc}

### Tips & Further Reading


## Reference & Links

[^1]: [Title](link)
