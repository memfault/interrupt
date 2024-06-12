---
title: Diving into JTAG — Usage Scenarios (Part 5)
description: "Some examples of JTAG usage"
author: zamuhrishka
tags: [arm, cortex-m, mcu, debugging, debugger]
---

<!-- excerpt start -->

In previous articles, we have considered the primary uses of JTAG, including
debugging and testing boards in production. For firmware developers, the first -
debugging - is the most common. In this article, I want to look at two uses of
JTAG Boundary Scan, which are also common tasks for a firmware developer: board
bring-up and reverse engineering.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Getting Started

To make working on this use case more convenient you need a specialized program.
In this article, we will look at working with the program
[TopJTAG Probe](http://www.topjtag.com/probe/) and debugger
[SEGGER J-Link](https://www.segger.com/products/debug-probes/j-link/). These
aren't the only tools suitable for this task, but I am a long-time fan of J-Link
debuggers, and the TopJTAG Probe program turned out to be the simplest and the
most accessible from a quick Google search.

While many people are familiar with SEGGER products, TopJTAG Probe requires an
introduction.

The TopJTAG Probe program is software that works with the JTAG interface and is
used for testing printed circuit boards (PCB). This software is commonly used to
diagnose and repair PCBs and work with microcontrollers and other devices that
support JTAG.

A boundary-scan (JTAG) based simple logic analyzer and circuit debugging
software. It provides the ability to monitor pin values in real-time without
interference with the normal operation of a working device and to interactively
set up pin values to test board-level interconnects or on-chip internal logic.

To start working with TopJTAG Probe, we need to create a new project. To do
this, go to the _File->New Project Wizard_ menu.

<p align="center">
 <img width="100%" src="{% img_url jtag-part5/topjtag-create-new-prj-step-1.png %}" alt="TOPJtag new project creation step 1" />
</p>

In the next window, select the required JTAG Probe and frequency. In my case, it
is SEGGER J-Link and 12 MHz.

<p align="center">
 <img width="100%" src="{% img_url jtag-part5/topjtag-create-new-prj-step-2.png %}" alt="TOPJtag new project creation step 2" />
</p>

Click _Next_.

The program will scan our JTAG circuit and display a list of available TAP IDs.

> You should already have a debugger connected to the board and the computer for
> this step to go well

<p align="center">
 <img width="100%" src="{% img_url jtag-part5/topjtag-create-new-prj-step-3.png %}" alt="TOPJtag new project creation step 3" />
</p>

Click _Next_.

At this point, you must specify .BSD files for each detected TAP. If you do not
specify a .BSD file for a TAP, it will be put into the `BYPASS` state.

We specify
[STM32F405\_415\_407\_417\_LQFP100.bsd](https://bsdl.info/details.htm?sid=61a8799988cb03f688ca59b002289d77)
file for the first TAP - SGS/Thomson (`06413041h`) as it is responsible for
Boundary Scan. We leave the second TAP in `BYPASS`. To select a file, click the
_CLICK HERE TO SET_ link and select the desired file.

<p align="center">
 <img width="100%" src="{% img_url jtag-part5/topjtag-create-new-prj-step-4.png %}" alt="TOPJtag new project creation step 4" />
</p>

After that, press _Finish_, and the program is ready.

Appearance of the program:

<p align="center">
 <img width="100%" src="{% img_url jtag-part5/topjtag-main-view.png %}" alt="TOPJtag the main window" />
</p>

`1` - Pins Window. The _Pins window_ lists all pins belonging to the selected
device. Any pin can be quickly located by typing its name, number, or port name
into the search box. The pin or bus name, PIN, port name, type, and current
value are displayed for every pin.

`2` - JTAG Chain (Main) View. The _JTAG Chain view_ (main program's view) shows
package outlines with color-coded pin values. These values are updated while the
boundary scan is running.

Device instructions are displayed below the package outline.

Pin values are color-coded. Black means 0, red means 1, brown means high-Z, and
blue means linkage pin. For bidirectional pins, it is possible to display either
input or output values.

`3` - Watch Window. The _Watch window_ keeps pins of interest in one place to
simplify monitoring and control.

`4` - Waveform Window. The _Waveform window_ records and displays waveforms.
This is a wonderful tool to view and analyze a signal's history. It is
particularly useful for fast-changing signals.

## Board Bring Up

Although assembled PCBs can be tested in production, as far as I know, it is not
a mandatory procedure. So when these boards come to the Embedded department,
especially when it's the first batch that firmware/embedded developers will work
with, the boards should be tested to ensure they work properly before they start
working on the firmware. Usually (at least in the case where I worked), this is
done by the circuit designer. Still, since the microcontroller is empty, this
check is limited to the test that the board does not burn up when turned on and
that the power supply system provides all the necessary voltages.

Sometimes, a special test firmware can be prepared specifically for bring-up,
which automatically or via the CLI interface can enable/disable board modules
for testing. In any case, no matter how it happens, bring-up almost always
requires interaction between the embedded and circuit designer and additional
work to create test firmware.

However, JTAG Boundary Scan can facilitate this work by allowing the circuit
designer to control the microcontroller pins and thus perform a better and
deeper analysis of the board without depending on the programmer. It also saves
the programmer from having to write special firmware for bring-up. You only need
a test board, a .BSDL file for the corresponding microcontroller, a debugger,
and a unique program to control the controller pins through GUI. Also, an
additional advantage of this approach is universality, i.e., you don't need to
write a new version of the test firmware for each new microcontroller. And if
you remember that Boundary Scan allows you to check the signal integrity for a
multiprocessor chain, where processors may have BGA cases and multilayer boards,
and it may be very difficult to get to the contacts with probes.

Let's see what is needed for an elementary bring-up. To begin with, at least,
you need to be able to control the microcontroller pins (set to logic one/logic
zero) and read the state in which the microcontroller pin is located, and JTAG
allows you to do this, and programs like TopJTAG Probe allow you to do it more
or less conveniently.

As a board for bring-up, I will use what I am already familiar with from the
previous parts.
[STM32F407G-DISC1](https://www.st.com/en/evaluation-tools/stm32f4discovery.html)
with microcontroller
[STM32F407VG](https://www.st.com/en/microcontrollers-microprocessors/stm32f407vg.html).

### GPIO output control

Let's control the LEDs on our board. Quite a workable task for bring-up. So we
know that the LEDs are connected to pins: `PD12`, `PD13`, `PD14`, `PD15`. They
are turned on by setting the pin to a logical one and turned off by a logic
zero. To select the required level on a pin, it is necessary to find this pin in
the _Pin_ window and select the necessary actions from the context menu:

<iframe width="100%" height="600px" src="https://www.youtube.com/embed/DrXdemXd80I" frameborder="0" allowfullscreen></iframe>

### GPIO input state view

Another necessary operation for such a bring-up is to view the state of the chip
pin. This can also be done using the JTAG Boundary Scan and TopJTAG application.
You can view the output state either in the _Watch_ window or in the _Waveform_
window. Let's look at the state of the output to which the button is connected:

<iframe width="100%" height="600px" src="https://youtube.com/embed/DrXdemXd80I" frameborder="0" allowfullscreen></iframe>

> **Note:** As mentioned in the article
> [Diving into JTAG. Part 3 - Boundary Scan](https://interrupt.memfault.com/blog/diving-into-jtag-part-3),
> there can be several scan cells per pin, and here you can see two of them: one
> for receiving and one for transmitting. When the controller transmits
> something, the receiving scan cell duplicates this signal, as seen in the
> video with LEDs. Still, if the output works only for receiving, the signal is
> present only on one cell, as seen in the video with the button.

## Reverse Engineering

One of the exciting things is that the `SAMPLE` instruction does not affect the
firmware in any way, which means that you can use it for a logic analyzer built
into the controller. Where can it be useful? For example, when you don't have an
oscilloscope or analyzer. And, as it seems to me, this function can be useful
when reverse engineering a board, especially microchips with BGA-type packages
or multilayer boards when it is difficult to determine with a simple multimeter
which pin of the chip is responsible for what and then using the `SAMPLE`
command can help.

However, this is a very simple logic analyzer. While seeing some very simple and
low-speed signals (such as controlling LEDs, pressing a button, or controlling
some load) with this analyzer, it is quite easy and simple as shown in the
chapter above, when dealing with higher-speed protocols, the signal that is
drawn on the Waveform is very distorted. Here is an example of how the
transmission of the symbol "**U**" (which has a code equal to `0x55`) through
UART at 1200 baud looks like:

<p align="center">
 <img width="100%" src="{% img_url jtag-part5/topjtag-uart-1200-baudrate-view.png %}" alt="TOPJtag uart 1200-baudrate-view" />
</p>

and at 2400 baud:

<p align="center">
 <img width="100%" src="{% img_url jtag-part5/topjtag-uart-2400-baudrate-view.png %}" alt="TOPJtag uart 2400-baudrate-view" />
</p>

As you can see, even at 2400 baud, the signal is distorted (2 bits were lost).

And here is what the SPI signal looks like with baudrate of 250 KBits/sec:

<p align="center">
 <img width="100%" src="{% img_url jtag-part5/topjtag-spi-view.png %}" alt="TOPJtag spi waveform" />
</p>

As you can see, it is hard to recognize SPI from this waveform, especially if
you look at the SCK signal.

Most likely, the impossibility of analyzing higher-speed signals in this way is
rooted in the limitations of the software and debugger that I use because they
are not designed for such a task. There may be software and hardware for this
purpose somewhere, but unfortunately, they are unavailable to me.

However, despite such limited use of this functionality, it can still be useful
in some specific cases: you can see which pins of the controller are alive and
functioning at all or analyze the logic of the firmware, for example on this
example:

<iframe width="100%" height="600px" src="https://youtube.com/embed/Z1xsTKtW4J8" frameborder="0" allowfullscreen></iframe>

You can see that after we press the button, communication starts on some
protocol. Although it is difficult to understand it from the signal, it is SPI
protocol.

## Conclusion

In this article, I have expanded on the area of JTAG usage. The examples may be
rare in real life, but they may be useful for someone. I will now use the JTAG
boundary scan for bring-up and board repair. :)

In the next article (most likely the final one), we will dive a bit into the
security issue related to JTAG: we will consider ways to restrict access to the
JTAG interface and also ways to bypass these restrictions.

> **Note**: This article was originally published by Aliaksandr on his blog. You
> can find the original article
> [here](https://medium.com/@aliaksandr.kavalchuk/diving-into-jtag-part-5-usage-scenarios-6fd8eda00e08).

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## Links

- [TopJTAG Probe Official Site](http://www.topjtag.com/probe/)
- [Segger Offical Site](https://www.segger.com/)
