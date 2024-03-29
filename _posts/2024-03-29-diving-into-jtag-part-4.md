---
title: Diving into JTAG — BSDL (Part 4)
description: "An overview of BSDL files"
author: zamuhrishka
tags: [arm, cortex-m, mcu, debugging, debugger]
---

<!-- excerpt start -->

In the previous article of this series, we briefly touched on how `.bsd` files
written in Boundary Scan Description Language (BSDL) describe the structure of
the boundary scan chain and the instruction set. In this article, we will
examine this language's syntax more closely before seeing how `.bsd` files are leveraged
in JTAG testing in the next article.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

> **Note**: In this series of articles, I discuss JTAG from the firmware
> engineer's point of view and to the extent necessary for the firmware
> engineer's work. I do not aim to thoroughly describe the syntax and format of
> BSDL, rather describe it to the extent necessary for simple understanding of
> the files supplied by the vendor chips. Also, I concentrate on the BSDL
> format, although other formats can be used to describe TAP chip controllers:
> SVF, STAPL, etc.

## Overview

Files with the `.bsd` extension describe how to interface with a particular chip
using the JTAG interface in the context of PCB and chip boundary-scan testing.
The main functions of these files are:

1. **Chip Configuration Description:** They provide a detailed description of
   the chip pin configuration, including which pins support JTAG.
2. **Testing and Diagnostics Support:** They support testing the PCB for shorts,
   open circuits, and other common problems. This is especially useful for
   verifying that components are properly mounted on the board.
3. **Test Automation:** They allow you to automate the testing process by
   providing the necessary information to the software that controls the test
   equipment.
4. **Test Portability:** Since BSDL is based on a standard, the files can be
   used with a variety of hardware and software that supports the standard,
   providing a high degree of test portability.

These files are a key component in the testing process of electronic devices,
especially in complex systems where accurate and efficient component testing is
important.

BSDL is a subset of VHDL, a hardware description language that describes how a
particular JTAG device must be implemented for boundary scan. You’ll find `.bsd`
files available from most vendors.

One of the major uses of BSDL is as an enabler for the development of tools to
automate the testing process based on IEEE 1149.1. Tools developed to support
the standard can control the TAP if they know how the boundary-scan architecture
was implemented in the device. Tools can also control the I/O pins of the
device. BSDL provides a standard machine and human-readable data format for
describing how IEEE 1149.1 is implemented in a device.

A BSDL description for a device consists of the following elements:

- Entity Descriptions
- Generic Parameters
- Logical Port Description
- Use Statements
- Pin Mapping(s)
- Scan Port Identification
- IDCODE Register Description
- Instruction Register Description
- Register Access Description
- Boundary Register Description

Let's jump into each of these and walk through some example snippets from `.bsd`
files.

## Entity Descriptions

The entity statement names the entity, such as the device name (e.g.,
`STM32F405_415_407_417_LQFP100`). An entity description begins with an `entity`
statement and terminates with an `end` statement:

```plaintext
entity XYZ is
    {statements to describe the entity go here}
end XYZ
```

All of the rest of the attributes and descriptions that we will look at we can
be located within this block. This structure allows the device to be described
for testing, debugging, and diagnostic purposes, providing the necessary
information for tools and equipment utilizing boundary scan technology.

## Generic Parameters

The `generic` keyword is used to define a parameter that can be changed when
implementing the description in a particular device.

```plaintext
generic (PHYSICAL_PIN_MAP : string := "DW");
```

In this example, `PHYSICAL_PIN_MAP` is a generic parameter that defines the chip
body type, which will default to the value "DW".

## Logical Port Description

The port description gives **logical** names to the I/O pins, and defines their
function as input, output, bidirectional, and so on.

```plaintext
port (
    OE    : in      bit;
    Y     : out     bit_vector(1 to 3);
    A     : in      bit_vector(1 to 3);
    GND   : linkage bit;
    VCC   : linkage bit;
    NC    : linkage bit;
    TDO   : out     bit;
    TMS   : in      bit
    TDI   : in      bit
    TCK   : in      bit
);
```

The `port` block contains both "useful" pins of the chip and the JTAG pins, as
well as power, ground, and missing pins - pins that are not supposed to be
connected to the chip.

If the port is a single port, it is marked as `bit`. If it is a bus, it is
labeled as `bit_vector(X to Y)`. Inputs are `in`, outputs are `out`, and
bidirectional lines are `inout`. Power/ground lines, analog I/O, and pins not
connected to the crystal are marked as `linkage`.

## Use Statements

The use statement indicates that a particular package or library will be used in
the current context. For example:

```plaintext
use STD_1149_1_1994.all;
```

