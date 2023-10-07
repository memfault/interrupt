---
# This is a template for Interrupt posts. Use previous, more recent posts from the _posts/
# directory for more inspiration and patterns.
#
# When submitting a post for publishing, submit a PR with the post in the _drafts/ directory
# and it will be moved to _posts/ on the date of publish.
#
# e.g.
# $ cp _drafts/_template.md _drafts/my_post.md
#
# It will now show up in the front page when running Jekyll locally.

title: Visualizing embedded data made easy
description:
  Debugging common issues with STMViewer software 
author: Peter
---

If you've ever wanted to plot some data acquired on your embedded target this article is for you. It presents some common usecases for visualizing data in real time using STMViewer software. Put an end to manual, time consuming and error-prone ways of collecting and displyaing data to speed up your debugging process. 

<!-- excerpt start -->

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## What exactly can it visulize ?

STMViewer consists of two modules which serve a slightly different purpose: 

1. Variable Viewer - a module that reads variables' values directly from RAM using ST-Link SWD (only SWDIO, SWCLK and GND conenction is needed). It is asynchornous, which means it samples the memory with the time period set on the PC. It is completelty non-intrusive, and is able to log many variables at once. There are some trade-offs though. The first one is that logged variables' addresses have to be constant, meaning they have to be globals. Moreover with more aggressive optimization some of the variables may get optimized and fail to be logged.

2. Trace Viewer - a module that reads and parses SWO (Serial Wire Viewer) data using ST-Link (SWDIO, SWCLK, SWO and GND connection needed). It is synchornous, meaning it only visualizes the datapoint with the rate they are produced on the target. A datapoint can be either a special single byte flag that is used to create digital plots, or any value that fits in up to 4 bytes in case of "analog" plots. It has a minimal performance cost - a single register write per plot point, since the serialization and timestamping is handled by the ITM peripheral. It can be used to profile parts of the codebase, or to visualize signals that are too fast for the asynchornous Variable Viewer. The main limitation is the ST-Link maximum allowed baudrate on SWO pin, and the fact that the ITM peripheral is supported on only Cortex M3/M4/M7/M33 cores. 


## I'm already using STMStudio/CubeMonitor, why create a new tool?

Let's start with the most commonly asked question - why bother creating a tool that already exists and is provided by the ST? There are at least two reasons: 
1. STMStudio is deprecated. It was a very useful tool, however some annoying bugs made it really hard to work with. Moreover, it does not support mangled C++ names and is only available for Windows. 
2. CubeMonitor is just too much overhead for debugging purposes. The software is intended for creating dashboards, and in my opinion during debugging nobody really cares about some cool-looking gauges and indicators. Maniupulating the plots data is simply inconvenient and writing values to logged variables is even harder. Not to mention you can get some deployment errors when you connect the nodeRED puzzles the wrong way. 
3. Last but not least - none of the available hobby-level software tools contain a trace data visualizer. Im my opinion this is the most interesting STMViewer's module that allows to visualize variables in a synchronous way, profile functions or interrupts literally in minutes. 

## Variable Viewer

<!-- TODO GIF ? -->

