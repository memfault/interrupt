---
title: "I2C in a Nutshell"
description:
  ""
tag: [buses]
author: francois
image: 
---

<!-- excerpt start -->
Lorem Ipsum
<!-- excerpt end -->

{:.no_toc}

## Table of Contents

<!-- prettier-ignore -->
* auto-gen TOC:
{:toc}


## Why use I2C



## Anatomy of an I2C transaction

I2C is made up of two signals: a clock (SCL), and a data line (SDA). By default,
the lines are pulled high by resistors. To communicate, a device pulls lines low
and releases them to let them rise back to high.


### Ones and Zeroes

As a digital bus, I2C ultimately transmits 1s and 0s between two circuits. It
does so using a very simple scheme: for pulse on the SCL line, one bit is
sampled from the SDA line. For this to work properly, the SDA line must remain
stable between the leading and falling edge of every SCL pulse. In other word,
the value of the SDA line while the SCL line is high is the value of the
transmitted bit.

For example, this is `0xC9` over I2C:

<script type="WaveDrom">
{ signal : 
  [
    { name: "SDA",  wave: "101.0.10.101." },
    { name: "SCL",  wave: "1.n........h."},
    { name: "bits", wave: "x.========x..",  data: ["1", "1", "0", "0", "1",
"0","0","1"]},
  ]
}
</script>

### Start and Stop

Since an I2C bus may have multiple masters, there must be a way to signal that
the bus is in use. This is accomplished with the START and STOP conditions.
Every I2C command starts with a START condition and ends with a STOP condition.

To send a START, an I2C master must pull the SDA line low while the SCL line is
high. After a START condition, the I2C master must pull the SCL line low and
start the clock.

To send a STOP, an I2C master releases the SDA line to high while the SCL
line is high.

In the diagram below, we indicate the START and STOP condition with "S" and "P"
respectively.

<script type="WaveDrom">
{ signal : 
  [
    { name: "SDA",  wave: "101.0.10.101." },
    { name: "SCL",  wave: "1.n........h."},
    { name: "bits", wave: "x==.......x=x",  data: ["S", "Data","P"]},
  ]
}
</script>

### Addresses

I2C is a multi-slave bus, so we must have a way to indicate which slave we would
like to send a given command to. This is accomplished with addresses. I2C
addresses are 7 bits, a few addresses are reserved and the rest are allocated
by the I2C-bus committee. Note that a 10-bit address extension does exist, but
is extremely uncommon[^1].

To communicate with a slave device, and I2C master simply needs to write its
7-bit address on the bus after the START condition. For example, the waveform
below captures an I2C transaction to a slave with address 0xC6:

<script type="WaveDrom">
{ signal : 
  [
    { name: "SDA",  wave: "101.0.1.010|.101." },
    { name: "SCL",  wave: "1.n........|...h."},
    { name: "bits", wave: "x==========|==x=x",  data: ["S", "1", "1", "0", "0", "1", "1","0","1","","0","1","P"]},
    { name: "data", wave: "x.=......=.|..x..", data: ["Address: 0xC6"]}
  ]
}
</script>


> **Address Conflicts**: Since the I2C address space is so limited, address
> conflicts are not uncommon. For example, you may want to include multiple
> instances of the same sensor on a single I2C bus.
> <br />To enable this use case IC designers typically allow their customers to set a
> few bits of a device's I2C address, either by typing pins high or low, or by
> writing a register. If all else fails, you may use an I2C multiplexer to
> resolve addressing conflicts.


### Reading and Writing

I2C masters may read or write to slave devices. This is indicated with a single
bit transmitted after the address bits. A `1` means the command is a read, and
a `0` means it is a write.

The example below shows a read sent to the slave device at address `0xC6`.

<script type="WaveDrom">
{ signal : 
  [
    { name: "SDA",  wave: "101.0.1.010|.101." },
    { name: "SCL",  wave: "1.n........|...h."},
    { name: "bits", wave: "x==========|==x=x",  data: ["S", "1", "1", "0", "0",
"1", "1","0","1","","0","1","P"]},
    { name: "data", wave: "x.=......==|..x..", data: ["Address: 0xC6", "W"]}
  ]
}
</script>

A common pattern is to send a write followed by a read. For example, many
devices expose several registers that can be read/written by first sending the
register address to the device in with a write command. 

To achieve this, the I2C master must first send a write command with the
register address, followed by a second START condition and a full read command
(including the slave address).

For example, this is what a write-then-read transaction to read register `0x11`
from the slave device at address `0xC6` would look like:

<script type="WaveDrom">
{ signal : 
  [
    { name: "", wave: "x==..==...==..==.=x", data: ["S", "Address: 0xC6", "W", "Reg Address: 0x11", "S", "Address: 0xC6", "R", "Data", "P"]}
  ]
}
</script>

### Ack/Nack

The I2C protocol specifies that every byte sent must be acknowledged by the
receiver. This is implemented with a single bit: `0` for ACK and `1` for NACK.
At the end of every byte, the transmitter releases the SDA line, and on the
next clock cycle the receiver must pull the line low to acklowledged the byte.

When the line remains high during the next clock cycle, it is considered a NACK.
This can mean one of several things:
1. A NACK after an address is sent means no slave responded to that address
2. A NACK after write data means the slave either did not recognize the command, or
   that it cannot accept any more data
3. A NACK during read data means the master does not want the slave to send any
   moe bytes.

Below is an example of a full I2C write command, with the ACK bits included. It
writes `0x9C` to the slave at address `0xC6`.

<!-- Can't get the narrow skin to work, so using an image
<script type="WaveDrom">
{ signal : 
  [
    { name: "SDA",  wave: "101.0.1.01010.1..0.101."},
    { name: "SCL",  wave: "1.n..................h."},
    { name: "bits", wave: "x===================x=x",  data: ["S", "1", "1", "0", "0", "1", "1","0","1","0", "1","0","0","1","1","1","0","0","1","P"]},
    { name: "data", wave: "x.=......===.......=x..", data: ["Address: 0xC6","W","A", "Data: 0x9C", "A"]}
  ],
  config: {skin: 'narrow'}
}
</script>
-->

![](/img/i2c-in-a-nutshell/full-i2c-command-narrow.png)



> Note: A NACK is not necessarily an error condition, it sometimes can be used to end
> a read. For example, reading 8 bytes from an I2C EEPROM would be implemented
> by sending a write command to set the EEPROM address offset we want to read from,
> followed by command which would NACK the 8th byte to signal to the EEPROM
> device that no more bytes are needed. This is simpler than a hypothetical
> protocol where we would also send a read length.


> NOTE ABOUT USES IRQ LINE TO COMMUNICATE SLAVE -> MASTER


## Debugging I2C


## Reference

[^1]: https://www.totalphase.com/support/articles/200349176

<!-- Wavedrom code to convert WaveDrom data to diagrams -->
<script src="https://cdnjs.cloudflare.com/ajax/libs/wavedrom/2.1.2/skins/default.js" type="text/javascript"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/wavedrom/2.1.2/skins/narrow.js" type="text/javascript"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/wavedrom/2.1.2/wavedrom.min.js" type="text/javascript"></script>
<script>
    if (document.addEventListener) {
      document.addEventListener("DOMContentLoaded", WaveDrom.ProcessAll(), false);
    }
</script>