indicates that this file uses the IEEE 1149.1 1994 version of the IEEE 1149.1
standard library. In the BSDL context, this expression indicates that the
definitions and constructs provided in the 1994 IEEE 1149.1 standard are used to
describe the boundary scan behavior. This ensures that the description is
compatible with tools supporting that standard.

## Pin Mapping(s)

This element maps logical signals from the Logical Port Description section onto
the physical pins of a particular device package. This is necessary because the
logical signals of a chip may be the same, but the specific pins are different
for different packages. For example:

```plaintext
attribute PIN_MAP of XYZ : entity is PHYSICAL_PIN_MAP;

constant DW:PIN_MAP_STRING:=
    "OE  :  1," &
    "Y   :  (2,3,4)," &
    "A   :  (5,6,7)," &
    "GND :  8," &
    "VCC :  9," &
    "TDO :  10," &
    "TDI :  11," &
    "TMS :  12," &
    "TCK :  13," &
    "NC  :  14";
```

The distribution of single pins is quite transparent: one logic name - one pin:

```plaintext
"OE  :  1," &
```

The buses are described next way:

```plaintext
"Y   :  (2,3,4)," &
```

## Scan Port Identification

The scan port identification statements define the device's TAP. For example:

```plaintext
attribute TAP_SCAN_CLOCK of TCK : signal is (10.0e6, BOTH);
attribute TAP_SCAN_IN of TDI : signal is TRUE;
attribute TAP_SCAN_OUT of TDO : signal is TRUE;
attribute TAP_SCAN_MODE of TMS : signal is TRUE;
```

`TAP_SCAN_CLOCK`, `TAP_SCAN_IN`, `TAP_SCAN_OUT`, and `TAP_SCAN_MODE` are
constants defining standard JTAG signals. `TCK`, `TDI`, `TDO`, `TMS` are the
port names defined in the **Pin Mapping(s)** section for a particular chip. The
value `TRUE` indicates active use of this port.

Additionally, the `TAP_SCAN_CLOCK` attribute specifies the maximum frequency of
the clock signal, in this example it is 10 MHz and that the signal is active on
both clock edges (rising and falling). Only the `BOTH` parameter is used for the
clock signal, but in general the following parameters can also be used for
signals: `RISING` or `FALLING`.

## IDCODE Register Description

The `IDCODE_REGISTER` attribute is used to define the `IDCODE` register of the
chip. This attribute is part of the IEEE 1149.1 (JTAG) specification. The format
is:

```plaintext
attribute IDCODE_REGISTER of <entity> : entity is
    "{specific attribute value}";
```

For example:

```plaintext
attribute IDCODE_REGISTER of STM32F405_415_407_417_LQFP100: entity is
     "XXXX" &                  -- 4-bit version number
     "0110010000010011" &      -- 16-bit part number
     "00000100000" &           -- 11-bit identity of the manufacturer
     "1";                      -- Required by IEEE Std 1149.1
```

## Instruction Register Description

The Instruction Register description identifies the device-dependent
characteristics of the Instruction Register. For example:

```plaintext
attribute INSTRUCTION_LENGTH of XYZ : entity is 2;

attribute INSTRUCTION_OPCODE of XYZ : entity is
    "BYPASS (11), "&
    "EXTEST (00), "&
    "SAMPLE (10) ";

attribute INSTRUCTION_CAPTURE of XYZ : entity is "01";
```

The `INSTRUCTION_LENGTH` attribute defines the length of the `IR` register in
bits.

The `INSTRUCTION_OPCODE` attribute defines the command codes.

The `INSTRUCTION_CAPTURE` attribute defines the value that will be loaded into
the shift register when TAP enters the `Capture-IR` state, in other words, this
attribute specifies what the instruction register is initialized with.

## Register Access Description

The register access defines which register is placed between `TDI` and `TDO` for
each instruction.

