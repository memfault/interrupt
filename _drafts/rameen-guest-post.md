---
title: "Beyond Embedded in the 2020s"
author: rameen
excerpt: "For embedded systems the decade turnover couldn't be better
timed.  As we embark upon the next decade the capabilities of software for
embedded systems will catch up to those of hardware, and at some point
an emergent discipline - something beyond embedded - will take shape."
---

> Editor's note: We're kicking off 2020 with a think piece from Rameen Aryanpur.
> This strikes a different tone from our standard fare (usually technical how
> to's and tutorials), but we hope you will enjoy it just as well. We are always
> looking for new submissions, so don't hesitate to get in touch if you would
> like to contribute!

There are a lot of decade in review, next decade prediction pieces floating
around. Allow me to throw my hat in the ring. For embedded systems the decade
turnover couldn't be better timed.  The capabilities of embedded systems and the
tools we use to deploy, manage, and debug them have come a long way in the last
ten years. The vast majority of this progress, however, has come in hardware
rather than software. So, color me a prospective prophet: as we embark upon the
next decade the capabilities of software for embedded systems will catch up to
those of hardware and at some point an emergent discipline - something beyond
embedded - will take shape.

Serendipitous to the decade motif, I wrote my first line of software for an
embedded system in early 2010. I wrote my first line of functioning software for
an embedded system soon after - in late 2011. Kidding aside, what became
increasingly clear as the 2010s rumbled along was that the term "embedded" was
being used to describe a more diverse and capable set of technologies than
before. One implication of this was that learning to write software for embedded
systems in the 2010s was, to put it lightly, really hard. In retrospect, some of
that difficulty can be attributed to the inherent challenges in understanding
embedded mental models. But another outsize, generally unspoken reason for this
difficulty was that those traditional mental models were becoming less useful
more or less at the rate at which the capabilities of the hardware were
increasing. Absent new mental models (or the fledgling application of existing
ones from other domains) traditional embedded best practices quickly became no
longer best practices and learning to write software for embedded systems was,
well, really tough.

The rapid increase in capability of embedded hardware in the 2010s was primarily
attributable to ARM and chip vendors’ investment into and around the Cortex-M
and Cortex-A cores (sorry Cortex-R, no shout out here). RISC-V, a more recent
entrant to the hardware space, has also started to have an impact although not
yet to the same degree. The scope and pace of hardware advancements were most
pronounced in the Cortex-M line, where today it's a stretch to refer to mid to
high-end microcontrollers as microcontrollers. With expandability to megabytes
of flash and RAM and hundreds of megahertz, if not soon a gigahertz of
processing speed, as well as L1 and L2 caches, pipelined architectures, and
ever-expanding (and now customizable) ISAs, mid to high-end "microcontrollers"
at the dawn of the 2020s look a lot more like Pentium II processors of the 1990s
than PIC32 microcontrollers of the 2000s/2010s. It's this difference that
characterizes the divergence in embedded systems hardware and software today and
that hints at how the two will again converge. In the 2010s we largely wrote
software assuming it was going to run on something like a PIC32 when it was
actually going to run on something like a Pentium II.

Having Pentium II-like machines on the compute and memory axes with single and
double precision floating point coprocessors has unlocked large performance
gains for general purpose signal processing, hard real time, and safety critical
applications. But it has also opened greenfield opportunities for general
purpose applications ill-served by microprocessors (for example because of
hardware/software complexity) and traditional microcontrollers. Today, the
software to capture those greenfield opportunities is free to graduate from
RTOSes to more flexible and capable general purpose operating systems, and the
tools to support deploying, managing, and debugging that software are free to
graduate from UART logs, print statements, and JTAG debuggers to continuous
integration and deployment systems and Infrastructure as code platforms. Mature,
scalable, maintainable, and extensible software frameworks and DevOps tools
already exist for Cloud and Mobile applications. In the 2020s they (or
similar-looking relatives) will become the standard for many greenfield embedded
applications, at which point embedded software and hardware will have converged
and we'll enter the era of beyond embedded.

But what does beyond embedded look like? What types of applications, products,
and services will be made possible and which will be successful? Unfortunately,
Marty didn't bring his almanac back from the future, so how beyond embedded as
an opportunity manifests in the form of specific outcomes is anyone's guess. The
nature of the opportunity on a technical level, however, can be reduced to the
ability to reason about the properties of embedded systems in parlance fully
decoupled from the details of the underlying hardware. This will create the
opportunity for embedded systems software to grow outwards in the form of
scalable distributed systems rather than downwards in the form of
"software-running-on-hardware" as has been customary.

Similar transformations have happened in various areas of computing in decades
past with profound implications. Broadly-available, affordable server
multi-tenancy in the 2000s was catalyzed in part by hardware advancements in
virtualization and the development of software to manage and leverage those
advancements at scale. This combination of hardware and software enabled the
application of existing and new mental models and other modes of reasoning about
massively scalable, multi-tenant distributed systems, which in turn contributed
to the rise of Cloud computing in the 2010s.

What products and services are created and scale in the era of beyond embedded
will be one of the most exciting stories in tech to unfold this decade. But
because of its highly technical nature and the relatively small number of
practitioners in the space it will likely do so under the radar. The second
order effects, however – in particular applications and business successes -
will be numerous and broadly publicized. No matter how beyond embedded plays out
this decade I just have one ask - please, don’t confuse any of this with
blockchain!