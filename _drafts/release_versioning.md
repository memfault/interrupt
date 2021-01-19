---
title: Firmware Versioning
description:
  Firmware versioning with relelase management in mind
author: tyler
image: img/<post-slug>/cover.png # 1200x630
---

There are good ways to do firmware versioning, and there are bad ways. You want to encode as much information in a version that you can, without breaking many of the conveniences that versions provide us. Versions should be:

- Sortable
- Encode compatibility information about upgrades or downgrades
- Reference a specific "revision" of the software
- Tell us a bit about how major the changeset was

Many of these are captured in the Semantic Versioning, but this is only one piece of the puzzle.

<!-- excerpt start -->

In this article, we are going to talk through all the various pieces of metadata to be including in your firmware binaries, debug symbols, and build artifacts so that developers and machines can quickly and easily know **exactly** which build is which and how to fetch associated files for the debug, test, and release pipeline.

<!-- excerpt end -->

This post builds upon many of our previous posts, but brings it all together under one roof and tells a more cohesive story. When code snippets are captured in a previous post, a link will be provided. 

{% include newsletter.html %}

{% include toc.html %}

## Semantic Versioning

[Semantic Versioning](https://semver.org/), or SemVer for short, is probably the most popular versioning scheme in software today. It was built to solve the versioning story for a single package, but also how a single package interoperates with dependencies. 

SemVer uses a sequence of three digits, MAJOR.MINOR.PATH, and also allows for an optional pre-release tag and build metadata tag. All together, a version looks like:

MAJOR.MINOR.PATCH-[pre-release-tag]+[build-meta-tag]

The major, minor, and patch digits are sorted numerically (1.1.1 > 1.1.0), the pre-release tag is sorted alphanumerically (rc > beta > alpha), and the build-meta-tag is ignored completely when determining sort order or equality (1.1.1+abcd == 1.1.1+efgh).

Fortunately, or maybe unfortunately, we shouldn't just make up our own interpretations of what MAJOR, MINOR, and PATCH mean. We should try to stick to their definitions to help our future selves, co-workers, and engineers trying to integrate with your software.

Although it might seem pedantic, let's dig into when a release lead might want to increment each field. 

### Incrementing MAJOR

Example: 1.0.0 → 2.0.0

SemVer dictates that the MAJOR field should be incremented when "you make incompatible API changes".

For most firmware projects, the MAJOR field might *never* be incremented. If I'm writing firmware for an nRF52, and it is the only MCU in my hardware product, it doesn't have to interact or communicate with many other pieces of software or hardware, so there shouldn't be any incompatible API changes!

There is one ocassion in which I would seriously consider incrementing the MAJOR field, and that is when our firmware becomes **incompatible with itself**. Let me explain. 

Devices perform firmware updates all the time, and your device should be generally be able to update from any firmware version to any other.

If you deploy a version 1.0.0, a version 1.1.0, and a version 1.2.0, your firmware should be able to update from 1.0.0 → 1.1.0 then 1.1.0 → 1.2.0, or it can skip 1.1.0 entirely and jump from 1.0.0 → 1.2.0. This should be handled gracefully! 

However, keeping that backwards compatibility can weigh down our firmware, especially if need to keep migration code in our firmware and we are already low on code space. This is exactly what happened within our firmware at Pebble, the smartwatch company.

The Pebble 2.0.0 firmware was released early 2014. We proceeded to release roughly 10 firmware versions between this time and early 2015. Within all of these versions, we were changing *a lot* of stuff. We completely changed our file system (from Contiki's COFFEE[^coffee] to an internally built one), changed persistent data structures that were stored on internal and external flash, and completely re-architected how we stored our BLE bonding information. 

Despite all of these changes, we made sure to enable our firmware to update from an old 2.0.0 release, to our latest release, 2.9.0, performing all the migrations that would rewrite the data structures and migrate the filesystem.

------
TODO


### Incrementing MINOR

Example: 1.1.0 → 1.2.0

SemVer dictates that the MINOR field should be incremented "when you add functionality in a backwards compatible manner".

In the context of firmawre, this means that your device is able to upgrade from a version with the same MAJOR to any *newer* version with the same MAJOR. For example, as we discussed in the MAJOR section, any firmware in the 2.x release train should be able to update to any newer 2.x version, such as 2.0.0 → 2.5.0. 

What this doesn't allow is *downgrading* firmware versions, which is going from a firmware version to an older one, such as 2.5 → 2.0. Speaking from experience, this is a nightmare to deal with and you should just disable this behavior from ever happening.

We provide a code snippet later in the article to help prevent against downgrades at the installer level.

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

### Detecting Downgrades

One ofthe most unpleasant and avoidable ways of bricking a device is to allow it to install incompatible versions, such as downgrades. If SemVer is following closely, almost all dangerous downgrade paths could be avoided without much logic or defensive work.

Although using `scanf` isn't really recommended due to code-size bloat, here is a simple example snippet of code to detect firmware downgrades. 

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

## Git Revisions

The second piece of metadata that we want to include in our firmware is the Git revision or Git tag, such as `d287bc7311` or `v1.1.1`. 

This is primarily for human consumption, and is not meant to be parsed by code or used in any significant way. 

You can call `git describe` to acquire the Git revision, commit SHA, most recent tag, and how many commits away from the most recent tag you are. 

```
$ git describe --tags --always --dirty="+" --abbrev=10
1.1.1-3-g8bb5d7f+
```

- `1.1.1` - The most recent `tag` in our Git history on our current branch
- `3` - The number of additional commits on top of our parent tag, `1.1.1`
- `g[8bb5d7f162]` - The commit SHA, prefixed with `g`. I like to use 10 characters to represent the commit as the default of 7 is usually not enough to remain unique for larger projects.
- `+` - Denotes that the project directory was "dirty" at the time. This usually means the developer had uncommitted changes when a firmware was built.

### Makefile Changes

```Makefile
GIT_DESCRIBE := \"$(shell $(GIT) rev-parse --short HEAD)\"
CFLAGS += -DGIT_DESCRIBE
```

### Why it isn't enough uniqueness

The one issue with Git revisions is that they are not unique versions! If multiple developers are working of hte same base commit and both build binaries, upload them somewhere, they are different!

## GNU Build ID

- Uniquely identify builds
- Upload build artifacts based on this ID
- GDB Symbol Servers are going to make things easier

### Don't use GCC? Not to worry.

https://github.com/memfault/memfault-firmware-sdk/blob/master/scripts/fw_build_id.py


## Putting it all together

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


<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^reference_key]: [Post Title](https://example.com)
<!-- prettier-ignore-end -->
