---
title: "Peeking inside CMSIS-Packs"
description: Overview of the CMSIS-Pack distribution and some use cases
author: noah
tags: [toolchain, arm]
---

CMSIS-Packs are a package format defined by ARM for vendors to provide various
software artifacts that simplify dealing with target devices.

<!-- excerpt start -->

In this article, we'll take a look at what CMSIS-Packs are, and how they can be
useful!

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## CMSIS-Pack Contents

The format and associated tooling are described here:

> <https://arm-software.github.io/CMSIS_5/Pack/html/index.html>

A CMSIS-Pack is a ZIP file (with the `.pack` extension).

The actual contents of an individual pack can vary a lot, but in general,
there's the minimum items below (which are usually the most useful too):

1. a C header providing peripheral definition file for the chip(s) supported by
   the pack
2. startup files / linker scripts for various toolchains
3. SVD files that describe the peripheral registers
4. flash algorithms (FLM files) for reading + writing on-board non-volatile
   memory
5. peripheral driver files

There's often a lot more included in a Pack, for example:

- documentation (datasheets or user manuals)
- various tools (flash utilities, debug scripts)
- project templates
- board support packages
- middleware libraries
- sample projects

## Downloading CMSIS-Packs

The list of published CMSIS-Packs is available here:

> <https://developer.arm.com/tools-and-software/embedded/cmsis/cmsis-packs>

The `.pack` files can be downloaded directly from there.

However, there's another option for searching and retrieving CMSIS-Packs,
courtesy of one of my favorite tools: **PyOCD**

> PyOCD integrates CMSIS-Pack management as a way of accessing flash algorithms
> for chips not natively supported by the tool. It's a pretty awesome way to
> provide flashing support to a LOT of chips!<br>
> To learn more, check out our [Deep Dive into ARM Cortex-M Debug Interfaces]({% post_url 2019-08-06-a-deep-dive-into-arm-cortex-m-debug-interfaces %}).

For example, if I wanted the CMSIS-Pack for an Atmel SAMD21 chip, I can run
these commands from my terminal:

```bash
# install pyocd using the python package manager, pip
❯ pip install pyocd
# search for my part
❯ pyocd pack find samd21
  Part            Vendor      Pack         Version   Installed
----------------------------------------------------------------
  ATSAMD21E15A    Microchip   SAMD21_DFP   3.5.132   True
  ATSAMD21E15B    Microchip   SAMD21_DFP   3.5.132   True
...
```

There are several chips supported by a single CMSIS-Pack in this case. I can
download the pack using the `install` command (it will match the correct pack
based on the shortened name I provide, or you can specify the exact chip name,
both are identical in this case but may be different for other chips):

```bash
❯ pyocd pack install samd21
Downloading packs (press Control-C to cancel):
    Microchip::SAMD21_DFP::3.5.132
Downloading descriptors (001/001)
```

Great! now that we've downloaded the pack... where is it and what do we do with
it?!

First we need to find it. PyOCD stores packs in a cache directory- we can use
this python one-liner to find out where that is:

```bash
❯ python -c 'import cmsis_pack_manager; print(cmsis_pack_manager.Cache(False, False).data_path)'
/home/noah/.local/share/cmsis-pack-manager

# on my system, it was under the home directory; see `man file-hierarchy`
# section on 'HOME DIRECTORY' for why it makes sense for PyOCD to use that!

# search that location for any *.pack files
❯ find "$(python -c 'import cmsis_pack_manager; print(cmsis_pack_manager.Cache(False, False).data_path)')" -name '*.pack'
/home/noah/.local/share/cmsis-pack-manager/Keil/STM32F4xx_DFP/2.15.0.pack
/home/noah/.local/share/cmsis-pack-manager/Microchip/SAMD21_DFP/3.5.132.pack

# I've got 2 pack files installed on my system. to extract one of them, I can
# unzip it somewhere
❯ mkdir -p /tmp/unpack
❯ cd /tmp/unpack
❯ unzip /home/noah/.local/share/cmsis-pack-manager/Microchip/SAMD21_DFP/3.5.132.pack
```

At this point, it's a bit of an exploration for the files we're interested in.
You can search for `*.svd` files, `*.FLM` files, linker scripts `*.ld`, CMSIS
header files, etc.

## Flash Algorithms

One breadcrumb of information that can be useful (and is how PyOCD uses the Pack
file to flash chips) is the flash algorithm, which is specified in the top-level
`*.pdsc` file. The `*.pdsc` format is detailed here:

<https://www.keil.com/pack/doc/CMSIS/Pack/html/packFormat.html>

