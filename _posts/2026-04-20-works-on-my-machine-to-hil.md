---
title: "From \"Works on My Machine\" to Hardware-in-the-Loop"
description: >
  Most embedded teams start testing firmware over USB with FTDI or similar
  bridges. That works on the bench. This post shows what breaks the moment
  you move to CI, and walks through building a DIY hardware-in-the-loop rig
  from scratch — power control, netboot, OpenOCD, a signal router MCU, and
  the udev glue that holds it together.
author: edwardv
tags: [ci, testing, hardware-in-the-loop, openocd, raspberry-pi]
---

> **TL;DR:** This post shows how to build a DIY hardware-in-the-loop rig
> around a Raspberry Pi, a UART hub, netboot for Linux boards, OpenOCD for
> MCUs, and a small MCU as a deterministic signal router. It's the prototype
> I used before moving to custom PCBs. Good enough on its own for a few
> devices if you don't mind the cable mess, and a good way to learn exactly
> what breaks before going to a more solidified setup.

Bridging UART and other protocols over USB is great, right up until you try
to run it in CI.

On your desk, everything works. Board plugged into your laptop over USB, test
script runs, firmware passes. You ship it. Then someone else on the team pulls
the branch, runs the same test, and it fails... or worse, it passes, and the
bug only shows up a week later in the field. The classic "works on my machine,"
but now for hardware.

<!-- excerpt start -->

That's where things start breaking in ways that are hard to debug. The usual
shortcuts all fail the same way: USB isn't deterministic, mocks replace real
peripherals with assumptions, and QEMU runs a model of the CPU, not your board.
Each one works until the bug you're chasing lives in the thing you abstracted
away.

The next step up is hardware-in-the-loop: a dedicated test adapter that sits
between your CI pipeline and your DUT and handles the full signal layer
properly. This article walks through the DIY version end to end — power
control, console access, DUT boot, protocol mastering, state observation,
orchestration, and CI integration. Full hardware list is at the end of the
article.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Power

The first problem to solve is power control — the ability to hard-reset your
DUT from software without touching it. You have three options: Power over
Ethernet, USB power, or direct DC.

PoE is clean if you already have PoE infrastructure in place and your board
has an ethernet port. The DUT gets power and network over one cable, and
software control is a matter of an SNMP command to the switch. If you're
starting from scratch, though, the PoE-capable switch and splitter costs add
up fast across a rack of devices, so the build-cost-to-capability ratio isn't
great.

USB power is convenient, but in practice it can be unreliable. "Off" doesn't
always mean fully off, and some switches still leak current. Splitting power
and data is a more robust approach, and it better reflects real-world
conditions. It also solves another limitation: some targets simply need more
power than USB can provide, since you can't draw unlimited current from a USB
port or hub.

Powering through a switched rail is the easiest path. A 4 or 8 port relay
board works as a starting point. I used an SRD-05VDC-SL-C relay board.
There's no overcurrent protection, but the hardware is cheap and the control
model is trivial. The control header needs a 5V supply and a GPIO-driven
signal input. You tie the supply pin to the Pi's 5V rail and switch the
coil's control input from a GPIO. Pull the GPIO high, the relay closes, the
DUT gets power. Pull it low, the DUT goes dark. That's a hard reset you can
trust. You can buy barrel jack and USB male connectors with a terminal block
which allows you to easily wire power from the relay to the target.

> **Note:** The relay only switches the positive supply rail (VCC) of your
> target. The ground (GND) remains permanently connected and should not be
> switched.

<figure>
  <img src="{% img_url works-on-my-machine-to-hil/power-uart-management-node.jpg %}" />
  <figcaption>From top to bottom: management node (Raspberry Pi 4), UART hub, 4-port relay, and a Pico and Pico 2</figcaption>
</figure>

## Connectivity

The management node is the brain of the whole setup. A Raspberry Pi running
Linux is a good choice: cheap, well-supported, easy to script against. The
first thing to wire up is UART from each DUT. Serial console access is how
you monitor boot output, capture test results, and interact with a shell if
your firmware exposes one.

