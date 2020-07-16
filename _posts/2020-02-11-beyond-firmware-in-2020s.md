---
title: "Beyond Firmware in the 2020s"
author: rameen
excerpt: "In the next decade the capabilities of software for
  embedded systems will catch up to those of hardware, 'firmware' will become
  increasingly difficult to distinguish from 'software', and a new discipline
  altogether will begin to take shape."
---

> Editor's note: We're starting off February with a think piece from Rameen
> Aryanpur. This strikes a different tone from our standard fare (usually
> technical how to's and tutorials), but we hope you will enjoy it just as well.
> We are always looking for new submissions, so don't hesitate to get in touch
> if you would like to contribute!

With the turn of the decade, there are many next-decade prediction pieces floating
around. For embedded systems, the decade turnover could not be better timed. The
capabilities of embedded systems and the tools we use to deploy, manage, and
debug them have come a long way in the last ten years. The vast majority of
this progress, however, has come in hardware rather than software. So, color me
a prospective prophet. In the next decade: the capabilities of software for
embedded systems will catch up to those of hardware, "firmware" will become
increasingly difficult to distinguish from "software", and a new discipline
altogether will begin to take shape.

The rapid increase in capability of embedded hardware in the 2010s was primarily
due to ARM and chip vendors’ investment in the Cortex-M and Cortex-A families of
devices. Thanks to that investment, it is a stretch today to refer to mid to
high-end SoCs as microcontrollers. With expandability to megabytes of flash and
RAM and hundreds of megahertz of processing speed[^3], as well as L1 and L2
caches, pipelined architectures, and ever-expanding (and now customizable) ISAs,
mid to high-end "microcontrollers" at the dawn of the 2020s look a lot more like
Pentium II processors of the 1990s than PIC32 microcontrollers of the
2000s/2010s. The trend is clear, and shows no sign of slowing down: our MCUs are
catching up to CPUs. Consider the amount of RAM available in top-of-the-line
STM32 chips over the past few years:

![STM32 Available RAM over time]({% img_url embedded-in-2020/stm32-chart.png %})

In the 2010s we largely wrote software assuming it was going to run on something
like a PIC32 when it was actually going to run on something like a Pentium II.
The advent of RISC-V, an open-source entrant to the microcontroller space, will
only accelerate the trend.

Having Pentium II-like machines with single and double precision floating point
coprocessors has unlocked large performance gains for general purpose signal
processing, hard real time, and safety critical applications. But it has also
opened greenfield opportunities for general purpose applications ill-served by
microprocessors (for example because of hardware/software complexity) and
traditional microcontrollers. The early signs are here: from Google's release of
their popular TensorFlow framework for machine learning on MCUs[^1], to new
microcontrollers with extensive image processing & computer vision
capability[^2]. Larger companies know this, and they are competing to get a
foothold into the embedded space to stitch together the software ecosystem of
tomorrow. Amazon purchased FreeRTOS, Microsoft purchased ExpressLogic, while
Google built Little Kernel.

So, what can we expect? First, firmware development will increasingly look like
software development. Agile and iterative approaches will be applied to great
results. As the complexity increases, the need for tooling will become
increasingly clear. While vendor IDEs, print statements, and JTAG got us this
far, the 2020s will be the era of Continuous Integration, live monitoring, data
pipelines, experimentation frameworks, open source. Stated more simply: firmware
as a discipline will embrace devops techniques and tools already in use in the
Cloud and Mobile application worlds. This is what I like to call "Beyond
Firmware".

But what types of applications, products, and services will be made possible and
which will be successful in this new era? Unfortunately, Marty did not bring his
almanac back from the future, so we cannot know for sure. My guess is that
hardware businesses will not only do more software engineering, but look a lot
more like software businesses. They'll use the methods of software engineering,
the tools of software engineering, but also the business model of software
engineering. Bolt - a prominent hardware VC - called it in 2015[^4]. On a
technical level, embedded software will become increasingly decoupled from the
underlying hardware it runs on. Software abstractions and mental models will be
part of our day-to-day.

Various areas of computing in decades past have undergone similar
transformations with profound implications. Broadly-available, affordable
server multi-tenancy in the 2000s was catalyzed in part by hardware
advancements in virtualization and the development of software to manage and
leverage those advancements at scale. This combination of hardware and software
enabled the application of existing and new mental models and other modes of
reasoning about massively scalable, multi-tenant distributed systems, which in
turn contributed to the rise of Cloud computing in the 2010s. Beyond firmware
will follow its own path, driven at its source by the momentum of traditional
embedded and shaped along the way by the surrounding landscape of technology
development and broader market forces.

The manifestation of technology in the world of beyond firmware will be one of
the most exciting disruptions to unfold this decade. The irony of course is
that like most truly disruptive technology it will largely fly under the radar,
but second-order effects like business successes will be numerous and broadly
publicized. Take a look beneath the surface of a press release or white paper,
however, and you will see an emergent discipline coming to life, one that will
fundamentally redefine the technical conversation around embedded systems and
in the process disrupt industries ripe for disruption.

## References

[^1]: [TensorFlow Lite for Microcontrollers](https://www.tensorflow.org/lite/microcontrollers)
[^2]: [Introducing the K210 RISC-V AI Camera (M5StickV)](https://m5stack.com/blogs/news/introducing-the-k210-risc-v-ai-camera-m5stickv)
[^3]: [Teensy 4.0 Development Board](https://www.pjrc.com/store/teensy40.html)
[^4]: [The 3 Business Models That Matter](https://blog.bolt.io/business-models-that-matter/)
