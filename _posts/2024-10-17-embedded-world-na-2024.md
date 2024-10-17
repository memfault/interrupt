---
title: Embedded World North America 2024 Recap
description: A recap of Embedded World 2024
author: gminn
tags: [embedded-world, conferences]
---

Last week, a handful of Memfolks had the opportunity to travel to Austin to
attend the first ever Embedded World North America[^ew_na]! Embedded World NA
welcomed 3,500 visitors and 180 vendors across 3 days[^ew_na_pr]. While it was
surely a smaller showing than Nueremburg's Embedded World, we still wanted to
quickly touch on our takeaways from the event.

<!-- excerpt start -->

In this post, we will cover what we learned from the first Embedded World North
America. Our team had the chance to meet with some IoT device makers and
understand what is top of mind for them.

<!-- excerpt end -->

<p align="center">
  <img width="700" src="{% img_url embedded-world-na-2024/memfault-booth.jpg %}"/>
</p>

{% include newsletter.html %}

{% include toc.html %}

Three major themes stood out to us at this conference while perusing the booths
and talking to developers at our booth: Security, AI, and Zephyr.

## Security regulations heating up

Security regulations have become increasingly part of our conversations with IoT
companies, and this trend was also apparent at EW. Leaders know that their
products must comply with regulations like the CRA and PSTI[^psti]. They want to
stay ahead as best they can and are worried about getting started too late and
missing deadlines. In fact, last week, the CRA was adopted by the EU[^eu_cra],
marking an important next step in the official publication of this law.

I would be remiss not to mention that we at Memfault have been thinking a lot
about security. We have compiled some resources on what to focus on, including
[IoT Security Best Practices](https://memfault.com/blog/7-iot-security-practices/),
[Cyber Trust Mark Guide](https://memfault.com/blog/us-cyber-trust-mark/), and
the [CRA Guide](https://memfault.com/blog/eu-cyber-resilience-act-guide/). We
hope these references are helpful if your team is thinking about how to comply
with the new standards!

## More AI at the edge and LLM-based tooling

<p align="center">
  <img width="500" src="{% img_url embedded-world-na-2024/edge-impulse-cat.jpg %}"/>
</p>

Unsurprisingly, AI continues to be a hot topic, with nearly every semiconductor
company showing its own edge AI demos. The
[Edge Impulse](https://edgeimpulse.com/) booth also really caught our attention.
They had a demo showing a smart litter tray that could use lightweight ML models
to learn the habits and track the weight of the cat for better health
monitoring. See the cute stuffed cat picture above 😸!
[Driver AI](https://www.driver.ai/) was another highlight - a tool that
automatically creates technical documentation from source code using LLMs. They
launched their product publicly this month[^driver_ai_launch]. I, for one, would
love a tool to help understand complex parts of a software stack better!

## Zephyr continues to gain popularity

As we also noted
[in our EW recap earlier this year](https://interrupt.memfault.com/blog/embedded-world-2024#zephyr-is-winning-hearts--minds),
Zephyr continued to come up in conversations with developers across the
industry. We had a few people ask for advice on selecting the right RTOS,
particularly asking for comparisons between RTOSs like Zephyr and FreeRTOS. A
Zephyr community member wrote
[a blog post last year on their transition to Zephyr from FreeRTOS](https://zephyrproject.org/why-we-moved-from-freertos-to-zephyr-rtos/),
which you might find helpful if you are considering such a switch. Zephyr was at
the ADI booth this year with their signature kite flying to mark the spot 🪁
Zephyr recently hit 100,000 contributions, marking an exciting milestone in its
trajectory as one of the fastest-growing open source projects[^cncf_report].

## Conclusion

This conference was another great opportunity for us to meet a variety of
embedded teams at our booth and peek at the latest technology from vendors. No
time was wasted in the announcement[^ew_na_pr] -- the next Embedded World North
America will be in Anaheim next November! We can't wait to see what the second
year of this regional EW will bring. If we didn't see you this year, we hope
we'll cross paths in 2025.

_Shoutout to Memfolks who attended the conference this year and helped put
together this post: Chris, Eric, Jon, François, JP and Jesse._

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^ew_na]: <https://embedded-world-na.com/>
[^ew_na_pr]: <https://embedded-world-na.com/press-releases/first-embedded-world-north-america-ends-in-style/>
[^eu_cra]: <https://www.consilium.europa.eu/en/press/press-releases/2024/10/10/cyber-resilience-act-council-adopts-new-law-on-security-requirements-for-digital-products/>
[^driver_ai_launch]: <https://www.allaboutcircuits.com/news/driver-emerges-from-stealth-with-new-way-write-decode-tech-docs/>
[^cncf_report]: <https://www.cncf.io/blog/2024/07/11/as-we-reach-mid-year-2024-a-look-at-cncf-linux-foundation-and-top-30-open-source-project-velocity/>
[^psti]: <https://www.gov.uk/government/collections/the-product-security-and-telecommunications-infrastructure-psti-bill-factsheets>

<!-- prettier-ignore-end -->