If you're running more than a handful of devices you'll hit the Pi's UART
limit fast. The solution is a multi-channel USB UART adapter — the CH348 is
a good pick, exposing 4 or 8 independent serial ports over a single USB
connection. Each port shows up as its own `ttyUSB` device on the management
node, and if you wire the adapter out to JST connectors you get clean, keyed
cabling to each DUT instead of a tangle of jumper wires. One USB cable into
the Pi, eight independent serial connections out to a shelf of boards. I have
to admit though, that even with JST my wiring of this prototype always looked
like a mess.

## Booting the DUT

Once you have power and console, you need a way to get code running on the
DUT. The approach depends on the target class.

For Linux devices that have ethernet or WiFi, netboot is the path of least
resistance. The management node serves DHCP, hands the DUT a TFTP path for
the kernel, and exports a root filesystem over NFS. The DUT comes up with a
fresh, known-good image every reset, and you swap kernels by dropping a new
file on the management node. No SD card juggling.

<figure>
  <img src="{% img_url works-on-my-machine-to-hil/netboot.jpg %}" />
  <figcaption>booting using netboot</figcaption>
</figure>

> **Warning:** Do not run this on your home or office network. A test rig
> serving DHCP will happily hand out leases to everything in the broadcast
> domain. Put the test network behind a VLAN-capable switch and isolate it.
> The management node gets a leg in both VLANs: one to the lab network and
> one to the uplink for internet access.

For microcontroller targets, the equivalent workflow is OpenOCD over a debug
probe. You don't need to buy anything fancy — any Raspberry Pi Pico or Pico 2
can be flashed as a CMSIS-DAP debug probe, or you can buy the official
[Raspberry Pi Debug Probe](https://www.raspberrypi.com/products/debug-probe/)
to have the cables included. From the management node you script `openocd` to
halt the target, flash a fresh firmware image, and release reset. It's the
same idea as netboot: every test run starts from a clean, known image.

Here's what that looks like in practice for an STM32F4 target, driven by a
Pi-hosted Pico debug probe:

```bash
openocd \
  -f interface/cmsis-dap.cfg \
  -c "transport select swd" \
  -f target/stm32f4x.cfg \
  -c "init" \
  -c "reset halt" \
  -c "program {firmware.elf} verify" \
  -c "reset run" \
  -c "shutdown"
```

The sequence matters. Halt the target before flashing, verify the image after
writing, then release reset and let it run. That gives you a clean slate on
every test run. If you have multiple dongles connected, you can use
`cmsis-dap vid_pid 0x...` to target a specific adapter.

**Don't skip the reset line.** The Pico debug probe only wires SWDIO, SWDCLK,
and GND by default. OpenOCD will happily halt and flash a healthy target over
SWD alone — but at some point your target won't be healthy (broken firmware,
failed test leaving it wedged), and `reset halt` will time out with no way to
recover remotely. Wire a GPIO to the DUT's NRST pin and use it to reset the
device before OpenOCD runs. You'll need it sooner rather than later, especially
if there's no button on the target board.

One more practical issue: the CH348 UART hub disconnects during OpenOCD
flashing, even when you target the debug probe precisely by USB hardware ID.
The exact cause isn't clear to me — likely USB bus reset propagation during
the flash sequence — and I haven't found a fix yet. I added retry logic in
my scripts. That also means you'll want stable device names. Assign persistent
names to each UART port by hardware ID using udev, and the hub
re-enumerating doesn't scramble your port assignments. The serial port for
DUT 3 is still the port for DUT 3 after the reconnect.

Here are the udev rules I've been using for 8 stable ports. Adjust the path
fragments to match your hub's actual topology (`udevadm info -a /dev/ttyUSBx`
shows you the values).

`/etc/udev/rules.d/99-ch9344-persistent-tty.rules`:

```
ACTION=="add|change", SUBSYSTEM=="tty", KERNEL=="ttyCH9344USB[0-9]*", \
  ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="55d9", \
  PROGRAM=="/usr/local/sbin/ch9344-tty-alias.sh %m 8 ttyCH9344PERSIST", \
  SYMLINK+="%c", GROUP="dialout", MODE="0660"
```

`/usr/local/sbin/ch9344-tty-alias.sh`:

