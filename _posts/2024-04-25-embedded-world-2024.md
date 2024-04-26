---
title: Standout Exhibits at Embedded World 2024
description:
  A recap of the noteworthy exhibits from Embedded World 2024, including
  Hightec, Bluewind, RISC-V, TinyML, Zephyr, Emproof, and Embedd
author: francois
tags: [embedded-world, conferences]
---

Earlier this month, I had the pleasure of traveling to Nuremberg, Germany to
attend Embedded World. If you have not heard about it before, Embedded
World[^ew] is the largest trade show in the embedded systems industry. This
year, over 35,000 people attended and 1,100 businesses exhibited at the
Nuremberg Messe.

Embedded World is an amazing opportunity to learn about the latest developments
in the industry, and to meet the people behind the technology. Semiconductor
manufacturers, software vendors, engineering firms, module makers and component
suppliers all attend, show off their products, and give talks to present their
latest thinking and innovation. It doesn’t hurt that Nuremberg, where it is
held, is a beautiful city with good food, good beer, and great public transit.

I spent a lot of time at the Memfault booth of course. It was thrilling to meet
so many engineers who wanted to learn more about observability. We welcomed
hundreds of people at our booth, with 4 demo stations in active use throughout
the 3 days of the show. The Memfault socks we brought as booth swag were a big
hit.

<!-- excerpt start -->

In this post, I will share the technologies I saw at Embedded World 2024 that I
was most impressed by. I was lucky to have plenty of time to walk from booth to
booth, and had some fantastic conversations that I hope you will find
interesting.

<!-- excerpt end -->

![]({% img_url embedded-world-2024/memfault-booth.jpg %})

{% include newsletter.html %}

{% include toc.html %}

## Rust, front & center

![]({% img_url embedded-world-2024/hightec.jpg %})

This year, Rust seems to have broken through in the embedded community. Nowhere
was it more evident than at the Hightec[^hightec] booth. For those of you who
are not familiar with Hightec, they sell qualified builds of open source
compilers (e.g. GCC) for safety critical industries such as automotive. This
year, they had a HUGE crab-inspired mascot in the background of their booth to
announce the availability of their ASIL D qualified Rust compiler. I’m excited
to see serious ecosystem players like Hightec make such a huge bet on Rust, as I
believe it is a better language and runtime for the embedded world.

This was not the only Rust I saw at the show. For example, I had a great chat
with engineers at Bluewind[^bluewind-rust] about their use of Rust. We also had many
booth visitors ask us whether or not Memfault supports Rust - we do and were
glad to share our experience building some of our SDKs in Rust at Memfault.
We've known for a while that Rust is growing in use in embedded - several of our
customers use the language in their products - but it was encouraging to hear so
much about it at a serious industry event.

## RISC-V continues to gain popularity

![]({% img_url embedded-world-2024/risc-v.jpg %})

RISC-V had a huge booth hosting multiple ecosystem players. SiFive was there of
course, but I found it notable to see industrial heavyweights such as Siemens
exhibiting at the RISC-V booth. There is a lot of energy in the RISC-V
ecosystem, and we’re starting to see some of our customers use RISC-V chips in
their products. While I continue to be a big fan of the ARM architecture, it’s
great to see alternatives develop, especially open source ones.

I've especially been following the e-trace standard (naturally) which offers a
number of trace facilities for RISC-V systems. Siemens gave a great presentation
at the show about their use of the standard.

## Everybody wants to do AI

![]({% img_url embedded-world-2024/tinyml.jpg %})

Perhaps the most popular topic of conversation at Embedded World this year was,
unsurprisingly, AI at the Edge. The TinyML foundation (pictured above) is
leading the charge, bringing together exciting startups such as Edge Impulse and
Imagimob that make it easy to deploy models to your devices. The foundation
released the 2024 edition of their "AI at the Edge" report[^ai-at-the-edge]
which they were distributing at the show. It's a great read!

