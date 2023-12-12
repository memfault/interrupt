---
title: Proper Release Versioning Goes a Long Way
description:
  Include standardized versions and other build metadata such as Git revisions and GNU Build ID's into firmware binaries and debug symbols.
author: tyler
image: img/release-versioning/cover.png
tags: [git, best-practices]
---

Release versioning might seem like a boring topic. Honestly, it should be. There should only be a couple of right ways to do versioning, and each project should pick one of the agreed-upon methods (SemVer[^semver], CalVer[^calver], etc.) that makes the most sense to the project. We don't live in this ideal world unfortunately and many projects choose to deviate from versioning standards. I'm not here to say they are wrong, but I'd like to discuss what they might be missing out on.

Along with a typical release version, it is useful to include extra metadata, such as the Git revision, [GNU Build ID]({% post_url 2019-05-29-gnu-build-id-for-firmware %}), build timestamp, and other related items. With all of this in place, both machines and developers can quickly figure out *exactly* which release is running on a given device, what the source is, where it came from, and how to rebuild it if necessary.

<!-- excerpt start -->

In this article, we are going to talk through all the various pieces of metadata to be included in your firmware binaries, debug symbols, and build artifacts so that developers and machines can quickly and easily know **exactly** which build is which and how to fetch associated files for the debug, test, and release pipeline.

<!-- excerpt end -->

This post builds upon many of our previous posts and brings all the relevant information together under one post.

{% include newsletter.html %}

{% include toc.html %}

## Overview

Versions don't exist just to differentiate one release from another. They serve other purposes! Such purposes include, but are not limited to:

- Providing an ordering to versions (e.g. 1.1 > 1.0)
- Encode compatibility information, breaking changes, and upgrade/downgrade restrictions
- Reference a specific revision of the software
- Reference a specific build of the software
- Tell users when *major* changes took place (e.g. Windows 8 → Windows 10)

If a project chooses to use just a Git SHA, or a build timestamp, or a random ASCII name, then many or all of the above benefits that versions typically provide are lost. We can do better!

Throughout this article, we'll talk about the various pieces that I believe provide a holistic picture of the software that is packaged into a release and running on a device. First, let's talk about Semantic Versioning.

## Semantic Versioning

[Semantic Versioning](https://semver.org/), or SemVer for short, is likely the most popular versioning scheme in software today. It was built to solve the versioning story for a single package, but also how a single package interoperates with dependencies.

SemVer uses a sequence of three digits, MAJOR.MINOR.PATCH, and also allows for an optional pre-release tag and build metadata tag. Altogether, a version looks like:

```
MAJOR.MINOR.PATCH-[pre-release-tag]+[build-meta-tag]
```

The MAJOR, MINOR, and PATCH digits are sorted numerically (1.1.11 > 1.1.10), the pre-release tag is sorted alphanumerically (rc > beta > alpha), and the build-meta-tag is ignored completely when determining sort order or equality (1.1.1+abcd == 1.1.1+efgh). Each should be incremented when the following occurs, according to the spec:

- MAJOR when you make incompatible API changes
- MINOR when you add functionality in a backward-compatible manner
- PATCH when you make backward-compatible bug fixes

Although it might seem pedantic, let's dig into when a release lead might want to increment each field for a firmware project.

### Incrementing MAJOR

Example: 1.0.0 → 2.0.0

For most firmware projects, the MAJOR field might *never* be incremented. If I'm writing firmware for an nRF52, and it is the only MCU in my hardware product, it doesn't have to interact or communicate with many other pieces of software or hardware, so there shouldn't be any incompatible API changes!

There is one occassion where I would seriously consider incrementing the MAJOR field, and that is when our firmware becomes **incompatible with a version of itself**. Let me explain.

Normal software packages can be updated at any time, with almost no repercussions. If incompatibility arises, just revert the change and continue. However, on firmware devices, the firmware updates itself. If there is an incompatibility, it might require a re-install, a factory reset, or in the worst of cases, the device might be bricked!

With firmware updates, you also can't expect every device to install every single version (unless that is a hard requirement of the update flow). If you deploy version 1.0.0, version 1.1.0, and version 1.2.0, your firmware should be able to update from 1.0.0 → 1.2.0 and skip 1.1.0 entirely. If a user puts your device in their nightstand and pulls it out 6 months later, this is exactly what should happen.

However, keeping that backward compatibility can weigh down our firmware, especially if we need to keep migration code in our firmware. Migration code adds complexity, code space, and increases our QA test matrix dramatically.

Let's take the Pebble Smartwatch firmware as an example. The Pebble 2.0.0 firmware was released in early 2014. Over the next year, we released versions all the way up to 2.9.1. Within all of these versions, we were changing *a lot* of stuff. We changed our file system (from Contiki's Coffee[^coffee] to an internally built one), re-wrote persistent data structures that were stored on internal and external flash, and completely re-architected how we stored our BLE bonding information. All of these things were critical to the functionality of our firmware, and at no point could we lose any of this data.