```sh
#!/bin/sh
set -eu
minor="${1:-}"
count="${2:-}"
prefix="${3:-}"
case "$minor" in
  ''|*[!0-9]*) exit 1 ;;
esac
case "$count" in
  ''|*[!0-9]*) exit 1 ;;
esac
if [ "$count" -le 0 ]; then
  exit 1
fi
idx=$((minor % count))
printf '%s%s\n' "$prefix" "$idx"
```

## The Signal Router

With power, console, and firmware provisioning in place, the next layer is
protocols — SPI, I2C, and peripheral UARTs between the test infrastructure
and the DUT's interfaces. This is where a Raspberry Pi alone falls down.
Linux can drive these protocols, and with hardware SPI it can push data in
the low tens of MHz, but it cannot do so with cycle-accurate timing. Once you
need precise timing, fast turnarounds, or multiple buses running in
parallel — especially SPI in the 10–20+ MHz range — scheduler jitter starts
showing up as flaky tests.

This is usually the point where setups stall. Everything works on the bench,
but the moment you try to automate it, small issues start compounding.

The fix is to put an MCU between the Pi and the DUT. Any MCU you're already
comfortable with works: STM32, ESP32, RP2040, Pico 2, whatever's in your
parts drawer. The MCU is deterministic in a way a Pi isn't: when your test
script tells it to send a 16-byte SPI transaction at 4 MHz with a specific
gap between bytes, that's exactly what comes out of the pins. RP2040/2350
boards like the Pico and Pico 2 are especially attractive here because PIO
lets you implement precise, custom bus behavior in hardware-assisted state
machines. In essence, the management node orchestrates, and the MCU executes.

As a first prototype, wiring is just protocol-to-protocol matchmaking. The
MCU's I2C pins go to the DUT's I2C pins, SPI to SPI, UART to UART. A
breadboard handles this fine at this stage — you're not chasing signal
integrity yet, just validating that the architecture works. Interestingly, if
you want to add CAN and your board doesn't support it, you can drop a
CAN-to-SPI chip on the breadboard to start testing CAN capabilities in the
DUT.

<figure>
  <img src="{% img_url works-on-my-machine-to-hil/i2c-breadboard.jpg %}" />
  <figcaption>I2C breadboard with pull-ups and logic analyzer connected</figcaption>
</figure>


Everyone has at some point lost too much time to little mistakes that creep
into these setups. When building out I2C I made some obvious ones: picking
the wrong pin, missing a pull-up. A logic analyzer on the bus tells you in
ten seconds whether the signal is even leaving the MCU. That's why it goes on
the breadboard from the beginning, not after you've already spent hours
convinced your firmware is broken. Also: if your $5 logic analyzer starts
misbehaving, try replacing the USB cable before anything else. Speaking from
experience.

<figure>
  <img src="{% img_url works-on-my-machine-to-hil/logic-analyzer-screenshot.png %}" />
  <figcaption>Logic analyzer showing I2C</figcaption>
</figure>

A quick note on voltage levels: this article assumes a 3.3V DUT, which
matches a Pico or most modern MCU dev boards. If your DUT runs at 1.8V or
5V, you will need to wire in level shifters on every signal line.

## Simulating the Real World on the Breadboard

Populate the breadboard with the peripherals your DUT actually expects to
talk to in the field. If the firmware reads a temperature sensor on boot, put
a real one on the breadboard. If it expects an EEPROM at a specific I2C
address, wire one up. The closer the test rig looks to a real deployment, the
more your CI catches real bugs instead of fake ones that only happen on the
bench.

## Reading DUT State

Beyond active protocol transactions, you often need to passively read DUT
state: are GPIOs in the right configuration, did a peripheral initialize
correctly, is a status line asserted. The textbook answer is to bolt an I2C
GPIO expander onto the management node and poll it.

If the signal router MCU is a Pico or Pico 2, there's another option: PIO.
It's a bank of state machines on the chip that can sample a set of pins on
every clock edge — far faster and more precise than polling an expander. The
catch is that PIO takes learning. We started with an expander but quickly
moved to wiring MCU pins directly. The RP2350B has 48 GPIOs available for
PIO, with some limitations: each PIO block sees a 32-pin window of the GPIO
space, so covering all 48 pins means splitting work across multiple PIO
blocks.

## Test Orchestration

With the hardware up, the next question is what to actually test.

