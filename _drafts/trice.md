---
title: Trace & Log with `TRICE` and get `printf` comfort inside interrupts and everywhere
description:
author: thomas
image: /img/trice/cover.png
---

![]({% img_url trice/TriceCheckOutput.gif  %})


{% include toc.html %}


## Description

*Trice* is an unusual software tracer-logger and consists of these parts to use:

- [x] [trice.c](https://github.com/rokath/trice/tree/master/pkg/src/trice.c) containing the [less that 1KB](https://github.com/rokath/trice/tree/master/docs/TriceSpace.md) runtime code using [triceConfig.h](https://github.com/rokath/trice/tree/master/test/MDK-ARM_STM32G071RB/Core/Inc/triceConfig.h) as setup.
- [x] [trice.h](https://github.com/rokath/trice/tree/master/pkg/src/trice.h) containing a **C** language macro `TRICE`, generating [tiny code](https://github.com/rokath/trice/tree/master/docs/TriceSpeed.md) for getting real-time `printf` comfort at "speed-of-light" for any micro-controller.
  * [x] Example: `float x = 3.14159265/4; TRICE( Id(12345), "info:π/4 is %f with the bit pattern %032b\n", aFloat(x), x );`
- [x] PC tool **trice**, executable on all [Go](https://golang.org) platforms:
  * [ ] Android
  * [x] Linux
  * [ ] MacOS
  * [x] Windows
  * [ ] Interface options for log collectors are possible.


![]({% img_url trice/life0.gif  %})

## Abstract

If you develop software for an embedded system, you need some kind of system feedback. Debuggers are awesome tools, but when it comes to analyzing dynamic behavior in the field, they are not usable.

Logging then, usually done with printf-like functions, gets quick a result after having i.e. `putchar()` implemented. This turns out to be an expensive way in terms of processor clocks and needed FLASH memory, when you regard the library code and all the strings needing FLASH memory space. For small micro-controllers that´s it.

Bigger micro-controllers are coming with embedded trace hardware. To use it, an expensive tool is needed. Useful for analyzing complex systems, but for in-field related issues at least unhandy.

Unhappy with this situation, the developer starts thinking of using digital pins or starts emitting some proprietary LED blinking codes or byte sequences, difficult to interpret.

<!-- excerpt start -->

The *Trice* technique tries to fill this gap, being minimally invasive for the target and as comfortable as possible. It is the result of a long-year dissatisfaction and several attempts to find a loophole to make embedded programming more fun and this way more effective.
<!-- excerpt end -->

{% include newsletter.html %}

##  A brief history of *Trice*

Developing firmware means dealing with interrupts and often timing. How do you check, if an interrupt occurred? Ok, increment a counter and display it in a background loop with some printf-like function. What about time measurement? Set a digital output to 1 and 0 and connect a measurement device. Once, developing software for a real-time image processing device, I had no clue where in detail the processing time exploded when the image quality got bad. A spare analog output with a video interrupt synced oscilloscope gave me the needed information after I changed the analog output on several points in my algorithm. But, hey guys, I want to deal with my programming tasks and do not like all this hassle connecting wires and steer into instruments.

A `printf` is so cool on a PC, developing software there. But an embedded device often cannot use it for performance reasons. My very first attempt was writing the format string `.const` offset together with its values in a FIFO during a log statement and to do the `printf` it in the background. But that is compiler specific. Ok the full string address is better but needs more buffer space. [Zephyr](https://docs.zephyrproject.org/latest/reference/logging/index.html) for example does something like that calling it "deferred logging".

Then, one day I had the idea to compute short checksums for the format strings in a pre-compile step and to use them as ID in a list together with the format strings. That was a step forward but needed to write a supporting PC program. I did that in C++ in the assumption to get it better done that way. Finally, it worked but I hated my PC code, as I dislike C++ now because of all its nuts and bolts to handle, accompanied by missing libraries on the next PC. The tool usability was also unhandy and therefore error-prone and the need became clear for a full automatized solution. Also, what is, if 2 different format strings accidentally generate the same short checksum? There was a way around it, but an ID-based message filtering will never be possible that way.

The need became clear for controllable IDs and management options. And there was [Go](https://golang.org) now, an as-fast-as-**C** language, easy to learn, promising high programming efficiency and portability. It would be interesting to try it out on a real PC project.

Trying to add channels in form of partial *TRICE* macro names was blowing up the header code amount and was a too rigid design. Which are the right channels? One lucky day, I decided handle channels just as format string parts like `"debug:Here we are!\n"` and getting rid of them in the target code this way also giving the user [full freedom](https://github.com/rokath/trice/tree/master/internal/emitter/lineTransformerANSI.go) to invent any channels.

Another point in the design was the question of how to re-sync after a data stream interruption because that happens often during firmware development. [Several encodings](https://github.com/rokath/trice/tree/master/docs/TriceObsoleteEncodings.md) were tried out, a proprietary escape sequence format and an alternative flexible data format with more ID bits were working reliable but with [COBS](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing) things got satisfying. A side result of that trials is the **trice** tool option to add different decoders if needed.

There was a learning **not** to reduce the transmit byte count to an absolute minimum, but to focus more on `TRICE` macro speed and universality. That led to a double buffer on the target side discarding the previous FIFO solution. The [COBS package descriptor](https://github.com/rokath/trice/tree/master/docs/TriceMessagesEncoding.md#2-cobs-encoding-and-user-protocols) allowing alongside user protocols is result of the optional target timestamps and location info some users asked for, keeping the target code as light as possible. Float and double number support was implementable for free because this work is done mainly on the host side.

*Trice* grew, and as it got usable I decided to make it Open Source to say "Thank You" to the community this way.

Learning that *Trice*  is also a [baby girl name](https://www.babynamespedia.com/meaning/Trice), our daughter Ida designed the little girl with the pen symbolizing the `TRICE` macro for recording and the eyeglasses standing for the PC tool **trice** visualizing the logs.

![]({% img_url trice/TriceGirlS.png  %})

##  How it works - the main idea

*Trice* performs **no** [costly](https://github.com/rokath/trice/tree/master/docs/TriceVsPrintfSimilaritiesAndDifferences.md#printf-like-functions) printf-like functions on the target at all. The `TRICE` macro, instead, just copies an ID together with the optional values to a buffer and is done. In the minimum case, this can happen in [6-8](https://github.com/rokath/trice/tree/master/docs/TriceSpeed.md) processor clocks even with target timestamps included. When running on a 64 MHz clock, **light can travel about 30 meters in that time**.

To achieve that, an automatable pre-compile step is needed, executing a `trice update` command on the PC. The **trice** tool parses then the source tree for macros like `TRICE( "msg: %d Kelvin\n", k );` and patches them to `TRICE( Id(12345), "msg: %d Kelvin\n", k );`, where `12345` is a generated 16-bit identifier copied into a [**T**rice **I**D **L**ist](https://github.com/rokath/trice/tree/master/til.json). During compilation than, the `TRICE` macro is translated to the `12345` ID only, and the optional parameter values. The format string is ignored by the compiler.

The target code is [project-specific](https://github.com/rokath/trice/tree/master/test/MDK-ARM_STM32G071RB/Core/Inc/triceConfig.h) configurable.  In **immediate mode**, the stack is used as *Trice* buffer and the TRICE macro execution includes the quick [COBS](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing) encoding and the data transfer. This more straightforward and slower architecture can be interesting for many cases because it is anyway much faster than printf-like functions calls. In **deferred mode** a service swaps the *Trice* double buffer periodically, the COBS encoding takes part and with the filled buffer the background transfer is triggered. Out buffer and half *Trice* buffer share the same memory for efficiency.

During runtime, the PC **trice** tool receives all what happened in the last ~100ms as a COBS package from the UART port. The `0x30 0x39` is the ID 12345 and a map lookup delivers the format string *"msg: %d Kelvin\n"* and also the bit width information. Now the **trice** tool can write target timestamp, set msg color and execute `printf("%d Kelvin\n", 0x0000000e);`

---
  ![]({% img_url trice/triceCOBSBlockDiagram.svg  %})

The **trice** tool is a background helper giving the developer focus on its programming task. The once generated ID is not changed anymore without need. If for example the format string gets changed into `"msg: %d Kelvin!\n"`, a new ID is inserted automatically and the reference list gets extended. Obsolete IDs are kept inside the [**T**rice **I**D **L**ist](https://github.com/rokath/trice/tree/master/til.json) for compatibility with older firmware versions. It could be possible when merging code, an **ID** is used twice for different format strings. In that case, the **ID** inside the reference list wins and the additional source gets patched with a new **ID**. This may be unwanted, but patching is avoidable with proper [ID management](https://github.com/rokath/trice/tree/master/docs/TriceIDManagement.md). The reference list should be kept under source code control.

##  *Trice* features

###  Open source

Target code and PC tool are open source. The MIT license gives full usage freedom. Users are invited to support further *Trice* development.

###  Easy-to-use

Making it [facile](https://github.com/rokath/trice/tree/master/docs/TriceUsageGuide.md) for a user to use *Trice* was the driving point just to have one **trice** tool and an additional source file with a project-specific simple to use `triceConfig.h` and to get away with the one macro `TRICE` for most situations. *Trice* understands itself as a silent helper in the background to give the developer more focus on its real task. If, for example, `trice log` is running and you re-flash the target, there is ***no need to restart*** the **trice** tool. When [til.json](https://github.com/rokath/trice/tree/master/til.json) was updated in a pre-build step, the **trice** tool automatically reloads the new data during logging.

The **trice** tool comes with many command-line switches (`trice help -all`) for tailoring various needs, but mostly these are not needed. Usually, only type `trice l -p COMn` for logging with a 115200 bit/s baud rate.

###  Small size - using *Trice* <u>frees</u> FLASH memory

Compared to a printf-library code which occupies [1](https://github.com/mludvig/mini-printf) to over [20](https://github.com/mpaland/printf#a-printf--sprintf-implementation-for-embedded-systems) KB FLASH memory, the *Trice* code is really [small](https://github.com/rokath/trice/tree/master/docs/TriceSpace.md) - less 1 KB will do already but provide full support.

###  Execution speed

Can it get faster than [that](https://github.com/rokath/trice/tree/master/docs/TriceSpeed.md)? Only 3 runtime Assembler instructions per *Trice* needed in the minimum case! Additionally target timestamp and location, disable interrupts, and restore interrupt state and cycle counter increment can consume a few more processor clocks, if enabled, but a *Trice* is still incomparable fast.

###  Robustness

When a *Trice* data stream is interrupted, the [COBS](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing) encoding allows an immediate re-sync with the next COBS package delimiter byte and a default *Trice* cycle counter gives a high chance to detect lost *Trice* messages.

###  More comfort than printf-like functions but small differences

*Trice* is usable also inside interrupts and additional [format specifier support](https://github.com/rokath/trice/tree/master/docs/TriceVsPrintfSimilaritiesAndDifferences.md#Extended-format-specifier-possibilities) gives options like binary or bool output. Transmitting runtime generated strings could be a need, so a `TRICE_S` macro exists supporting the `%s` format specifier for strings up to 1000 bytes long. It is possible to log float/double numbers using `%f` and the like, but the numbers need to be covered with the function `aFloat(x)` or `aDouble(y)`. Also, UTF-8 encoded strings are implicitly supported if you use UTF-8 for the source code.

![]({% img_url trice/UTF-8Example.PNG %})

###  Labeled channels, color and log levels

You can label each *Trice* with a channel specifier to [colorize](https://github.com/rokath/trice/tree/master/docs/TriceColor.md) the output. This is free of any runtime costs because the channel strings are part of the log format strings, which are not compiled into the target. The **trice** tool will strip full lowercase channel descriptors from the format string after setting the appropriate color, making it possible to give each letter its color.

Loggers use log levels and offer a setting like "log all above **INFO**" for example. The *Trice* channels can cover that but can do better: Inside [emitter.ColorChannels](https://github.com/rokath/trice/tree/master/internal/emitter/lineTransformerANSI.go) all common log levels defined as *Trice* channels alongside with user channels. The user can adjust this. The **trice** tool has the `-pick` and `-ban` switches to control the display in detail. Also a `-logLevel` switch can be used to set a display threshold as channel position inside ColorChannels.

If a target side log level control is needed, a **trice** tool extension could each of these log level channels assign an ID range and a target side log threshold can control which IDs are transmitted. No need to implement that right now, because the runtime and bandwidth costs are so small for each *Trice* and a back control path is needed which is better designed by the user. Also, the [IDManagement](https://github.com/rokath/trice/tree/master/docs/TriceIDManagement.md) would get more complex.

![]({% img_url trice/COLOR_output.png %})


###  Compile-time enable/disable `TRICE` macros on file level 

After debugging code in a file, there is [no need to remove or comment out `TRICE` macros](https://github.com/rokath/trice/tree/master/docs/TriceConfiguration.md#target-side-trice-on-off). Write a `#define TRICE_OFF` just before the `#include "trice.h"` line and all `TRICE` macros in this file are ignored completely by the compiler, but not by the **trice** tool. In the case of re-constructing the [**T**rice **ID** **L**ist](https://github.com/rokath/trice/tree/master/til.json) these no code generating macros are regarded.

```C
//#define TRICE_OFF // enable this line to disable trice code generation in this file object
#include "trice.h"
```

###  Target and host timestamps 

Enable target timestamps with a variable you want inside [triceConfig.h](https://github.com/rokath/trice/tree/master/test/MDK-ARM_STM32G071RB/Core/Inc/triceConfig.h). This adds a 32-bit value to each *Trice* sequence, which carries the the system clock, a millisecond second or another event counter. The **trice** tool will automatically recognize and display them in a default mode you can control. If several `TRICE` macros form a single line, the **trice** tool only displays the target timestamp of the first `TRICE` macro.

Embedded devices often lack a real-time clock and some scenarios can last for weeks. Therefore the **trice** tool precedes each *Trice* line with a PC timestamp, if not disabled. This is the *Trice* reception time on the PC, what can be some milliseconds later than the target *Trice* event.

###  Target source code location 

Some developers like to see the `filename.c` and `line` in front of each log line for quick source location. Enable that inside [triceConfig.h](https://github.com/rokath/trice/tree/master/test/MDK-ARM_STM32G071RB/Core/Inc/triceConfig.h). This adds a 32-bit value to the *Trice* sequence containing a 16-bit file ID and a 16-bit line number. The file ID is generated automatically by inserting `#define TRICE_FILE Id(nnnnn)` in each source.c file containing a `#include "trice.h"` line. 

```C
#include "trice.h"
#define TRICE_FILE Id(52023)
```

Because software is bound to change, it could happen you get obsolete information this way. Therefore the **trice** tool log option `-showID` exists to display the *Trice* ID in front of each log line what gives a more reliable way for event localization in some cases. Also, you can get it for free because no target code is needed for that. 

###   Several target devices in one log output

Several **trice** tool instances can run parallel on one or different PCs. Each **trice** tool instance receives *Trices* from one embedded device. Instead of displaying the log lines, the **trice** tool instances can transmit them over TCP/IP (`trice l -p COMx -ds`) to a **trice** tool instance acting as a display server (`trice ds`). The display server can fold these log lines in one output. For each embedded device a separate *Trice* line prefix and suffix is definable. This allows comparable time measurements in distributed systems. BTW: The *Trice* message integration could be done also directly with the COBS packages.

###  Any byte capable 1-wire connection usable

The usual *Trice* output device is an [UART](https://github.com/rokath/trice/tree/master/docs/https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter) but also [SEGGER-RTT](https://github.com/rokath/trice/tree/master/docs/TriceOverRTT.md) is supported over J-Link or ST-Link devices. Many microcontroller boards can act as a *Trice* bridge to a serial port from any port ([example](https://github.com/rokath/trice/tree/master/docs/TriceOverOneWire.md)).

###  Scalability

The various [*Trice* ID management features](https://github.com/rokath/trice/tree/master/docs/TriceIDManagement.md) allow the organization also of bigger software systems. More than 65000 possible different IDs should match also large projects. Just in case: 16-bit for the ID is a not to hard changeable value.

###  Portability and Modularity

The **trice** tool is written in the open source language [*Go*](https://go.dev/) and is therefore usable on many platforms. That means the automatic code patching and ID handling side with `trice update`.

All C-compilers should be able to compile the target *Trice* code and there is no hardware dependency despite the byte transmission. MCUs with 8-bit to 64-bit, little or big-endian are supported.

Any user program able to read a [JSON](https://github.com/rokath/trice/tree/master/til.json) file, can receive the [documented](https://github.com/rokath/trice/tree/master/docs/TriceMessagesEncoding.md) *Trice* message format, look up the ID, and perform a printf-like action to translate into log strings. The **trice** tool with its `log` switch is a working example.

Using [COBS](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing) packages starting with a [package descriptor](https://github.com/rokath/trice/tree/master/docs/TriceMessagesEncoding.md#package-mode-prefix) allows alongside user protocols. The other way around is also implementable: In a user protocol embedded `Trice` messages.

The **trice** tool is expandable with several decoders. So it is possible to implement a minimal *Trice* encoding, if bandwidth matters heavily and control that with switches.

When less RAM usage is more important the target double buffer is replaceable with a FIFO. So the user will be able to decide at compile time about that. Right now, an immediate mode is selectable inside [triceConfig.h](https://github.com/rokath/trice/tree/master/test/MDK-ARM_STM32G071RB/Core/Inc/triceConfig.h) avoiding any buffer by paying a time toll.

The **trice** tool supports [many command line switches](https://github.com/rokath/trice/tree/master/docs/TriceUsageGuide.md#9-options-for-trice-tool).

###  Optional *Trice* messages encryption

The encryption opportunity makes it possible to test thoroughly a binary with log output and release it without the need to change any bit but to make the log output unreadable for a not authorized person. Implemented is the lightweight [XTEA](https://de.wikipedia.org/wiki/Extended_Tiny_Encryption_Algorithm) as option, what will do for many cases. It should be no big deal to add a different algorithm.

##  Bottom line

The *Trice* technique is new and still under development. Additional tests and bug fixing is necessary. A **trice** tool [configuration file](https://github.com/rokath/trice/tree/master/docs/TriceConfiguration.md#host-configuration-file) and interfacing [Grafana](https://grafana.com/) or similar tools would be possible extensions. Getting started with *Trice* will take a few hours, but probably pay off during further development.

##  A few maybe interesting links

<!--
* [https://mcuoneclipse.com/2016/10/17/tutorial-using-single-wire-output-swo-with-arm-cortex-m-and-eclipse/](https://mcuoneclipse.com/2016/10/17/tutorial-using-single-wire-output-swo-with-arm-cortex-m-and-eclipse/)
* [https://mcuoneclipse.com/2016/11/05/tutorial-getting-etm-instruction-trace-with-nxp-kinetis-arm-cortex-m4f/](https://mcuoneclipse.com/2016/11/05/tutorial-getting-etm-instruction-trace-with-nxp-kinetis-arm-cortex-m4f/)
* [https://interrupt.memfault.com/blog/a-deep-dive-into-arm-cortex-m-debug-interfaces](https://interrupt.memfault.com/blog/a-deep-dive-into-arm-cortex-m-debug-interfaces)
* [https://interrupt.memfault.com/blog/instruction-tracing-mtb-m33](https://interrupt.memfault.com/blog/instruction-tracing-mtb-m33)
* [python script is used to parse ITM trace packets from the SWO pin on the STM32 using OpenOCD](https://github.com/robertlong13/SWO-Parser)
* [MCUXpresso IDE Instruction Trace](https://www.nxp.com/docs/en/quick-reference-guide/MCUXpresso_IDE_Instruction_Trace.pdf)
* [Arm CoreSight technology which introduces powerful new debug and trace capabilities](https://www2.keil.com/coresight)
-->

<!-- prettier-ignore-start -->
* [printf/log Debugging](https://emp.jamesmunns.com/debug/logging.html)
* [NanoLog - extremely performant nanosecond scale logging system](https://github.com/PlatformLab/NanoLog)
* [baical - high-performance tools set for sending and receiving traces](http://baical.net/index.html)
* [*Trice* on Github with examples](https://github.com/rokath/trice)
* [*Go* home](https://go.dev/)
<!-- prettier-ignore-end -->