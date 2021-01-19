---
title: Firmware Versioning
description:
  Firmware versioning with relelase management in mind
author: tyler
image: img/<post-slug>/cover.png # 1200x630
---

Release versioning might seem like a boring topic. Honestly, it should be. There should only be a couple of right ways to do versioning, and each project should pick one of the agreed upon methods (SemVer[^semver], CalVer[^calver], etc.)  that makes the most sense to the project. We don't live in this ideal world, and many projects choose to deviate from the standards. I'm not here to say they are wrong, but I'd like to discuss what you might be missing out on!

On top of a standard versioning scheme, it's useful to include extra metadata about a given release, such as the Git revision, [GNU Build ID]({% post_url 2019-05-29-gnu-build-id-for-firmware %}), build timestamp, and other related items. With all of this in place, both machines and developers can quickly hone in on *exactly* which release is running on a given device.

<!-- excerpt start -->

In this article, we are going to talk through all the various pieces of metadata to be including in your firmware binaries, debug symbols, and build artifacts so that developers and machines can quickly and easily know **exactly** which build is which and how to fetch associated files for the debug, test, and release pipeline.

<!-- excerpt end -->

This post builds upon many of our previous posts and brings all the relevant information together under one post.

{% include newsletter.html %}

{% include toc.html %}


## Overview

Versions don't exist just to differentiate one release from another. They serve other purposes! Such purposes include, but are not limited to:

- Providing an ordering to versions (e.g. 1.1 > 1.0)
- Encode compatibility information, breaking changes, and upgrade/downgrade restrictions
- Reference a specific "revision" of the software
- Reference a specific "build" of the software
- Tell users when *major* changes took place (e.g. Windows 8 → Windows 10)

If a project chooses to use just a Git SHA, or a build timestamp, or a random ASCII name, then many or all of the above benefits that versions typically provide are lost. We can do better. 

Throughout this article, we'll talk about the various pieces that I believe provide a hollistic picture of the software that is packaged into a release and running on a device. First, let's talk about Semantic Versioning.

## Semantic Versioning

[Semantic Versioning](https://semver.org/), or SemVer for short, is likely the most popular versioning scheme in software today. It was built to solve the versioning story for a single package, but also how a single package interoperates with dependencies. 

SemVer uses a sequence of three digits, MAJOR.MINOR.PATH, and also allows for an optional pre-release tag and build metadata tag. All together, a version looks like:

```
MAJOR.MINOR.PATCH-[pre-release-tag]+[build-meta-tag]
```

The MAJOR, MINOR, and PATCH digits are sorted numerically (1.1.11 > 1.1.10), the pre-release tag is sorted alphanumerically (rc > beta > alpha), and the build-meta-tag is ignored completely when determining sort order or equality (1.1.1+abcd == 1.1.1+efgh). Each should be incremented when the following occurs, according to the spec:

- MAJOR when you make incompatible API changes
- MINOR when you add functionality in a backwards compatible manner
- PATCH when you make backwards compatible bug fixes

Although it might seem pedantic, let's dig into when a release lead might want to increment each field when applied to a firmware project.

### Incrementing MAJOR

Example: 1.0.0 → 2.0.0

For most firmware projects, the MAJOR field might *never* be incremented. If I'm writing firmware for an nRF52, and it is the only MCU in my hardware product, it doesn't have to interact or communicate with many other pieces of software or hardware, so there shouldn't be any incompatible API changes!

There is one ocassion where I would seriously consider incrementing the MAJOR field, and that is when our firmware becomes **incompatible with a version of itself**. Let me explain. 

Normal software packages can be updated at any time, with almost no repercussions. If incompatibility arises, just revert the change and continue. However, on firmware devices, the firmware updates itself. If there is an incompatibility, it might require a re-install, a factory reset, or in the worst of cases, the device might be bricked!

With firmware updates, you also can't expect every device to install every single version (unless that is actually a hard requirement of the update flow). If you deploy a version 1.0.0, a version 1.1.0, and a version 1.2.0, your firmware should be able to update from 1.0.0 → 1.2.0 and skip 1.1.0 entirely. If a user puts your device in their nightstand and pulls it out 6 months later, this is exactly what should happen.

However, keeping that backwards compatibility can weigh down our firmware, especially if we need to keep migration code in our firmware. Migration code adds complexity, code space, and allowing installs from multiple firmware versions increases our QA test matrix dramatically. 

Let's take the Pebble Smartwatch firmware as an example. The Pebble 2.0.0 firmware was released early 2014. Over the next year, we released versions all the way up to 2.9.1. Within all of these versions, we were changing *a lot* of stuff. We changed our file system (from Contiki's Coffee[^coffee] to an internally built one), re-wrote persistent data structures that were stored on internal and external flash, and completely re-architected how we stored our BLE bonding information. All of these things were critical to the functionality of our firmware, and at no point could we lose any of this data.