As discussed in
[Part 1](https://interrupt.memfault.com/blog/diving-into-jtag-part1), various
codes placed in the instruction register can select various internal registers
as a data register: `BYPASS`, `IDCODE`, `BSR`. The `REGISTER_ACCESS` attribute
determines which instruction will connect which register as a data register. For
example:

```plaintext
attribute REGISTER_ACCESS of XYZ : entity is
    "BOUNDARY (EXTEST, SAMPLE), "&
    "BYPASS (BYPASS) ";
```

Similar to instructions that may be standard, or may be unique to a particular
chip manufacturer (or a particular model), the JTAG module's plug-in registers
may not be limited to a list of `DEVICE_ID`, `BYPASS`, and `BOUNDARY`. By
entering the `MY_INSTRUCTION_OPCODE` instruction in the `INSTRUCTION_OPCODE`
attribute, it is possible to tell the control software in the `REGISTER_ACCESS`
attribute that it wishes to connect a non-standard register if the corresponding
instruction has been loaded into the instruction register:

```plaintext
attribute REGISTER_ACCESS of SIMPLE_IC : entity is
    <...>
    "MY_REG[8] (MY_INSTRUCTION)," &
    <...>
```

Here [8] defines the length of the register.

In addition, you can specify in this attribute what value the register to be
connected should be initialized with when the JTAG module enters the
`Capture-DR` state:

```plaintext
"MY_REG[8] (MY_INSTRUCTION CAPTURES 01010101)," &
```

## Boundary Register Description

The Boundary Register description contains a list of boundary-scan cells, along
with information regarding the cell type and associated control. For example:

```plaintext
attribute BOUNDARY_LENGTH of XYZ : entity is 7;

attribute BOUNDARY_REGISTER of XYZ : entity is
    "0 (BC_1, Y(1), output3, X, 6, 0, Z), "&
    "1 (BC_1, Y(2), output3, X, 6, 0, Z), "&
    "2 (BC_1, Y(3), output3, X, 6, 0, Z), "&
    "3 (BC_1, A(1), input, X), "&
    "4 (BC_1, A(2), input, X), "&
    "5 (BC_1, A(3), input, X), "&
    "6 (BC_1, OE, input, X), "&
    "6 (BC_1, *, control, 0)";
```

The `BOUNDARY_LENGTH` attribute specifies the total number of boundary scan
cells.

The `BOUNDARY_REGISTER` attribute specifies the configuration of each boundary
scan cell. Each entry in the attribute represents one cell and includes
information about the cell type, associated signal, mode of operation, and
optional parameters. The format of these entries starting with the first item
is:

- **Cell Number:** The number of the scanning cell and correspondingly the bit
  number in the `BSR` register.
- **Cell Type (BC_1, BC_2, etc.):** Indicates the cell class defined by IEEE
  1149.1. This may sound familiar because we discussed scan cell types in
  [the last article](https://interrupt.memfault.com/blog/diving-into-jtag-part-3#the-boundary-scan-cells).
- **Bound Signal:** For example, `Y(1)`, `A(1)` specifies the logical name of
  the signal bound to this cell.
- **Mode of Operation:** Indicates the cell's function: `input`, `output2`,
  `output3`, `bidir`, `control` or `controlr`. We also touched these functions
  in
  [the previous article](https://interrupt.memfault.com/blog/diving-into-jtag-part-3#the-boundary-scan-register-bsr).
- **Additional Parameters:** These include initial values or conditions for
  specific modes of cell operation. For example, for the STM32F407VG
  microcontroller, these additional parameters are as follows:
  - `Safe` - the value with which the BSR cell should be loaded for safe
    operation when the program can select a random value.
  - `ccel` - the control cell number. Specifies the control cell that drives the
    output enable for this port. As discussed in last week's article, there can
    be multiple scan cells per controller pin and in such cases usually one of
    the scan cells is a control cell, which controls the functionality of one or
    more other cells, for example, by switching them between input and output
    modes. And this parameter indicates the number of the cell which is the
    control cell for this cell
  - `disval` - the value to be written to the control cell for it to switch this
    port from the "output" state to the "input" state.
  - `rslt` - the resulting state. Shows the state of the driver when it is
    disabled.

## Conclusion

In this article we have covered the BSDL format and syntax that describe the
device hardware interface for the JTAG interface including pin mappings, pin
attributes, and control functions supplied chip vendors. I will use these files
in the next article to describe various JTAG usage scenarios that can be useful
for firmware engineers.

> **Note**: This article was originally published by Aliaksandr on his blog. You
> can find the original article
> [here](https://medium.com/@aliaksandr.kavalchuk/diving-into-jtag-protocol-part-4-bsdl-29fc4081502c).

<!-- Interrupt Keep START -->

{% include newsletter.html %}

{% include submit-pr.html %}

<!-- Interrupt Keep END -->

{:.no_toc}

## Links

- [JTAG Boundary Scanner](https://github.com/viveris/jtag-boundary-scanner)
- [JTAG with the BS (Boundary Scan) - pyjtagbs](https://github.com/colinoflynn/pyjtagbs)
- [Bringing JTAG Boundary Scan into 2021](https://circuitcellar.com/research-design-hub/design-solutions/bringing-jtag-boundary-scan-into-2021/)
- [DSP56300 JTAG Examples](https://www.nxp.com/docs/en/application-note/AN2074.pdf)
- [Bringing JTAG Boundary Scan into 2021](https://circuitcellar.com/research-design-hub/design-solutions/bringing-jtag-boundary-scan-into-2021/)
- [jtag-boundary-scan](https://github.com/jxwleong/jtag-boundary-scan)
- [Lauterbach: Boundary Scan User’s Guide](https://www2.lauterbach.com/pdf/boundary_scan.pdf)
- [Looking at JTAG: .bsdl by hand](https://habr.com/ru/post/660795/)