The easiest test is to trigger a real event through a real peripheral and
watch what the firmware does. In software terms this is your first
end-to-end test — not a unit test of an isolated function, but a full-stack
exercise of the system behaving as it would in the field.

UART is the lowest-friction starting point. The console is already wired.
Open the port, send a command, check the response:

```python
import time
import serial

def run_start_test(port="/dev/ttyUSB0", baudrate=115200, timeout=5):
    with serial.Serial(port, baudrate=baudrate, timeout=0.1) as ser:
        ser.reset_input_buffer()
        ser.write(b"START\n")
        ser.flush()

        deadline = time.time() + timeout
        while time.time() < deadline:
            line = ser.readline().decode("utf-8", errors="replace").strip()
            if not line:
                continue

            print(f"DUT: {line}")

            if "OK" in line:
                return True
            if "ERROR" in line:
                return False

    raise TimeoutError("DUT did not respond within timeout")


if __name__ == "__main__":
    passed = run_start_test()
    print("PASS" if passed else "FAIL")
```

As the rig matures, you wire real peripherals into the test and drive them
from the signal router MCU to simulate real-world sequences: a button held
for 500ms, a sensor reading crossing a threshold, a communication bus going
silent for three seconds. These conditions are hard to reproduce reliably on
a bench and nearly impossible to run in simulation. With a programmed MCU,
they become repeatable test cases you can run on every commit.

This is fundamentally different from mocking. Mocks test whether your code
handles a faked abstraction correctly. HIL tests whether the system behaves
correctly when actual hardware does what it does in the real world. Both
matter, but HIL catches the bugs that only appear when real peripherals are
involved: timing dependencies, electrical edge cases, boot-order sensitivity.
Those tend to be exactly the bugs that slip through to production.

A word of caution: deciding how tests are scheduled, who owns which DUT, how
results get collected, how failures trigger retries versus alerts — that's a
whole project in itself. Treat it separately from building the hardware. The
hardware is only part of the story, and orchestration needs an equally big
time investment.

## CI Integration

Once tests pass locally, they should ideally run automatically.

The simplest option is to run a self-hosted CI runner on the management node.
GitHub Actions, GitLab Runner, and most other CI platforms support this:
install a small agent on the Pi, register it with the CI provider, and let it
poll for jobs. A push triggers a workflow that flashes a new image, runs the
test suite, and reports results back — no extra cloud infrastructure, no
inbound firewall rules.

## What's Next

At some point we weren't testing firmware anymore. We were testing our test
setup.

The prototype is great and you can build it in a few days. But as firmware
complexity grows, you'll outgrow it. What you might encounter:

- No electrical protection if you're using relays
- No real insight into the power consumption of the boards
- Each target device needs different wiring
- No bus contention protection (both sides can drive the same line at once —
  one high, one low — which can damage your boards)
- Doesn't scale well to many boards with many users

The next obvious iteration is a PCB power board and signal router. That's
exactly what we did, but it comes with its own challenges. Hardware iteration
is slow and expensive in a way software isn't. A PCB issue is a two-week
turnaround and a JLC/PCBWay invoice. The breadboard prototype exists to
de-risk the PCB, and it gave us this story to tell.

<figure>
  <img src="{% img_url works-on-my-machine-to-hil/pcb-power-board-prototype.jpg %}" />
  <figcaption>A PCB prototype of a 4-port eFuse power distributor. Newer versions have an onboard MCU.</figcaption>
</figure>

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## Hardware List

| Component | Part / Model | Notes |
|---|---|---|
| Management node | Raspberry Pi 4 (4GB) | Any Linux SBC works |
| Relay | SRD-05VDC-SL-C (4 or 8 port) | 5V coil, GPIO-controlled |
| USB UART hub | CH348 | 4 or 8 port; JST connectors are handy |
| Signal router MCU | Any MCU you're familiar with | STM32, ESP32, Pico / Pico 2 all work |
| Debug probe | Pico flashed as CMSIS-DAP | Doesn't need to be anything fancy |
| Logic analyzer | Generic works fine | If you buy a $5 one, you may need to replace the USB cable |
| Pull-up resistors | 4.7kΩ | For I2C lines |
| VLAN-capable switch | Any managed switch | Required for netboot isolation; management node goes in both VLANs |
| Peripherals / sensors | Project-specific | Match what your DUT expects in the field |
