---
title: "Beyond Embedded in the 2020s"
author: rameen
excerpt: "For embedded systems the decade turnover couldn't be better
timed.  As we embark upon the next decade the capabilities of software for
embedded systems will catch up to those of hardware, and at some point
an emergent discipline - something beyond embedded - will take shape."
---

> Editor's note: We're starting off February with a think piece from Rameen
> Aryanpur.  This strikes a different tone from our standard fare (usually
> technical how to's and tutorials), but we hope you will enjoy it just as well.
> We are always looking for new submissions, so don't hesitate to get in touch
> if you would like to contribute!

With the turn of the decade, there are many next decade prediction pieces floating
around. For embedded systems the decade turnover couldn't be better timed.  The
capabilities of embedded systems and the tools we use to deploy, manage, and
debug them have come a long way in the last ten years. The vast majority of
this progress, however, has come in hardware rather than software. So, color me
a prospective prophet: in the next decade the capabilities of software for
embedded systems will catch up to those of hardware, "firmware" will become
increasingly difficult to distinguish from "software", and a new discipline
altogether will begin to take shape.

The rapid increase in capability of embedded hardware in the 2010s was primarily
attributable to ARM and chip vendors’ investment into and around the Cortex-M
and Cortex-A cores (sorry Cortex-R, no shout out here). Today it's a stretch to
refer to mid to high-end microcontrollers as microcontrollers. With
expandability to megabytes of flash and RAM and hundreds of megahertz of
processing speed, as well as L1 and L2 caches, pipelined architectures, and
ever-expanding (and now customizable) ISAs, mid to high-end "microcontrollers"
at the dawn of the 2020s look a lot more like Pentium II processors of the 1990s
than PIC32 microcontrollers of the 2000s/2010s. It's this difference that
characterizes the divergence in embedded systems hardware and software today and
that hints at how the two will again converge. In the 2010s we largely wrote
software assuming it was going to run on something like a PIC32 when it was
actually going to run on something like a Pentium II. The advent of RISC-V, an
open-source entrant to the microcontroller space, will only accelerate the
trend. 

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

But what types of applications, products, and services will be made possible
and which will be successful in the era of beyond embedded? Unfortunately,
Marty didn't bring his almanac back from the future, so how beyond embedded as
an opportunity manifests in the form of specific outcomes is anyone's guess.
The nature of the opportunity on a technical level, however, can be reduced to
the ability to reason about the properties of embedded systems in parlance
fully decoupled from the details of the underlying hardware. This in turn will
allow for the application of existing abstractions and mental models from other
domains like cloud and mobile. Massively scalable asymmetric distributed
computing and cross-device event driving are two outcomes that could result
from this cross pollination.


Various areas of computing in decades past have undergone similar
transformations with profound implications. Broadly-available, affordable
server multi-tenancy in the 2000s was catalyzed in part by hardware
advancements in virtualization and the development of software to manage and
leverage those advancements at scale. This combination of hardware and software
enabled the application of existing and new mental models and other modes of
reasoning about massively scalable, multi-tenant distributed systems, which in
turn contributed to the rise of Cloud computing in the 2010s. Beyond embedded
will follow its own path, driven at its source by the momentum of traditional
embedded and shaped along the way by the surrounding landscape of technology
development and broader market forces.

The manifestation of technology in the world of beyond embedded will be one of
the most exciting disruptions to unfold this decade. The irony of course is
that like most truly disruptive technology it will largely fly under the radar,
but second-order effects like business successes will be numerous and broadly
publicized. Take a look beneath the surface of a press release or white paper,
however, and you'll see an emergent discipline coming to life, one that will
fundamentally redefine the technical conversation around embedded systems and
in the process disrupt industries ripe for disruption.