The file contains many interesting pieces of information. One particularly useful
one is the Flash Algorithms used to program a piece of hardware. Look for a
section of the XML with the `<algorithm>` tag, for example:

_Note: I added the comments explaining each section_

```xml
<!-- device name -->
<device Dname="ATSAMD21E15BU">

<!-- processor information -->
<processor Dcore="Cortex-M0+"
           Dendian="Little-endian"
           Dmpu="NO_MPU"
           Dfpu="NO_FPU"/>

<!-- CMSIS header. This particular pack wants you to include a catch-all header,
which fans out to the specific header file for the chip ("samd21e15bu.h") -->
<compile header="samd21b/include/sam.h" define="__SAMD21E15BU__"/>

<!-- SVD file, containing peripheral register descriptions -->
<debug svd="samd21b/svd/ATSAMD21E15BU.svd"/>

<!-- Memory map, RAM and ROM regions -->
<memory id="IROM1"
        start="0x00000000"
        size="0x8000"
        default="1"
        startup="1"/>
<memory id="IROM2" start="0x00400000" size="0x400"/>
<memory id="IRAM1" start="0x20000000" size="0x1000" default="1"/>

<!-- flash algorithm- this is a small executable that is described here:
https://www.keil.com/pack/doc/CMSIS/Pack/html/flashAlgorithm.html

A flasher can load the executable into RAM on the chip, point the PC at one of
the functions inside it, and run it to perform flashing operations.

Note that this part has on-board FLASH as well as EEPROM, and uses different
algorithms for different regions!
-->
<algorithm name="samd21b/keil/Flash/ATSAMD21_32.FLM"
           start="0x00000000"
           size="0x00008000"
           default="1"/>
<algorithm name="samd21b/keil/Flash/ATSAMD21_32_EEPROM.FLM"
           start="0x00400000"
           size="0x00000400"
           default="1"/>
```

## Reset Strategies

The Pack Description (`*.pdsc`) file can also contain optional debug sequences:

<https://www.keil.com/pack/doc/CMSIS/Pack/html/pdsc_family_pg.html#element_sequences>

For example, the Pack for the NXP RT685 chip contains a sequence for resetting
the external flash on the EVK board:

```bash
# download the RT685 Pack
❯ pyocd pack install rt685
# unzip the pack files
❯ mkdir rt685-pack
❯ unzip -d rt685-pack ~/.local/share/cmsis-pack-manager/NXP/MIMXRT685S_DFP/13.1.0.pack
```

The `NXP.MIMXRT685S_DFP.pdsc` file in this pack contains a few interesting reset
sequences.

### `ResetFlash`

This sequence pulses the Reset pin on the external flash chip on the EVK board.
The addresses written in the sequence are various memory-mapped peripheral
registers (Power/Clocking, GPIO), documented in the chip user manual.

```xml
<sequence name="ResetFlash" Pname="cm33">
  <block/>
    <control if="(__connection &amp; 0x01)">
      <block>
        //  Reset external flash if connection for target debug
        Write32(0x40004130, 0x130U);
        Write32(0x40021044, 0x4U);
        Write32(0x40020074, 0x4U);
        Write32(0x40102008, 0x1000U);
        Write32(0x40102288, 0x1000U);
        DAP_Delay(100);
        Write32(0x40102208, 0x1000U);
    </block>
  </control>
</sequence>
```

### `ResetSystem`

This sequence performs a system reset, applicable to the EVK. It does the
typical AIRCR reset for a Cortex-M processor, resets the external flash, sets a
watchpoint and waits for the chip to halt after reset.

```xml
<sequence name="ResetSystem" Pname="cm33">
  <block>
    __dp = 0;
    __ap = 0;
      // System Control Space (SCS) offset as defined in Armv6-M/Armv7-M.
      __var SCS_Addr   = 0xE000E000;
      __var AIRCR_Addr = SCS_Addr + 0xD0C;
      __var DHCSR_Addr = SCS_Addr + 0xDF0;
      __var DEMCR_Addr = SCS_Addr + 0xDFC;
      __var tmp;
      //Halt the core
      Write32(DHCSR_Addr, 0xA05F0003);
      //clear VECTOR CATCH and set TRCENA
      tmp = Read32(DEMCR_Addr);
      Write32(DEMCR_Addr, tmp | 0x1000000);
      Sequence("ResetFlash");
      // Set watch point at SYSTEM_STICK_CALIB access
      Write32(0xE0001020, 0x50002034);
      Write32(0xE0001028, 0x00000814);
      __errorcontrol = 1;
      // Execute SYSRESETREQ via AIRCR
      Write32(AIRCR_Addr, 0x05FA0004);
    Sequence("WaitForStopAfterReset");
      __errorcontrol = 0;
  </block>
</sequence>
```