Everyone else was talking about AI too! Semiconductors are releasing new
development kits geared towards AI such as the Infineon PSoC 6 AI EK[^psoc6-ai],
software vendors are integrating AI features, and research labs are showing off
cool new use cases for AI such as power efficient Atrial Fibrillation detectors
for healthcare use cases.

## Zephyr is winning hearts & minds

![]({% img_url embedded-world-2024/zephyr.jpg %})

Every year, the Zephyr booth is bigger, showing off more sponsors, and busier.
2024 was no exception. I’m thrilled to see the energy behind this open source
project with an ecosystem of chips, tools, and libraries coming together around
it.

Zephyr may be the most active open source project in the embedded space with
over a thousand commits every month from hundreds of authors. As a member of the
Zephyr project, Memfault got to sit at the Zephyr booth each day to chat with
attendees. We spoke with various embedded developers and semiconductor companies
excited about transitioning to Zephyr, or in the case of semiconductor
companies, adding support for Zephyr in their SDKs.

## Exciting new startups are sprouting

![]({% img_url embedded-world-2024/exein.jpg %})

Exein[^exein] is a new startup in the embedded security space. They have built
an agent for embedded Linux devices which collects security signal from the
device as it is used and uploads it to their cloud for analysis and alerting.
And yes, their agent is written in Rust! I think we’ve done too little for too
long to secure IoT devices, and am glad to see startups like Exein take up the
challenge of IoT security.

![]({% img_url embedded-world-2024/emproof.jpg %})

Another security startup, Emproof[^emproof] provides firmware hardening services
to device makers. Their clever solution scans and rewrites firmware binaries to
obfuscate it (preventing reverse engineering) and add security features such as
stack guards.

I've always been suspicious of security-by-obscurity, the idea that obfuscation
will make you safer. But I think the approach of automatically adding security
features such as stack guards, address randomization, and others to firmware has
a lot of potential.

![]({% img_url embedded-world-2024/incari.jpg %})

I was surprised by the number of Embedded GUI solutions (they call themselves
“Human Machine Interface” - HMI platforms) that exist. Qt of course had a large
booth, but so did Embedded Wizards and a long list of others.

One startup in the space stood out to me: Incari[^incari]. They have built a GUI
editor for hardware (Incari Studio) and an embedded rendering engine which can
render those GUIs on limited devices. Incari Studio even has a Figma integration
so you can drop a Figma design into its studio, then compile your design
directly for your device. That’s a pretty cool workflow! Unlike Qt it won’t work
on your MCU, however, as it requires 256MB and a higher level OS (QNX, Linux,
…).

![]({% img_url embedded-world-2024/embedd.jpg %})

Last but not least, I had a chance to chat with Michael at Embedd[^embedd]. They
have built an AI system that can parse a data sheet provided as PDF and spit out
a fully functional driver in C. This feels like a task perfectly suited for AI
techniques, and I look forward to trying it out when it goes into general
availability. I would be thrilled if I never had to write a driver again!

## Conclusion

This was the most exciting Embedded World yet. I got to meet many of our
partners and customers, learn about up and coming startups disrupting the
industry, and get a pulse on border hardware and software trends. Companies
building devices should consider sending an engineer or two.

I look forward to seeing you all there next year. In the meantime, I will be at
the brand new
[Embedded World North America conference](https://www.embedded-world.de/en/embedded-world-wide/embedded-world-north-america)
in Austin this October!

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^ew]: <https://www.embedded-world.de/en>
[^bluewind-rust]: <https://www.bluewind.it/case-study/rust/>
[^hightec]: <https://hightec-rt.com/en/>
[^ai-at-the-edge]: <https://www.wevolver.com/article/2024-state-of-edge-ai-report>
[^psoc6-ai]: <https://www.infineon.com/cms/en/product/evaluation-boards/cy8ckit-062s2-ai/>
[^exein]: <https://www.exein.io/>
[^emproof]: <https://www.emproof.com/>
[^incari]: <https://www.incari.com/>
[^embedd]: <https://embedd.it/>

<!-- prettier-ignore-end -->
