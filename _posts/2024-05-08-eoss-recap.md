---
title: Embedded Open Source Summit 2024 Recap
description:
  A discussion of our favorite talks from EOSS 2024, including those in the
  Embedded Linux Conference and the Zephyr Developer Summit
author: tsarlandie
tags: [conferences]
---

April marked the return of the Embedded Open-Source Summit, this year in
Seattle. I was lucky enough to be able to attend and split my time between the
Memfault booth in the exposition hall and many of the captivating presentations.
Since
[the videos have just been published](https://www.youtube.com/playlist?list=PLbzoR-pLrL6q2lcVcQ8s1uVNxde0aVdp3)
on the the Linux Foundation’s YouTube account, we thought it would be a good
time to highlight some of the talks and give you a quick summary which will,
hopefully, inspire you to go watch them!

<!-- excerpt start -->

We cover the talks I was able to see in person, as well as some talks seen by
my colleagues since they were posted. Obviously this is just our little biased
selection, we have not been able to see everything, let us know in the comments
what we missed!

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Comparing Linux OS Image Update Models

- [YouTube link](https://www.youtube.com/watch?v=enejawJmb5k&list=PLbzoR-pLrL6q2lcVcQ8s1uVNxde0aVdp3&index=11&pp=iAQB)
- Recommended by: Thomas

Drew Moseley, from Toradex and ex-Mender engineer, did some great research to
compare different strategies to update Linux systems. He starts with a little
explanation of what delta updates can look like for an A/B system: he covers
SWUpdate with ZChunks, Rauc, and Mender proprietary delta-update. Of course, he
also includes Toradex and starts with a little refresher of how it works
differently using libostree (a git like approach to your entire filesystem).

Then he devised a few “update scenarios”, including one very small change in a
configuration file, installing a big package, removing a big package. The choice
of scenarios seemed very relevant to me and I was glad to see someone take the
time to “run the numbers” and compare the number of bytes downloaded for each
scenario and each solution.

As always, the perfect solution will depend on your use-case. This talk gives a
solid methodology and some solid starting numbers to compare different
approaches. Thank you Drew for taking the time to do this and share it with the
community!

## Maximizing SDCard Life, Performance and Monitoring with KrillKounter

- [YouTube link](https://www.youtube.com/watch?v=mxed984GAIw&list=PLbzoR-pLrL6q2lcVcQ8s1uVNxde0aVdp3&index=51&pp=iAQB)
- Recommended by: Thomas

_(EDIT: An initial version of this post incorrectly attributed the
"silent-read-only behavior" to Samsung - Andrew clarified that this was observed on a SanDisk card. We regret the error.)_

Andrew Murray of the good penguin shared of their research on SDCards. Like
Andy, we often run into customers who use SDCard as their primary flash storage
and have lots of questions about the reliability of these cards. I will now
systematically point them to this great talk!

Andy started with a very thorough explanation of how flash storage works. I
thought I knew the topic well but I still learnt quite a bit, especially what
multi-layer flash storage really is and how it works (the 0 and 1 on the flash
are really analog values - instead of reducing one analog value to a 0 or a 1,
the multi-layer flash will infer 2 or 4 bits from one analog value - this is why
these types of flash memory are more “fragile” - fascinating stuff). He also
covered in great details the role of the flash controller and its different
roles.

He then presented a test setup that his team and him devised to take some
real-world measurement of flash storage and see how it would fail. He discusses
how different flash brands can fail in different ways (some <s>Samsung</s>
SanDisk apparently goes read-only - without telling the OS that it’s not
writing, some other flash just start timing-out on all read and write
operations).

KrillKounter is their attempt to build a tool from all this research. It will
keep track of how many writes to a flash (not easy because it needs to be
tracked across reboots and for each SDCard that might get pulled/inserted in the
device).

There was a lot more in this talk and I can’t recommend it enough!

## Engineering Secure SSH Access for Engineers

- [YouTube link](https://www.youtube.com/watch?v=M8RTsz0I5bM&list=PLbzoR-pLrL6q2lcVcQ8s1uVNxde0aVdp3&index=60&pp=iAQB)
- Recommended by: Thomas

Colin (Garmin) goes through the challenges for enabling SSH access to devices in
production. Passwords and secret keys are not great solutions: if you share one
across all your devices then a leak would be catastrophic. And a leak is hard to
protect against because the engineers need the keys with them when doing
maintenance work. So instead, he recommends using certificates to create keys
that are only valid for one hour or one day. The device SSH daemon validates
that the engineer key has been signed by the proper certificate, and that we are
still within the correct validity period. This way, the only real secret (the
private key used to sign the keys) can be stored in a very safe storage,
engineers are only granted access for a few hours (so an ex-employee will not
have access anymore).

The talk goes in much more details on how to actually implement this.
Demonstrates the commands to create certificates, options available in SSH to
restrict permissions of the key, mentions a few tools that can be used to manage
the certificate and sign the keys for the engineers, etc. He also discusses some
challenges that are specific to embedded devices such as how to deal with devices
that may not have the correct time set (he also reminds us that NTP is not a
safe protocol and time attacks are possible).

A great talk from someone who has obviously gone pretty far into the
implications of keeping SSH open on production device. Another one that I will
be recommending for years to come!

## From C to Rust: Bringing Rust abstractions to the Embedded Linux

- [YouTube link](https://www.youtube.com/watch?v=hmQr4fq6sH0&list=PLbzoR-pLrL6q2lcVcQ8s1uVNxde0aVdp3&index=58&pp=iAQB)
- Recommended by: Thomas

Fabien Parent, of Linaro, is a well known name of the community. I was curious
to hear him talk about Rust. The talk did not disappoint.

Fabien starts with a good overview of what is available for Rust in the Kernel.
He explains that Rust is limited to the device drivers because “core” components
of the Kernel needs to remain compatible with all architectures and some of
these architectures are not supported by the LLVM backend used by rustc (LLVM is shiny and new but [the kernel built with gcc still supports more architectures](https://chat.openai.com/share/b275dd72-bc8b-4faf-8a10-6fd368210896)).

A big challenge for Rust kernel developers today is that although tooling was
merged, there is very little in terms of abstraction. So to develop a new device
driver in Rust, you first need to write some abstractions. Many people have
started working on this but this work is often left unmerged because code cannot
be merged in the kernel unless it’s actually used somewhere. Any serious device
driver will require many abstractions, making for some big patch requests that
are complicated, hard to review (especially by non-Rust developers) and hard to
merge.

At this point, you probably share a bit of my disappointment. The best lesson of
the talk was Fabian’s optimistic approach to all of this. Instead of giving up,
he put some thoughts into the best way to move everything forward. His strategy
is to find some small drivers (that do not exist today because just replacing C
code with Rust code is also not acceptable for merge) that require only a few
abstractions and to use them to progressively introduce Rust abstractions in the
Kernel.

I loved hearing this positive take on all of this. A good reminder that you can
sit and complain about the state of things or take it as a challenge ;)

## Yocto Project: 5 Year Plan, Now What?

- [YouTube link](https://www.youtube.com/watch?v=mSP5MNt-58U&list=PLbzoR-pLrL6q2lcVcQ8s1uVNxde0aVdp3&index=26)
- Recommended by: Thomas

Megan Knight (AWS) and Andrew Wafa (ARM) went through the 5 year plan for Yocto,
highlighting all the results and sharing more on what is coming next.

Yocto is such a big project, and we are often working with pretty old versions.
It was refreshing to hear about the “modern” side of Yocto and got me interested
in testing some of the new stuff (maybe even give the graphical builder another
try 😛). I heard that a big part of this 5 year plan effort was to catch up on
technical debt and that now that it’s done, they are hoping to be able to
attract more volunteers to stay on top of the bug flow and also work on some
exciting new features.

It was also clear from the talk that although there is a lot of interest in
Yocto, there is not as much money flowing into this project. If your company can
contribute money or resources to Yocto, now might be a great time to do it!

## Delta Updates: Making Updates Leaner

- [YouTube link](https://www.youtube.com/watch?v=1rzf7tE-cKY&list=PLbzoR-pLrL6q2lcVcQ8s1uVNxde0aVdp3&index=5&pp=iAQB)
- Recommended by: Thomas

I only caught glimpses of this talk but I understand they have been doing some
research on how to strategically prepare the filesystem images so that delta
updaters would be able to compress the delta more efficiently. This is a
fascinating topic as we know how important compressing updates can be for some
of our customers, especially these relying on expensive cellular modem radios!

## Unwrap()-ing Rust on Embedded Linux

- [YouTube link](https://www.youtube.com/watch?v=XcUmGuGEPYU&list=PLbzoR-pLrL6q2lcVcQ8s1uVNxde0aVdp3&index=29&t=509s&pp=iAQB)
- Recommended by: Thomas

Obviously a bit biased here as this was my own presentation. This talk was very
much from the angle of “everything I would have liked to know when I started
writing Rust for embedded Linux”. We covered different ways to cross-compile and
package Rust apps, what to expect in terms of binary size, some strategies to
reduce the size of the binary and a few other lessons-learnt.

## Status of Embedded Linux

- [YouTube link](https://www.youtube.com/watch?v=BjK2r6TBxvQ&list=PLbzoR-pLrL6q2lcVcQ8s1uVNxde0aVdp3&index=36)
- Recommended by: Thomas

Marta (Syslinbit) and Tim (Sony) discussed everything they saw of interest in
the embedded linux world. The scope was wide, covering GCC improvement all the
way to discussing the status of the [elinux.org](http://elinux.org) website.

I won’t try to summarize this as this was already a very dense summary of
embedded linux news. I do encourage you to take a listen to get a better sense
of what is happening in our world.

## Rusty swapping: Rewriting a zswap backend in Rust

- [YouTube link](https://www.youtube.com/watch?v=WoXviWYsaOM&list=PLbzoR-pLrL6q2lcVcQ8s1uVNxde0aVdp3&index=42&pp=iAQB)
- Recommended by: Thomas

Vitaly (Konsulko) shared his experience writing a kernel driver in Rust. After
listening to Fabian talk the day before, I was curious to hear another take on
Rust in the kernel. Vitaly shared his experience writing a new zswap backend in
the kernel. For fun, he wrote it both in C and Rust and compared the results.

His conclusions are similar to Fabian’s: Rust in the kernel is hardly ready for
prime time today. The Rust learning curve is a challenge, getting the tools set
up correctly seems like a major pain, collaborating with existing kernel C API
requires a lot of unsafe code which reduces the “memory safety” advantage of
Rust. And once again, figuring out the way to get Rust code merged seems to be
the biggest issue. One encouraging point was a slight improvement in performance
and size with the Rust version.

## Patterns and Anti-Patterns in Embedded Development

- [YouTube link](https://youtu.be/M3lWe54jxiA?si=ATQc7wt3HQy4eE6v)
- Recommended by: Pat

Marta Rybczynska walks through security incidents from 2023 concerning devices
running Embedded Linux, protocols used by devices, and the Linux kernel itself
while going over what patterns and anti-patterns we can take away from the
incidents. In particular, I found the segment about the train software
fascinating. That the security researchers found that almost every train had a
different software version really shows the importance of having a defined OTA
and versioning process, not just for software management but also security. As a
big fan of the Yocto project, I also great enjoyed the fun fact about how the
now-infamous XZ backdoor wouldn’t build on the Yocto Release Candidate that
included it!

## Charging a Battery with Zephyr

- [YouTube link](https://www.youtube.com/watch?v=OAhP4c19FiQ&list=PLbzoR-pLrL6r63cNz4YY6OR7J-l7BShkY&index=3)
- Recommended by: Gilly

The maintainer of the charger drivers API, Ricardo Rivera-Matos from Ciirus
Logic, explains the role that a Zephyr host can play in controlling battery
charging. Notably, in enables setting the charge profile, setting protections
and limits for battery charging which could include limiting the maximum charge
percentage to improve battery life, and getting status info on charging. The
charger driver API was inspired by the Linux Power Supply Class, so for those
familiar with power management in Linux, the charger API should be familiar. We
also got a sneak peak at some of the improvements they want to make to the
charging API, including adding support for charger telemetry to observe a
charging session with ADCs, for example. This was a great talk that got me
wanting to try out this API!

## Enabling Real-Time Secure Connectivity to the Industrial Edge with Single-Pair Ethernet and Zephyr

- [YouTube link](https://www.youtube.com/watch?v=krhiEAjwcvs&list=PLbzoR-pLrL6r63cNz4YY6OR7J-l7BShkY&index=21)
- Recommended by: Gilly

Jason Murphy from Analog Devices discussed the usage of Zephyr on the industrial
edge using support for 10BASE T1L Single-Pair Ethernet. He explained how Zephyr
is great for industrial edge applications because of its native TCP/IP
networking stack supporting Ethernet protocols and low-latency networking, among
the well-known benefits like portability and community support. He also walks
through the board configuration (devicetree and Kconfigs) to use the ethernet
drivers and board support for the ADI eval boards for 10BASE T1L. I liked
learning about this option for wired applications using Zephyr since most of the
conversation in IoT is around wireless protocols!

## Conclusion

Does this feel like a lot of great content? We are pumped just writing about it!
And we have only scratched the surface: there are
[60 videos for the Embedded Linux Conference](https://youtube.com/playlist?list=PLbzoR-pLrL6q2lcVcQ8s1uVNxde0aVdp3&si=hPlloivCd5zZNgis),
[53 from the Zephyr Developer Summit](https://www.youtube.com/playlist?list=PLbzoR-pLrL6r63cNz4YY6OR7J-l7BShkY),
and many more from the Open-Source Summit. Let us know what we missed in the
comments below!

You can count on us to be there against next year!

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->
