---
title: Debugging One-in-a-Million Bugs
description:
  Techniques for working through two different one-in-a-million bugs
author: noah
image: img/<post-slug>/cover.png # 1200x630
---

Dealing with rare bugs in embedded devices can be a huge pain- there's often
very little information about the problem, and devices may operate differently
due to hardware or environmental variations!

<!-- excerpt start -->

This post will work through two different, very rare bugs I recently worked on.
We'll go over the strategies and breakthroughs, as well as some takeaways about
streamlining the process next time.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Bug #1: UART Data Loss/Corruption

This bug appeared during beta test of new BLE-connected device. A bug report
came in with the ever-dreaded:

> After a few days of use, the device stops syncing data to the mobile phone

😵‍💫 not a lot to go on. The error was happening _very_ rarely; it only seemed
to occur on a few specific devices, and required typically at least a day of
runtime before happening (sometimes several days until hitting the issue).

### Gathering Information, or The First Step is Realizing You Have a Problem

Fortunately, these devices record a variety of different error conditions, which
are available either via USB on a recovered device, or if the device is able to
sync. I messaged one of the users of an affected device, had them reboot it and
sync their data.

Now I could download the error logs and poke through, looking for any threads on
what happened.

Right before the device stopped reporting to our server, I found an error log
like the following:

```plaintext
2020-05-15T09:56:31Z BT transport error: expected 14 bytes got 12
```

OK! looks like we're seeing some errors when communicating with the Bluetooth
chip.

To debug it further, I added some additional logging to our next beta release-
printing out the abnormally truncated packet. When the error happens, that
should give us more information about what Bluetooth operation was in progress.

I also created a one-off build that would crash + store a coredump[^coredumps]
when hitting the error, and installed it to one device.

### Now We Wait

After pushing those changes, I was waiting for the error to reappear. Monitoring
for the error was a bit of a hassle; the debug logs of the affected devices were
uploaded to our server, and the simplest way to check for failures was to
download the logs for each device and grep for the failure log. I hacked a
little script up to run every hour or so on my computer, and I'd check
periodically whether any failures had happened.

About a day later a device hit the error condition. What I saw in the log:

```plaintext
2020-05-17T14:12:45Z BT transport error: expected 14 bytes got 12
2020-05-17T14:12:45Z BT transport error: 0e 0b 00 ab ...
```

The leading `0e` byte was unexpected- the serial protocol used with the chip had
a leading 8-bit `length` field, and here it was incorrect for the payload we
actually received!

Sadly the device with the coredump trigger did _not_ experience the error during
the test period 😞 which left me without the full context around the error.

### First Fix

This system typically stayed in a low-power mode, waking up when various
hardware components needed servicing. Specifically for the Bluetooth chip, there
was an IO pin used to signal a wakeup to the host MCU (Note: the UART interface
was using hardware flow control, hence the RTS signal below):

<script type="WaveDrom">
{ signal: [
  { name: "BT Host Wake", wave: "lh..l......" },
  { name: "MCU sleeping", wave: "hl........h"},
  { name: "Host RTS",     wave: "l.h......l."},
  { name: "BT UART TX",   wave: "h..3.4..h..", data: ["header", "payload"] },
]}
</script>

I suspected the junk byte was related to the chip's sleep mode, and sure enough,
the chip errata mentioned being sure to power down the UART RX hardware before
entering this specific low-power mode, to avoid data corruption 😠

Simple enough. I used some `libatomic`[^atomic-fetch-or] features to manage
required power state for the UART RX (it needed to be consistent between a
couple of interrupts and an RTOS task).

### Not Done Yet

After some bench testing I was pretty happy with this fix. I deployed it out to
the beta devices and 🤞 waited to see if the fielded devices hit the error.

Annnnd... about a day later, failure!

This error was a little different; no longer did we have the junk byte, but
instead the packet was truncated, then the remaining data was in a (malformed)
packet immediately following:

```plaintext
2020-05-18T01:26:35Z BT transport error: expected 14 bytes got 12
2020-05-18T01:26:35Z BT transport error: 0b 00 ab ...
2020-05-18T01:26:35Z BT transport error: bad packet header!
2020-05-18T01:26:35Z BT transport error: 01 00 00
```

I was at a bit of a loss now. The UART driver seemed to be giving up without
receiving the full packet from the Bluetooth chip. Normally for this type of
bug, I'd hook up a logic analyzer (for example the outstanding Saleae
probes[^saleae]) and get a capture of what was happening, but since I could only
rarely get the bug to occur on fielded devices, I needed another strategy.

### Breakthrough

When chatting with my coworker about the bug, he suggested I try to stress test
the driver by passing random packet lengths through it.

