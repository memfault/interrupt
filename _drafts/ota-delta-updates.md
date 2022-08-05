---
title: Saving bandwidth with delta OTA updates
description:
  Post Description (~140 words, used for discoverability and SEO)
author: francois
image: img/<post-slug>/cover.png # 1200x630
tags: [firmware-update]
---

Firmware update capability has become a must-have for most devices. Whether 
to add new features after launch, fix bugs, or urgently patch a security
hole, firmware update gives modern teams the flexibility they need to move fast
and react to a changing environment.

I've written at lenght about firmware update in the past, including on Interrupt
with a [Firmware Update Cookbook]({% post_url 2020-06-23-device-firmware-update-cookbook %})
and [a post about code signing](% post_url
2020-09-08-secure-firmware-update-with-code-signing %}), and even recorded a
[webinar on the
topic](https://memfault.com/webinars/device-firmware-update-best-practices/).

Since then, I've heard from some of you that they've run into roadblocks
implementing firmware update on bandwidth-limited devices. Indeed devices
connected over low bandwidth links like 900MHz wireless or LoRaWAN often
struggle to download a full firmware image in any reasonable time span.

One solution in this case is to implement _delta updates_, a technique which
allows devices to download only the bits and pieces they need rather than full
system images.

<!-- excerpt start -->

In this post, we introduce the concept of delta (i.e. partial) firmware updates,
explain why you may want to implement them in your projects, and go through an
implementation from top to bottom.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Delta Updates Explained

> Note: the next few sections provide an overview of delta updates, why it
> matters, and how it works. If you'd like to skip straight to implementation,
> click
> [here](#delta-updates-implementation).

The concept behind delta updates is intuitive. When we put together a new
version of our firmware, we do not change every single line of code. Instead, we
edit specific sections of the code to change the behavior of the code and
introduce a new feature or fix a bug.

This means that our new firmware is largely made up of the old firmware, plus a
few changes.

<img width=600px src="{% img_url fwup-delta/new-code-old-image.png %}" />

Wouldn't it be neat if we could collect only the code changes and send those
down to the device, rather than the full firmware update? This is exactly what
delta updates allow us to do.

The process is broken up into two steps. First, we need to extract the bits that
have changed from our new image, we'll call it DIFF.

<img width=600px src="{% img_url fwup-delta/new-code-DIFF.png %}" />

Then we reconstruct our image from our original firmware and our delta image,
which we'll call PATCH.

<img width=700px src="{% img_url fwup-delta/new-code-PATCH.png %}" />

### Pros and Cons of Delta Updates

The obvious advantage of delta updates is the small size of the resulting image.
Delta images are often one to two orders of magnitude smaller than full system
images. The reduction in size has multiple beneficial effects:

1. OTA updates become possible over very low bandwidth links (e.g. LoRaWAN).
2. Updates are much faster over low bandwidth links. For example, a 10MB image
   may take over 15 minutes to download over a BLE connection to a mobile phone,
   even at [peak throughput]({% post_url 2019-09-24-ble-throughput-primer %}). A
   delta update would take less than 1 minute to download, leading to a much
   better customer experience and less risk of power loss mid-update.
3. The lifetime of flash memory can be extended, as fewer writes are needed to
   install a delta image than a full image.
4. OTA consumer less power, thanks to the reduced communication and flash
   writing required.

However, there is no such thing as a free lunch! Delta updates come with some
downsides worth considering:

1. More complexity! Your firmware update code now needs to implement the
   patching algorithm, which creates more opportunities for bugs.
2. Delta updates requires more code space to implement the patching algorithm,
   so if you are constrained already it may not be appropriate for your project.
3. Managing delta updates creates more complexity on the backend and on the
   release process.

That last point bears elaborating a bit. Unlike all system images, each delta
image is only compatible with a single original version. In other words, while a
system image for firmware v1.2.0 can upgrade any prior version to v1.2.0, a
delta update for "v1.1.0→v1.2.0" can only upgrade devices running v1.1.0 to
v1.2.0.

This means that your OTA backend needs to be sophisticated enough to present
delta updates when devices are running compatible versions, and in all other
cases present a full system update. And each firmware releases requires you to
compile and upload a number of delta images for your versions in the field.

> Our OTA backend at [Memfault](https://memfault.com) supports delta updates!
> You can read more about in our
> [documentation](https://docs.memfault.com/docs/platform/ota/#delta-releases).

### Jojodiff

One of the key components of a delta update system is a binary diff and patch
system. There are remarkably few libraries that provide this functionality. The
excellent BSDiff[^bsdiff], and XDelta[^xdelta] both require too much memory to
work on most embedded systems. This leaves Jojodiff[^jojodiff], which has been
helpfully reimplemented by Jan Longboom[^jan] in his JanPatch library[^janpatch] optimized
for embedded systems.

While Jojodiff is neither fastest nor most efficient, it requires constant
space, linear time complexity, and can patch a file in-place. These features
make it well suited to limited environments such as MCU-based devices.

## Delta Updates Implementation

### Setup

### Building our Delta Updates

### Implementing JanPatch

## Conclusion



<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^reference_key]: [Post Title](https://example.com)
[^bsdiff]: [BSDiff](https://www.daemonology.net/bsdiff/)
[^xdelta]: [XDelta](https://sourceforge.net/projects/xdelta/)
[^jan]: [Jan Jongboom](http://janjongboom.com/) is the co-founder and CTO at Edge Impulse
[^janpatch]: [JanPatch on Github](https://github.com/janjongboom/janpatch)
<!-- prettier-ignore-end -->
