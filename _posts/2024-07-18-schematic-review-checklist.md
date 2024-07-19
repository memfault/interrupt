---
title: A Schematic Review Checklist for Firmware Engineers
description:
  A checklist with supporting stories for firmware engineers to review when
  doing a schematic review.
author: mschulte
---

<!-- excerpt start -->

Schematic reviews are a part of the hardware development cycle in many if not
most, hardware development companies. Typically led by the electrical
engineering team, it is easy to overlook design issues that will be important to
the firmware team. This post tells of a few stories of design misses that I have
made and puts some common lessons learned into a checklist for other firmware
engineers. It is not meant to be an exhaustive list, but rather a starting point
for a firmware engineer to build their own checklist with the goal of helping
teams catch software/hardware interaction bugs earlier in the design cycle when
they're cheaper to fix.

<!-- excerpt end -->

This post is written in a voice that makes two general assumptions:

1. The reader is a firmware engineer.
2. The reader is not doing the schematic layout

While those two assumptions helped me write this post, I think the
learnings/stories in it are still applicable for a variety of setups. e.g.: If
the reader is an electrical engineer that wants to understand some firmware
concerns. Or if the reader is responsible for both the electrical and firmware
design of a project. With either of these assumptions broken, feel free to put
on your firmware engineer hat and read! Hopefully, this will be helpful for you.

{% include newsletter.html %}

{% include toc.html %}

## Schematic Review - What's Your Role as a Firmware Engineer

Most of the schematic reviews I've attended are led by electrical engineers and
are focused on the electrical components of the system. There are usually some
questions about whether the proper diode was used or the capacitors were
selected properly. Things that an electrical engineer should care about but
typically fly over my head. As a firmware engineer, I've found it useful to
think about what hardware features the code I'll end up writing will need.
Starting from that perspective, I can leverage the schematic review as an
opportunity to catch design issues while the fix is relatively cheap, before
hardware has gone through various rounds of testing, which can be the difference
between hitting a schedule or not.

"Schematic review" here is mostly used as an easy blog post title, and a clear
time for the firmware and electrical teams to interact. However, the important
piece of this post is that you as a firmware engineer work with your electrical
engineering counterpart to ensure you have the hardware you need to develop,
debug, and deploy your code. And you identify those needs early on in the
development cycle with the electrical team when changes to PCBs are relatively
cheap.

Some pre-schematic review checklist items:

- [x] Get invited to the schematic review. Really, this is a callout to be
      working with your electrical engineer. If you've been working with your
      electrical engineer, providing guidance on what hardware you'll need,
      being invited to the schematic review should be a given.
- [x] Build your checklist upfront. Use this blog post as a starting point, but
      make sure you customize the list for the given project.

## Story 1 - Brown-out Loop {#story-1-brown-out-loop}

Years ago, I worked on an LTE enabled e-bike. The LTE module was powered off the
battery that powered the electric assist for the bike, but had a single cell
backup battery that would be used if the main battery was removed from the
device.

Once these modules were in the field, we started getting reports of them being
bricked. Upon inspection, we found that many of these devices had external flash
corruption, which was creating a bricked device due to a corrupt data structure
in external flash which caused the filesystem to crash when booting.

Looking at some of the logs on these devices, we could see that before the flash
corruption occurred, the device was getting caught in a brown-out loop.
Reproducing this in the lab, we found that at low backup battery, during an LTE
transmission, the voltage of the battery would sag, the battery controller would
hit a low voltage cutoff, and the device would shut off. Once the device
shut-off, the LTE transmission would immediately stop, the voltage of the
battery would recover, and the battery controller would re-enable the battery.
Once the battery was re-enabled, the device would turn back on, where it would
attempt to write to flash on boot and simultaneously attempt an LTE
transmission, which would cause a brown-out, and we'd loop again. It would do
this thousands of times before the battery voltage was low enough that it would
stop recovering.

Because of this brown-out loop, we were stress testing our external flash power
cycle resilience, and needless to say, we found bugs. Unfortunately for us, our
hardware did not have a method of shutting itself off. This meant that, even if
we know the backup battery had insufficient power to power our device, we could
not shut-off the entire system.

As a workaround, we ended up putting a voltage detection circuit + loop at the
very start of our firmware. If the device was attempting to boot with a measured
voltage of < CUTOFF VOLTAGE and the device did not detect a second power source
(measured by a GPIO being pulled high), the device would sit in a
`while (!external_power_okay()) {pet_watchdog();}` style loop, keeping at least
external flash ok. We also upgraded our external flash filesystem to a version
that was much more resilient to boot loops.

After this story, I always look to see if a battery powered device can both: (1)
effectively monitor battery state of charge, and (2) shutdown in a low state of
charge condition to prevent these sort of brown-out loops.

## Story 2 - Reset By Power Drain {#story-2-reset}

On that same project years back, we occasionally noticed the device would become
unresponsive. While we had a watchdog, for some reason the watchdog was either
not triggering, or not resetting enough state on the device. Once we were able
to connect our debug tools (namely, a CAN dongle), we could send a reset command
the device, it would recover and work perfectly. However, we had missed
designing in from the beginning a mechanism to reset the device that was
accessible to our users.

Because we had a battery in the device, our instructions to our users were to
"set the device aside and wait for the battery to lose power." Unfortunately for
us, the bike stuck in this state was also in our lowest power mode and had no UX
difference between a bike with a dead battery. We instructed our users to let
bikes set for 4 days before they could recover. That's a pretty poor user
experience.

While we solved this in future releases by adding an NFC based reset, this is an
issue that can only be fixed going forward, and not in reverse. Adding a
physical way to reset the device for a human user from the beginning is a very
powerful solution. How many things are fixed by "turning it off and turning it
back on again"?