## SVD File

This is another XML file that contains information about the peripheral
registers in the chip. Under certain conditions, it's **_invaluable_**; some examples
I've used before:

- using [PyCortexMDebug in gdb]({% post_url 2020-10-20-advanced-gdb
  %}#svd-files-and-peripheral-registers) to pretty-print register values while
  debugging 😍
- generating language bindings over the memory-mapped peripherals:
  <https://github.com/rust-embedded/svd2rust>

> Tip: always be careful when using these SVD files; they occasionally can
> have mistakes in them, which can be really confusing. When in doubt, check the
> datasheet or reference implementations (eg CMSIS headers) as a separate
> source.

## Drivers, CMSIS Header, Linker Scripts, etc.

### CMSIS Header

Often a CMSIS Pack will contain a C header file with register definitions; in
our example pack above, it's `samd21b/include/samd21e15bu.h`, and it has the
well-known CMSIS style, defining interrupt vectors and peripheral registers:

_Excerpt with interrupt vector enum:_

```c
/* Interrupt Number Definition */
typedef enum IRQn
{
/******  CORTEX-M0PLUS Processor Exceptions Numbers ******************************/
  Reset_IRQn                = -15, /* -15 Reset Vector, invoked on Power up and warm reset */
  NonMaskableInt_IRQn       = -14, /* -14 Non maskable Interrupt, cannot be stopped or preempted */
  HardFault_IRQn            = -13, /* -13 Hard Fault, all classes of Fault    */
  SVCall_IRQn               =  -5, /* -5  System Service Call via SVC instruction */
  PendSV_IRQn               =  -2, /* -2  Pendable request for system service */
  SysTick_IRQn              =  -1, /* -1  System Tick Timer                   */

/******  SAMD21E15BU specific Interrupt Numbers ***********************************/
  PM_IRQn                   =   0, /* 0   Power Manager (PM)                  */
  SYSCTRL_IRQn              =   1, /* 1   System Control (SYSCTRL)            */
  WDT_IRQn                  =   2, /* 2   Watchdog Timer (WDT)                */
...
```

_Excerpt with peripheral registers structure access (the structures are defined
in a common file in this Pack):_

```c
#define ADC_REGS                         ((adc_registers_t*)0x42004000)                /* ADC Registers Address        */
#define DAC_REGS                         ((dac_registers_t*)0x42004800)                /* DAC Registers Address        */
```

### Peripheral Drivers

Packs may also contain device drivers (CMSIS-style or other); for example, the
Pack for the STM32F407 family contains both:

- STM32CubeF4 HAL drivers
- CMSIS-style generated drivers

Often these drivers will be less applicable in production than what's available
from the vendor directly; Packs may not be updated as regularly, or contain
simplified driver code.

### Linker Scripts and Startup Code

Look for files with `startup` in the name, typically `.c` or `.s` files. These
are usually pretty generic (initialize `.data` and `.bss`, call some clock init
function), but are a useful reference.

Example linker files are often provided, usually hit or miss whether GNU `ld`
format or IAR (`*.icf`) etc., varies from vendor to vendor. These can be a
useful reference, eg if there are special one-time-programmable regions in flash
that can result in bricked chips, or if there's a certain way RAM is banked on
the chip that isn't obvious from the datasheet, etc., but always check directly
with the vendor as well.

## Current Status of CMSIS-Packs

In April 2021, ARM transferred the CMSIS-Pack initiative to the Linaro IoT and
Embedded Group- read more here:

- <https://www.linaro.org/blog/arm-transfers-cmsis-pack-technology-to-linaro/>
- <https://www.open-cmsis-pack.org/>

This appears to be in active development right now and could be an interesting
project to keep an eye on:

- <https://github.com/Open-CMSIS-Pack/Open-CMSIS-Pack>
- <https://github.com/Open-CMSIS-Pack/devtools>

## References

- CMSIS Pack official documentation:<br/>
  <https://arm-software.github.io/CMSIS_5/Pack/html/cp_Packs.html>
- Pack
  registry:<br/>
  <https://developer.arm.com/tools-and-software/embedded/cmsis/cmsis-packs>
- PyOCD documentation on Packs:<br/>
  <https://github.com/pyocd/pyOCD/blob/main/docs/target_support.md#cmsis-packs>

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}