As mentioned earlier Variable Viewer is an asynchronous module that samples predefined RAM addresses' values on your target. It can be especially useful when you want to visualize multiple, relatively slow signals (maximum sampling rate is around 1kHz). The RAM addresses are read from project's *.elf file that is parsed using GDB. For a quick start please refer to: [Variable Viewer](https://github.com/klonyyy/STMViewer#variable-viewer-1)


### PID controller
Let's see it in action on a step response of a classic servo cascaded PID:

<!-- TODO PICTURE PID -->

What we see are two cascaded controllers - position and velocity PID. The position PID is trying to follow the target and it generates velocity targets for the lower level controller, which in turn generates torque setpoints. When the responses are plotted out it's easy to notice the overshoots in the velocity PID response. Moreover we can clearly see when velocity target limit is triggered. You can easily measure the most important parameters of the response with built-in markers, or export a CSV for a more thorough analysis. 



## Trace Viewer 

<!-- TODO GIF ? -->

Trace Viewer is a synchronous module used to visualize SWO trace data. This means we can visualize fast actions without worrying  some data might be lost between sampling points. It does not require any configuration on the target since it is all done by the STMViewer using SWD. The only thing we have to do is to write data to the ITM->PORT[x] registers to send it. For example:

```c
ITM->PORT[0].u8 = 0xaa; //enter tag 0xaa - plot state high
foo();
ITM->PORT[0].u8 = 0xbb; //exit tag 0xbb - plot state low
```
can be used to profile a foo() function. Simple, isn't it? Of course you can wrap it up in a macro, but I wanted to keep is as simple as possible. No includes are needed since ITM is defined in CMSIS headers.

On the other hand this: 

```c
float a = sin(10.0f * i);          // some super fast signal to trace
ITM->PORT[x].u32 = *(uint32_t*)&a; // type-punn to desired size: sizeof(float) = sizeof(uint32_t)
```
can be used to visualize a float value.

Each register write generates two frames on the SWO pin - a data frame and a relative timestamp frame. The data frame is simply holding the channel value, size and the data, whereas the timestamp frame holds the time from last frame expressed in clock cycles. The fact that these frames can have a variable length, especially the timestamp one, makes it very effective in terms of utilizing the available SWO bandwidth. 

For a complete quick start please refer to: [Trace Viewer](https://github.com/klonyyy/STMViewer#trace-viewer-1)

Lets now see a couple of real-life examples. 

### Profiling function execution time

Let's check how much time it takes to copy a buffer using a well-known memcopy function. I prepared a small demo of ten copy operations, each time 64 bytes larger than the last one:

```c
	volatile uint8_t dest[1024];
	volatile uint8_t src[1024];
	volatile uint16_t size = 1;

	for (size_t i = 0; i < 10; i++)
	{
		ITM->PORT[1].u16 = size;
		ITM->PORT[0].u8 = 0xaa;
		memcpy(dest, src, size);
		ITM->PORT[0].u8 = 0xbb;
		size += 64;

		for (size_t a = 0; a < 300; a++)
			__asm__ volatile("nop");
	}
```

Besides the memcpy profiling we are also logging the current size on channel 1. As can be seen there is also a small blocking delay to make the plot more readable. Here are the results: 

<!-- TODO PICTURE MEMCPY -->

It seems that copying a single byte takes around 270ns, whereas copying 577 bytes takes approximately 25.35us @ 160Mhz. This was fast! 

### Profiling multiple interrupts

This example is going to be a bit simmilar to the previous one, except we will use many channels and try to see lower priority interrupts being preempted by a higher priority ones

I prepared three timers and set their interrupts priorities to: 0 (highest logical priority) for TIM7, 1 for TIM17 and 2 (lowest) for TIM6. I've put some dummy instructions in each interrupt to simulate some work being done.

<!-- TODO PICTURE INTERRUPTS -->

We can clearly see how the highest priority execution stops all other interrupts, and only after it finishes the other interrupts are resumed. What is really important in such experiments is that the marker register writes used in Trace Viewer are atomic so a higher interrupt cannot fire in between a datapoint generation.


### Fast ADC signal

Have you ever tried to visualize a fast ADC signal from an embedded target? Some time ago I had to check a step response of a low level current controller of a motor which had a timestep of 25us. I considered using DAC and an oscilloscope, or recording and replaying the data after the event. Both methods are time consuming and torublesome. Let's see how TraceViewer can handle this task - we will command a voltage step on an inductor and read back the current.

<!-- TODO PICTURE INDUCTOR -->

Now, knowing the test voltage, calculating resistance and measuring rise time we can easily determine the inductane!



<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^reference_key]: [Post Title](https://example.com)
<!-- prettier-ignore-end -->