## Story 3 - I2C Resets {#story-3-i2c-resets}

While developing a fitness tracker, I kept running into a bug where I
occasionally needed to fully power cycle a device to get the temperature sensor
to start working again. The temperature sensor was connected over I2C, and
digging into the issue, I found that, when this was happening, the I2C SDA line
was stuck low. We were able to reproduce that failure by having the MCU reset in
the middle of an I2C read. Because the I2C controller on the MCU was controlling
the clock and stopped clocking, the peripheral was still driving the SDA line,
waiting for the next clock signal.

We had no reset line headed to the I2C device, so we could not reset the I2C
device. However, we could manually clock out the remaining SCL cycles on the SCL
line, and so modified our boot sequence to manually toggle the SCL line to
"clock out" the remaining data, freeing up the I2C bus.

However, I'd still have preferred a true reset line.

## Story 4: GPIO Maps {#story-4-gpio-maps}

This is less of a war story and more of a shoutout to good electrical engineers.
Before walking into the schematic review for one project I was working on, the
EE on the project published a spreadsheet of all the GPIOs for the MCU. This
included their main function after reset, intended function, and any additional
notes. It made it very easy as a firmware engineer to scan the list of GPIOs and
ensure they were all configured properly. I'd recommend this is something that
all firmware engineers ask for when looking at a schematic.

## Story 5: Too much or too little debug {#story-5-too-much-or-to-little-debug}

I worked on a project once with an electrical engineer that proposed we make it
hard to attach a debugger to the board. He proposed that we force someone to
solder wires/leads to debug pads in order to get a debugger to work. As a young
firmware engineer, I was floored. The debugger was my lifeline, and especially
on a Cortex-M4 could help troubleshoot so many things. Making it difficult for a
firmware engineer to use their most powerful tool seemed ludicrous to me.

However, the electrical engineers point was that firmware engineers rely too
much on debuggers to debug devices, and don't spend enough time building
tools/systems to track down crashes/bugs in the field, where a debugger is
likely not attached. This was a good point. While working on fitness trackers
years ago, I'd seen a firmware update over UART protocol continuously fail
because none of the firmware developers used it (because it was broken of
course). This meant it was only ever used by our QA team, who would file bugs,
but it was never prioritized until one day it became a necessary firmware update
path, and required a lot of scripting on the host side to work around firmware
bugs.

So I think that debuggers are essential tools for firmware engineers, and we
should not hamper their usage. However, it's important to commit to building
tools and systems for debugging systems in the field that use only the tools
that will be available in the field, as your end goal as a firmware developer is
to be able to fix those issues, not just the ones you can see at your desk.

## My current checklist

Given these experiences, I have compiled the following list:

### Power/Battery

- [x] How does the firmware shutdown the device? Especially for a battery
      powered device.[1](#story-1-brown-out-loop)
- [x] For a battery powered device, Can the firmware measure how much power is
      available? How much accuracy is needed in this measurement?
      [1](#story-1-brown-out-loop)
- [x] How does a user reset your device?[2](#story-2-reset)

### GPIOs

- [x] Provide a GPIO map in a spreadsheet, with main function after reset,
      intended function, and any additional notes.
- [x] Are all MCU peripherals used only once?
- [x] If the MCU is held in reset, are the peripheral devices held in their
      proper state? (Pull-up/pull-downs where needed)
- [x] Does the reset line have a proper pull-up or pull-down? (This is a common
      failure, so worth noting).
- [x] Are the debug GPIOs used _only_ for debug purposes?

### Busses

- [x] Do you have a mechanism to reset all IC's attached to buses (when
      appropriate)?
- [x] Are all I2C devices on the same bus on different addresses?
- [x] If your MCU resets in the middle of an I2C read, how do you recover
      talking with the device you were reading from?[3](#story-3-i2c-resets)
- [x] If your MCU resets in the middle of a SPI transfer, how does the device
      the MCU was talking with handle that?
- [x] For all UART devices, are Rx/Tx lines setup properly (ensure Tx->Rx, and
      check the datasheets of the device to see if Rx means receiving on the
      peripheral side)?
- [x] For all UART devices, are RTS/CTS lines needed?
- [x] For all UART devices, are RTS/CTS lines setup properly (this is confusing,
      because RTS -> CTS, but some devices label them differently)?

### Memory/Flash Storage

- [x] Do you have a code size estimate? And RAM estimate? How much buffer do you
      have in case your estimate is low?
- [x] Do you have sufficient space for additional features over the lifetime of
      the product?
- [x] Where will crash logs be stored? (Typically need to survive a system
      reset). Think about this when thinking about RAM requirements.

### Debug/Test Infrastructure

- [x] How will you (a firmware engineer) attach a debugger to the board? (By
      soldering is not usually a good
      answer).[5](#story-5-too-much-or-to-little-debug)
- [x] Are non-essential, but helpful, GPIOs exposed for a debugger, such as SWO
      or ETM?
- [x] Do test points exist for power rails, analog signals, and buses?

## Conclusion

This checklist captures the most important items I have learned to ask about as
a firmware developer in schematic reviews. Feel free to take it/modify, and add
to it. Or, even better, comment on this article with your own horror stories and
lessons learned! I've found it useful to keep stories with rules to help me
remember why the rules exist in the first place. All rules are meant to be
broken, and sometimes I end up waiving items from the checklist, or adding new
items for specific projects. These rules have helped me catch a few
hardware/software interactions before boards were produced, which helped us keep
to our schedule and get devices shipped on time!

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}