🤦

Obvious in hindsight! I quickly hacked together some code that would send a
sequence of packets of incrementing length to the Bluetooth chip, and have the
chip echo them back to the host MCU. I also had to hack my development board to
permit running this test while the device was running from (simulated) battery
power, so the host MCU would be entering and exiting sleep mode.

```plaintext
$ bt_fuzz 1000000
.......
2020-05-19T04:51:07Z BT transport error: expected 14 bytes got 12
```

Sure enough, I was able to reliably reproduce the issue! Now I could pull out
the logic analyzer and debugger.

> A common technique when dealing with these low-level issues is to wire up a GPIO
> pin to toggle at interesting points, which you can use to trigger an
> oscilloscope or logic analyzer.

Turns out, the UART peripheral on the host MCU used a 16-character FIFO.
Interrupts could only be configured to trigger at a minumum of 1/2 the FIFO
size, so we used a combination of 2 interrupt triggers when receiving a packet:

- FIFO half full (received at least 8 bytes)
- RX timeout (configurable number of symbol times without receiving any data)

Unfortunately we had a couple of bugs related to this behavior:

1. the Bluetooth chip would de-assert the `BT Host Wake` signal when it started
   sending data to the host MCU. Depending on how quickly the host MCU starting
   reading data, this could happen before the UART FIFO interrupt fired, in
   which case we'd end up only receiving a partial packet before shutting down
   the UART RX (the second portion would appear in the _next_ packet, since it
   was still residing in the RX FIFO! hence the malformed packets)
2. the vendor UART driver code had a missing check when reading the RX "FIFO
   Empty" bit in the UART status register; if the UART was depowerd, the
   register would sometimes return non-zero values, which caused our software to
   timeout trying to read non-existent data!

Finally, with those two fixes applied, the problem was solved on my bench setup
and in the beta test 😌.

### Post Mortem

Thinking back, this problem had a couple of qualities that made it especially
difficult:

- it rarely occurred, and I had very little information when it did occur
- only occurred on devices in low-power mode, where most of our debugging was
  done on USB-powered test devices!
- required specific timing conditions on both the host MCU and the Bluetooth
  chip (not all systems could reproduce the problem at all, due to hardware
  differences)
- my own bias when investigating the problem (tunnel vision!)

The real breakthrough came from someone without their head buried in the
problem. It really helps to get a second opinion when you're stuck!

## Bug #2: I<sup>2</sup>C ACK Failure

This bug showed up during early development of a different system. This
particular device had one main microcontroller that interfaced with a computer,
and several peripheral microcontrollers connected via UART to the main MCU. Each
peripheral microcontroller was responsible for managing several sensors and
control IC's (connected over various interfaces, primarily I2C and UART). An
approximate system diagram looks like this:

<!-- <style>
.diag1 {
    max-width: 800px;
    margin-left: auto;
    margin-right: auto;
}
</style> -->
{% blockdiag %}
blockdiag {
  "Computer" <-> "Main MCU";
  "Main MCU" <-> "Peripheral MCU" -> "Temp sensor";
  "Peripheral MCU" -> "Force sensor";
  "Peripheral MCU" -> "Servo";
  "Peripheral MCU" -> "Fan controller";

  "Computer" [color = "pink"];
  "Main MCU" [color = "lightblue"];
  "Peripheral MCU" [color = "orange"];
}
{% endblockdiag %}{:.diag1}

The system's overall timing requirements were fairly rigid- every
microcontroller (and its attached peripheral devices) had to respond within 2ms.

### The Error

During system bring-up we had 4 prototypes, which were under heavy testing and
modification. One system would infrequently encounter an error on the peripheral
MCU where it detected loss of response with one of the attached sensors.

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^coredumps]: [Memfault Coredumps](https://docs.memfault.com/docs/mcu/coredumps/)
[^atomic-fetch-or]: [`atomic_fetch_or` from the `stdatomic` library](https://en.cppreference.com/w/c/atomic/atomic_fetch_or)
[^saleae]: [Saleae logic analyzers](https://www.saleae.com/)
<!-- prettier-ignore-end -->

<!-- Wavedrom code to convert WaveDrom data to diagrams -->
<script src="https://cdnjs.cloudflare.com/ajax/libs/wavedrom/2.1.2/skins/default.js" type="text/javascript"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/wavedrom/2.1.2/skins/narrow.js" type="text/javascript"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/wavedrom/2.1.2/wavedrom.min.js" type="text/javascript"></script>
<script>
    if (document.addEventListener) {
      document.addEventListener("DOMContentLoaded", WaveDrom.ProcessAll(), false);
    }
</script>