Despite all of these changes, a user could update directly from 2.0.0 to 2.9.1, and the boot-up sequence on the latter firmware would perform all the migrations (that was quite a long boot-up time).

When we were looking to revamp our firmware for a new product update, we decided to throw out all of the migration code from the 2 years before and move it into a "migration firmware" which users had to update through. With this, our 3.x set of firmwares had a clean slate with no backwards compatibility requirements! It was a breath of fresh air, and our firmware was thousands of lines of code smaller.

This was a backward-incompatible firmware update, and I believe it warranted the MAJOR increase from 2 to 3.

### Incrementing MINOR

Example: 1.1.0 → 1.2.0

In the context of firmawre, this is likely what most of your firmware version updates will be, especially if they contain new features and functionality. A MINOR update means that your device is able to upgrade from a version with the same MAJOR to any *newer* version with the same MAJOR. For example, as we discussed in the MAJOR section, any firmware in the 2.x release train should be able to update to any newer 2.x version, such as 2.0.0 → 2.5.0. 

A MINOR update should not allow *downgrading* firmware versions, which is going from a firmware version to an older one, such as 2.5 → 2.0. Speaking from experience, this is a nightmare to deal with, and you should just disable this behavior from ever happening. We provide a [code snippet](#detecting-downgrades) later in the article to help prevent against downgrades at the installer level.

### Incrementing PATCH

Example: 1.1.0 → 1.1.1

Last, SemVer says PATCH updates should be made "when you make backwards compatible bug fixes".

On past firmware projects that I've worked on, PATCH updates are usually hot-fixes, which are quickly tested and released after a critical bug is discovered upon releasing a prior firmware update. Each patch-fix might fix one or two bugs, and the amount of time needed to test and verify that the firmware works can be minimized. 

Ideally, nothing that should prevent a downgrade from occurring should be placed in a patch upgrade. This means no persistent data structure changes, no migration paths, and no changes in how the device communicates with external devices or services. 

Since there are minimal to no breaking changes, these firmware updates can be upgraded and downgraded to without harm.

### Pre-release Tag Best Practices

The pre-release tag is any series of dot separated identifiers consisting of ASCII letters and numbers.

The most common pre-release tags are 'alpha', 'beta', and 'rc', which should satisfy most of your versioning requirements. 

- Use 'alpha' when the release branch is first cut from master or from another release branch. It's pretty much assumed that 'alpha' builds are unstable and should be treated as broken-by-default.
- Use 'beta' when the API is stable, there are no longer any breaking changes and features being pushed into the release, and the stabilization phase has begun.
- Use 'rc' when you think everything is stable and QA should start doing rigorous testing, because it might turn our that this build could be "the one".

I've seen many developers create out-of-order pre-release tags, so be sure to [read the spec](https://semver.org/#spec-item-11) and test your assumptions. 

### Scripting and Testing SemVer

In all of my scripts that parse SemVer and when I need to quickly test version comparisons, I turn to the [python-semver](https://github.com/python-semver/python-semver) package.

```
$ pip install semver
$ python
>>> semver.parse_version_info('1.1.1-alpha.1') \
    < semver.parse_version_info('1.1.1-alpha.11')
True
```

### Mising Pieces of Semantic Versioning

Semantic Versioning provides a plethora of benefits to our firmware and release management workflows, but it doesn't solve all of our requirements. One of our requirements was to be able to be able to reference a specific build of a firmware, which SemVer does not help with.

If two developers both build a 1.1.1 firmware, we now have two builds of 1.1.1 with no way to tell the difference between the two! They could be based upon two entirely different revisions of software or even built with a different compiler. We could use pre-release tags or build meta tags, but we also don't want to pollute our SemVer string with data that could be better solved by other versions. 

Let's now look at Git revisions, which will help us identify the software revisions that our firmware is built from.

## Git Revisions

The second piece of metadata that we want to include in our firmware is a Git revision or Git tag, such as `d287bc7311`, `1.1.1-gd287bc7311`, or `1.1.1`, depending on how you like your tags.

By having the Git revision or tag baked into the firmware, we are able to determine which software the binary is based upon, which would help us when we are trying to triage bugs or build old versions. The git revision is primarily for human consumption and is not meant to be parsed by code or used in any significant way. 

You can call `git describe` to acquire the Git revision, commit SHA, most recent tag, and how many commits away from the most recent tag you are. 

```
$ git describe --tags --always --dirty="+" --abbrev=10
1.1.1-3-g8bb5d7f+
```

- `1.1.1` - The most recent `tag` in our Git history on our current branch
- `3` - The number of additional commits on top of our parent tag, `1.1.1`
- `g[8bb5d7f162]` - The commit SHA, prefixed with `g`. I like to use 10 characters to represent the commit as the default of 7 is usually not enough to remain unique for larger projects.
- `+` - Denotes that the project directory was "dirty" at the time. This usually means the developer had uncommitted changes when a firmware was built.

Refer to the `git describe` [documentation](https://git-scm.com/docs/git-describe) for more information.

### Missing Pieces of the Git Revision

The Git revision, just like the SemVer version, is missing the ability to identify unique builds of the firmware. Two developers could each build the Git revision `d287bc7311` and push binaries to an update server and we'd have no way of knowing they are two different builds, or know how to tie them back to the original source. 

We need yet another piece of build information! Last but not least, let's chat about GNU Build ID's.

## GNU Build ID

GNU Build ID's serve three purposes:

1. To uniquely identify and differentiate a specific firmware version from other ones.
2. Conversely, to verify that two binaries are in fact the same build.
3. To match debug symbols to a given binary.



- Uniquely identify builds
- Upload build artifacts based on this ID
- GDB Symbol Servers are going to make things easier

### Don't use GCC? Not to worry.

https://github.com/memfault/memfault-firmware-sdk/blob/master/scripts/fw_build_id.py


## Bringing It All Together

### Adding Git Revision to Makefile

```Makefile
GIT_REVISION := \"$(shell git describe --tags --always --dirty="+" --abbrev=10)\"
CFLAGS += -DGIT_REVISION
```

From here, you can create a `.c` file which uses the macro `GIT_REVISION` and it will be populated with the output of the shell command.

### Build binaries with the versions

Now that we've established the various versions and metadata that we need to use to satisfy our requirements, we can now put 

François, in his [Device Firmware Update Cookbook]({% post_url 2020-06-23-device-firmware-update-cookbook %}) post, describes how to [include metadata]({% post_url 2020-06-23-device-firmware-update-cookbook#image-metadata %}) in both ELF files and loadable firmware binaries.

With the version in the ELF file, we can investigate the metadata within GDB:

```
$ arm-none-eabi-gdb-py --se build/fwup-example-app.elf
Reading symbols from build/fwup-example-app.elf...
(gdb) p image_hdr
$1 = {
  image_magic = 51966,
  image_hdr_version = 1,
  crc = 0,
  data_size = 0,
  image_type = 2 '\002',
  version_major = 1 '\001',
  version_minor = 0 '\000',
  version_patch = 1 '\001',
  vector_addr = 134348832,
  reserved = 0,
  git_sha = "70859a3"
}
```

And when working with our `.bin` files, we can print the metadata using Python's `struct` package.

```python
def print_metadata(bin_filename):
    IMAGE_HDR_SIZE_BYTES = 32

    with open(bin_filename, "rb") as f:
        image_hdr = f.read(IMAGE_HDR_SIZE_BYTES)
        data = f.read()

    image_magic, image_hdr_version, \
        crc, data_size, image_type, \
        major, minor, patch, \
        vector_addr, reserved, \
        git_sha = struct.unpack("<HHLLBBBBLL8s", image_hdr)

    print("Version: {}.{}.{}".format(major, minor, patch))
    print("Git SHA: {}".format(git_sha.decode('utf-8')))
```

```
$ python ~/print_header.py build/fwup-example-app.bin
Version: 1.0.1
Git SHA: 70859a3
```

### Detecting Downgrades

One of the most unpleasant and avoidable ways of bricking a device is to allow it to install incompatible versions, such as downgrades. If SemVer is following closely, almost all dangerous downgrade paths could be avoided without much logic or defensive work.

Although using `scanf` isn't really recommended due to code-size bloat, here is a simple example snippet of code to detect firmware downgrades for those that use Semantic Versioning. 

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char *current_ver = "1.1.0";
    char *new_ver = "1.0.0";

    int currentMajor, currentMinor, currentPatch;
    int newMajor, newMinor, newPatch;

    sscanf(current_ver, "%d.%d.%d", &currentMajor, &currentMinor, &currentPatch);
    sscanf(new_ver, "%d.%d.%d", &newMajor, &newMinor, &newPatch);
    
    if (newMajor < currentMajor || newMinor < currentMinor) {
        // Dowgrade. Do not perform the "update".
    }
}
```

If you use another versioning scheme, you should still try to detect downgrades, whether that is by using the Git SHA, a timestamp, commit number, or anything else that is roughly stable and sequential.


<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^semver]: [Semantic Versioning 2.0](https://semver.org/)
[^calver]: [Calendar Versioning](https://calver.org/)
[^coffee]: [Contiki Coffee filesystem](https://anrg.usc.edu/contiki/index.php/Contiki_Coffee_File_System)
<!-- prettier-ignore-end -->