Despite all of these changes, a user could update directly from 2.0.0 to 2.9.1, and the boot-up sequence on the latter firmware would perform all the migrations (that was quite a long boot-up time).

When we were looking to revamp our firmware for a new product update, we decided to throw out all of the migration code from the 2 years before and move it into a "migration firmware" which users had to update through. With this, our 3.x set of firmwares had a clean slate with no backward-compatibility requirements! It was a breath of fresh air, and our firmware was thousands of lines of code smaller.

This was a backward-incompatible firmware update, and I believe it warranted the MAJOR increase from 2 to 3.

### Incrementing MINOR

Example: 1.1.0 → 1.2.0

In the context of firmware, this is likely what most of your firmware version updates will be, especially if they contain new features and functionality. A MINOR update means that your device can upgrade from a version with the same MAJOR to any *newer* version with the same MAJOR. For example, as we discussed in the MAJOR section, any firmware in the 2.x release train should be able to update to any newer 2.x version, such as 2.0.0 → 2.5.0.

A MINOR update does not need to allow downgrading firmware versions, which is going from a firmware version to an older one, such as 2.5 → 2.0. Speaking from experience, this is a nightmare to deal with, and you should disable this behavior from ever happening, *at least in production*. Allowing downgrades in a limited form on pre-release software will help your developers and QA teams test the upgrade path between two versions more easily.

