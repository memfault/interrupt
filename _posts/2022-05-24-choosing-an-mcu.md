---
title: Selecting Your Next Project's MCU
description:
  Choosing the best microcontroller is an important decision given the chip
  shortage and engineering effort involved.
author: lars
tags: [mcu]
---

<!-- excerpt start -->

Selecting the best chip can be tedious work but the best chip can save you a lot
of time and money, and might even be faster! So should you spend time finding
the best? I have some words on the topic.

<!-- excerpt end -->

If a primary goal of your next project is to learn a new MCU, you want to create
something easily reproducible, or if there will be only one machine building the
project, then I recommend you to go with the chip you want to learn, the chip
most readily available, or the easiest one to work with.

If you want to create a professional product that might be built thousands of
times with reasonable effort to be sold at a reasonable price point, then read
on.

{% include newsletter.html %}

{% include toc.html %}

## The Best MCU for an EE

I assume you, the reader, are the person responsible for the firmware of your
project and that you work with an electronics engineer (EE). If the EE presents
you with the finished PCB: Yeah, you don’t have to decide on the chip to use.
However, you might run into issues that the EE did not take into account.

For an EE the best chip has no pins at all, is the size of an SMD resistor, has
no special needs to power rails, runs at 0 Hz clock, and costs nearly nothing.
That would make it easy for the EE and getting it to work is within the
firmware's responsibility.

Often the EE will use the parametric search to find the microcontroller to use.
The parametric search is provided by the big distributors (Digikey, Farnell,
Mouser to name a few). It allows you to filter all available chips according to
their features. The EE can filter out all chips with the wrong voltage requirements,
unpractical packages (DIL, SMD, BGA), or improper operating temperature ranges
that the product needs to endure. These and many other filters are provided. In
the end, the EE sorts by price and selects the cheapest. Maybe they select a
handful and let the firmware engineer choose.

Easy right? Not exactly. The cheapest chip that meets the _hardware_ criteria is
usually one of the most painful chips to bring up and write firmware on. We've
all been there.

## EE and Firmware Engineers & MCU Features

The requirements of an MCU for a firmware engineer look quite different. They
care about significantly different things! Does the MCU have the peripherals
needed, does it have good performance, and enough RAM and flash memory. As for
the software requirements, firmware engineers are also looking for a solid set
of drivers, supported OS's, and tools. Researching all of this is tedious work.

One way around this is to stick to the processors already in use in other
projects. The argument would be that the people involved already know the chip,
the tools are already available, and buying more of it might make it even
cheaper also for the other projects. If the current standard chip can not be
used then you can probably find one that can be used in the same family or at
least from the same vendor that fits the requirements. That might not be the
cheapest chip, it might not even be on the EE’s list, but we can select it
anyway. We convince the manager that it is easier to use this chip as we already
have the knowledge and tools and that new tools would come in more expensive
than the cheaper chip the EE has found. And this might be the right way to go.
It all depends.

If you are true to yourself, you probably have to admit that the effort planned
for “learning a new chip” is a combination of not wanting to learn something new
and the fear of running into issues with the new chip. There is the risk of
something going catastrophically wrong with the new chip, such as a missing
required feature or an unforeseen errata. But if the benefits outweigh the
risks, we should jump into the adventure.

The differences between chips are bigger than one would expect. For example, the
Raspberry Pi RP2040 is a 32bit dual-core processor that runs at a maximum of 130
MHz and costs about one dollar. The ATMEGA649-16MUR is an 8-bit single-core
processor running at a maximum of 16 MHz and costs $3.69. Pricing is a dark art
of the vendors and many factors influence it. If the vendor provides a widely
used framework for an established chip, the prices may go up (or not go down) as
people will want to stick with it. Then producing more of a chip that is being
used in a high turnaround product (Mobile phone,..) results in less effort than
switching the machines to something only a few customers need.

## The Best MCU for a Firmware Engineer

What if we turn the tables and come up with the firmware list of chips that we
then present to the EE? What should we filter the available chips for?

### Peripherals and Software Stack