We provide a [code snippet](#detecting-downgrades) later in the article to help prevent against downgrades at the installer level.

### Incrementing PATCH

Example: 1.1.0 → 1.1.1

On past firmware projects that I've worked on, PATCH updates are usually hot-fixes, which are quickly tested and released after a critical bug is discovered upon releasing a prior firmware update. Each patch-fix might fix one or two bugs, and the amount of time needed to test and verify that the firmware works can be minimized.

Ideally, nothing that should prevent a downgrade from occurring should be placed in a patch upgrade. This means no persistent data structure changes, no migration paths, and no changes in how the device communicates with external devices or services.

Since there are minimal to no breaking changes, these firmware updates can be upgraded and downgraded to without harm.

### Pre-release Tag Best Practices

The pre-release tag is any series of dot separated identifiers consisting of ASCII letters and numbers.

The most common pre-release tags are 'alpha', 'beta', and 'rc', which should satisfy most of your versioning requirements.

- Use 'alpha' when the release branch is first cut from master or another release branch. It's pretty much assumed that 'alpha' builds are unstable and should be treated as broken-by-default.
- Use 'beta' when the API is stable, there are no longer any breaking changes and features being pushed into the release, and the stabilization phase has begun.
- Use 'rc' (release-candidate) when you think everything is stable and QA should start doing rigorous testing because it might turn out that this build could be "the one".

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

### Missing Pieces of Semantic Versioning

Semantic Versioning provides a plethora of benefits to our firmware and release management workflows, but it doesn't solve all of our requirements. One of our requirements was to be able to reference a specific build of a firmware, which SemVer does not help with.

For example, if two developers locally compile a "1.1.1" firmware, we now have two builds of "1.1.1" with no way to tell the difference between the two! They could be based upon two entirely different revisions of software or even built with a different compiler. We could use pre-release tags or build meta tags, but we also don't want to pollute our SemVer string with data that could be better solved by other versions.

Let's now look at Git revisions, which will help us identify the software revisions that our firmware is built from.

## Git Revisions

The second piece of metadata that we want to include in our firmware is a Git revision or Git tag, such as `d287bc7311`.

By having the Git revision baked into the firmware, we can determine which software the binary is based upon, which would help us when we are trying to triage bugs by performing a [git bisect]({% post_url 2020-04-21-git-bisect %}) or rebuilding old firmwares (provided we have [reproducible builds]({% post_url 2019-12-11-reproducible-firmware-builds %})).

You can call `git describe` to acquire the Git revision SHA with a `+` on the end if the project had a dirty state (local modifications) when called.

```
$ git describe --match ForceNone --abbrev=10 --always --dirty="+"
8bb5d7f162+
```

The use of `--match ForceNone` is because I just want the SHA and the dirty signifier and nothing else.

If you'd like to also include a tag and number of commits from the last tag like some projects have, you can use the following command:

```
$ git describe --tags --always --dirty="+" --abbrev=10
1.2.3-18-gdcf62a8fff
```

Refer to the `git describe` [documentation](https://git-scm.com/docs/git-describe) for more information.

### Adding Git Revision

To add a Git SHA to a firmware, we do the work within our build system or Makefile in our case.

```Makefile
GIT_REVISION := \"$(shell git describe --match ForceNone --abbrev=10 --always --dirty="+")\"
DEFINES += GIT_REVISION=$(GIT_REVISION)
CFLAGS += $(foreach d,$(DEFINES),-D$(d))
```

From here, you can create a file that uses the macro `GIT_REVISION` and it will be populated with the output of the shell command.

### Missing Pieces of the Git Revision

The Git revision, just like the SemVer version, is missing the ability to identify unique builds of the firmware. Two developers could each build the Git revision `d287bc7311` and push binaries to an update server and we'd have no way of knowing they are two different builds or know how to tie them back to the source.

We need yet another piece of build information! Last but not least, let's chat about GNU Build ID's.

## GNU Build ID

[GNU Build ID's]({% post_url 2019-05-29-gnu-build-id-for-firmware %}) are our final missing piece to our versioning story. The build ID is a 160-bit SHA1 string computed over the elf header bits and section contents in the file. Because it's computed over the binary itself, we can be sure that every unique binary will produce a unique build ID. Perfect!

A build ID serves three purposes:

1. To uniquely identify and differentiate a specific firmware version from other ones.
2. Conversely, to verify that two binaries are the same exact build.
3. To match debug symbols to a given binary.

The specifics on how to add a GNU Build ID are laid out in our [previous post]({% post_url 2019-05-29-gnu-build-id-for-firmware %}#adding-the-gnu-build-id-to-your-builds).

If you don't use GCC as your compiler, fret not! We have [a script in the Memfault Firmware SDK repo](https://github.com/memfault/memfault-firmware-sdk/blob/master/scripts/fw_build_id.py) to help out a number of our customers include a unique build identifier for their firmware builds, regardless of compiler, build system, or platform.

The future looks promising for the use of GNU Build ID's as well. A new symbol server, [debuginfod](https://sourceware.org/elfutils/Debuginfod.html) was recently released, and various tools are integrating it as a way to pull down debug symbols for binaries that include a build ID.

## Bringing It All Together

Now that we've discussed each of the three versioning items that I like to include in my firmware projects, we should now discuss how to do the deed.

### Setting MAJOR.MINOR.PATCH

Ultimately, you will want to set the MAJOR, MINOR, and PATCH numbers in macros. Whether these are defined using an auto-generated header file or defined as a CFLAG using `-D` is up to you. Where these values come from is the more exciting bit.

#### Git Tags & CI

If you have your [continuous integration]({% post_url 2019-09-17-continuous-integration-for-firmware %}) system set up to build your firmware, pulling the SemVer from the Git tag is a great way to go.

I like to configure my CI systems to watch tags that start with `release-*`. That way, I can tag and push releases with a few simple Git commands and a script in CI which parses the SemVer and sets build variables

```
$ git checkout release-1.0
$ git tag 1.0.0
$ git push origin 1.0.0
```

Before CI builds this tag, it will pull out 1.0.0 from the tag name, set a few CFLAGS (`FW_VERSION_MAJOR`, `FW_VERSION_MINOR`, etc.), and build the firmware.

#### Manually Set Versions

Another way I've seen versions get built is by setting a `fw_version.h` header manually.

```
// fw_version.h

#define FW_VERSION_MAJOR 1
#define FW_VERSION_MINOR 0
#define FW_VERSION_PATCH 0
```

A developer would make this change in a commit on its own and push it to a branch for CI to build.

### Baking Versions Into Binary Header

François, in his [Device Firmware Update Cookbook]({% post_url 2020-06-23-device-firmware-update-cookbook %}) post, describes how to [include metadata]({% post_url 2020-06-23-device-firmware-update-cookbook %}#image-metadata) in both ELF files and into a header of loadable firmware binaries.

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

One of the most unpleasant and avoidable ways of bricking a device is to allow it to install incompatible versions, such as downgrades. If SemVer is followed closely, almost all dangerous downgrade paths could be avoided without much logic or defensive work.

Although using `sscanf` isn't really recommended due to code-size bloat, here is a simple example snippet of code to detect firmware downgrades for those that use Semantic Versioning.

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char *current_ver = "1.1.0";
    char *new_ver = "1.0.0";

    int currentMajor = 0, currentMinor = 0, currentPatch = 0;
    int newMajor, newMinor, newPatch;

    sscanf(current_ver, "%d.%d.%d", &currentMajor, &currentMinor, &currentPatch);
    sscanf(new_ver, "%d.%d.%d", &newMajor, &newMinor, &newPatch);

    if (newMajor < currentMajor || newMinor < currentMinor) {
        // Downgrade detected. Do not perform the "update".
    }
}
```

If you use another versioning scheme, you should still try to detect downgrades, whether that is by using, a timestamp, commit number, or anything else that is roughly stable and sequential.

## Conclusion

Release versioning is one of those things that is only really thought about after the fact, and to me, feels similar to using better commit messages. It's only when I'm months into a project that I realize that I have code, branches, builds, symbol files, and patches all over the place with no organization.

By adhering to versioning standards and by including a Git SHA and build ID, many issues can be avoided and your developers can stop wasting time tracking down various builds and symbol files.

If you have strong thoughts on how firmware releases should be versioned, I'd love to hear them [on the Interrupt Slack](https://interrupt-slack.herokuapp.com/)!

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## Further Reading

- [Embedded Artistry - Giving Your Firmware Build a Version](https://embeddedartistry.com/blog/2016/12/21/giving-your-firmware-build-a-version/)
- [Wolfram Rösler - Build Number Generation With git And cmake](https://gitlab.com/wolframroesler/version)

## References

<!-- prettier-ignore-start -->
[^semver]: [Semantic Versioning 2.0](https://semver.org/)
[^calver]: [Calendar Versioning](https://calver.org/)
[^coffee]: [Contiki Coffee filesystem](https://anrg.usc.edu/contiki/index.php/Contiki_Coffee_File_System)
<!-- prettier-ignore-end -->