We should probably start with the available peripherals. If the product needs
one or more UART, SPI, I2C, CAN, USB, or Ethernet interfaces, then the chip
should better have them. Be cautious not to filter out too many chips at this
stage. Especially if you are designing a product that will be mass-produced. A
small difference in price can become a huge factor. Therefore, if the required
peripherals limit the selection too much, think about bit-banging an interface.
If we only need to read a few bytes from an external memory using I2C then doing
that with two GPIO pins and some code will probably be OK. Also don’t forget
about the “internal” peripherals, DMA, crypto accelerators, (watchdog) timers,
etc.

We also need to check the compatibility of the chip. Interfaces like USB, CAN,
and Ethernet are usually used in combination with a so-called “stack”. That is a
software library that implements all the complicated features of the interfaces.
If we need or want to use a certain stack then the peripheral in the selected
chip needs to be compatible. Otherwise, we might need to implement a driver or
switch to a different stack. Both can be a lot of work. Also, other code that we
want to reuse like libraries or (real-time) operation systems need to be checked
for compatibility. We need to weigh the price difference of the chip against the
amount of work needed to get the incompatible chip to work.

### Tooling

Another thing is tooling. We can not use a chip if we do not have a compiler to
generate code for it. We will need some way to flash the chip. (flash = write
the compiled code into the chip) A development board or simulator for the chip
will speed up firmware development before the project's PCB is available. A
debugger that lets you step through the code and check the values of variables
speeds up the finding of issues. If it can also provide trace information then
that helps to find the hard-to-find bugs. An IDE with syntax highlighting, that
allows for debugging and integrates with the version control system, will help
make software development more efficient.

The freely provided vendor tools might often be enough to get the firmware done.
But software development is often like driving in the dark. One has the map, so
what's the problem. But switching on the headlights helps with making the turns.
Good tools provide light. More light leads to fewer bugs. And therefore it leads
to new features faster. Even expensive tools can pay for themselves due to less
wasted development hours rather fast. The internet is full of war stories of
long bug hunts that would not have been necessary if the right tool would have
shone some light on the issue early on. Also, the best tools are usually not
chip-specific. Heck. Even the ST-provided ST-link debugger can work on Microchip
(and other) devices if used with OpenOCD! So don’t hesitate to ask your manager
for the expensive tools even if you could get it done with what you already
have. Less development time spent is a good argument.

### Community & Popularity

Another plus side for a chip can be if it is part of a bigger chip family. (The
base for the “standard chip” argument) Don’t value this too high. But in case of
an emergency, it can help. If at the end of firmware development the release
date of the product is close but the firmware is just a little bit too big, then
switching to the pin-compatible variant with just a bit more flash or a bit more
RAM might save the day. It is always good to have a plan B even if you never
expect to use it. Don’t plan to fail!

Community support and documentation quality might also be decision-leading
factors. Popularity plays a role. If you select a chip that nobody uses but you,
if the documentation is brief but incorrect and even the vendor suggests using a
different chip, then better listen. Writing firmware for a widely used chip
might help if you run into issues as someone else might have had the same issue
and already found the solution. It will also help on the compatibility side. The
chances that a new library you want to use has support for the chip will be
higher. But again just being a popular chip alone doesn’t mean much. But if you
need to decide between two otherwise equal chips, the more popular will probably
be the better choice. It also has a lower risk of production being stopped. So
unless you are in a chip shortage where the most popular chips are sold out the
fastest, the more popular chip is the better choice.

If you work in a company that buys a lot of chips then you can and should also
ask the distributors and their Field Application Engineers (FAE) to suggest a
chip that meets your requirements. Don’t expect them to recommend a competitor’s
chip though. It is also a good confirmation if the FAE recommends the same chip
you selected.

## Final Words

After all these considerations you now have filtered out the chips that won’t
work for this project. The ones left can be sorted by price and then let the EE
select the one he likes the most 😉.

If you are now asking where you can find the information to all chips to get
your selection process started, then that is the reason why I started
[ChipSelect](http://chipselect.org). It has just launched and will need a lot of
work. It's [open-source](https://github.com/JustAnother1/chipselect_www_php) and
I welcome any improvements.

It intends to help firmware engineers, like myself, find information regarding
the plethora of available microcontrollers and make it easier to find, decide
on, and get started with a new microcontroller.

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}
