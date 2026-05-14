---
title: Understanding Flash Wear On NAND Based Embedded Devices
description:
  ""
author: blake
tags: [flash, storage, embedded]
---

<!-- excerpt start -->
NAND based flash has become ubiquitous across all of computing. From server grade nVMEs down to our humble embedded
eMMC, it's almost guaranteed that you're interacting with this technology on a daily basis. Despite it's ubiquity it
has one large drawback. Over time as we write to the storage it degrades and eventually fails. This is manageable in
a data center where a bad drive could be swapped out, but catastrophic in an embedded device where it often cannot.
In this article we'll get a high level understanding of how NAND flash works, strategies to slow down the wear, and
how we can monitor it to ensure we aren't rapidly consume its lifetime.
<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## What is Flash Wear

At a very basic level we can define flash wear as the slow degradation of the ability to write to the flash, and
eventually its ability to retain data. The exact mechanism of this can vary across technologies, but it is typically
caused by repeated erase/write cycles. Reads can have some effect, but are generally negligible compared to damage
caused by erases and writes.

Since there are a number of flash technologies that all work in slightly different ways we're going to reduce scope
a bit. For the remainder of the article "Flash" will be referring to NAND based flash, typically eMMC.

## How Flash Memory Works

NAND based flash has a few levels of organization at a logical level. Pages represent the smallest unit that can be written
to. These are typically 4-8kB, but there are some higher density versions that can be up to 64KB. This effectively means that
if you want to write data to disk at a minimum you are doing a write of that size. So if you needed to modify 1kB of a kB page,
you would need to perform a write on the entire page.

The next level of organization is the block. A block is a collection of pages. Again this will vary based on the flash in
question, but there will typically be ~64-512 pages per block. The block adds another interesting wrinkle to our write path
as this is the minimum sized unit that you can erase. This is a wrinkle because programming a bit in flash means moving it from
`1` to `0`. The only way to go back to the `1` state is to erase the entire block. Putting these two facts together means that
when writing any amount of data to a block it must first be erased, even if that change would only affect a single page.

TODO: picture of page/block organization

This means that for the majority writes, regardless of size, we'll be erasing and writing an entire block! Now there are some
edge cases where this isn't always true. We'll get into this when we talk a bit more about wear levelling, but overall it means
that the number of bytes we're actually writing to disk is some multiple of the actual bytes that were requested. This is referred
to as "Write Amplification Factor" (WAF), and is just a ratio of the average number of bytes written to a disk when a single is
requested. We'll talk about strategies to reduce this later, but it is typically highly tied to the workload. Sequential writes
can be buffered to keep wear in the 1-2x range, whereas a large number of random small writes could lead to something on the scale
of 10x!

### Managed NAND (eMMC)

The most common form of flash we interact with on embedded Linux is managed NAND or eMMC. These are storage devices that follow
the JESD84[^jesd84] standard are typically comprised of two parts, NAND storage and a controller. The controller is responsible
for handling wear leveling (which we'll cover in a moment), and providing a standardized interface for drivers to interact with.
This is done by a series of registers defined in the previously mentioned JESD84[^jesd84] standard. These registers also expose
some interesting diagnositc data that we'll look at later.

TODO: Picture of controller and NAND

### Physics of Flash Wear

We don't really need to understand the physics to get a practical handle on flash wear, but it's very interesting! If you don't
want to suffer through a layman attempting to (poorly) explain some cool quantum mechanics interactions feel fre to skip to the
next section. Otherwise let's dig in!

#### Physical Structure

One part of NAND organization that we didn't cover previously is the cell. A cell is actually a single transistor. These transistors
can represent anywhere from 1-4 bits (and even more on more exotic technology!). These different configurations all have their own
names. While having more bits per cell does increase overall storage volume, it does have some tradeoffs in speed and endurance
that we'll cover in a moment. Here are the most common types you're likely to encounter on your travels.

| Type | Full Name | Bits per Cell |
| ------ | ----------- | --------------- |
| SLC | Single Level Cell | 1 |
| MLC | Multi Level Cell | 2 |
| TLC | Triple Level Cell | 3 |
| QLC | Quad Level Cell | 4 |

It's worth noting that MLC is poorly named. It was meant to differentiate from SLC, but the existence of newer technologies makes
it a bit unclear. It is commonly understood now the MLC means 2 bits per cell. My petition to rename it to DLC is still ongoing.

MLC and TLC are the most common types in the embedded world, with TLC becoming much more common in recent years. SLC is most
likely seen in places where you can afford to spend more money per byte to ensure longer life. You'd typically see these in
intensive industrial applications.

These transistors are a bit different from a traditional MOSFET. Between the traditional control gate and the source-drain channel
there is another "floating" gate that is electrically isolated on both sides by some thin oxide layers. The floating gate is
where the electrons storing our data are stored and gives these transistors their floating gate (FGMOSFET) name.

TODO: Picture fo floating gate MOSFET

#### Physical Anatomy of Program/Erase

We now have an understanding of how the NAND is constructed, but what does it mean to actually store a value in one of our cells?
Let's look at an SLC cell to understand the simplest case first. In SLC each cell stores 1 bit. That means we're in one of two states,
erased or not. But how is this actually encoded? Recall our floating gate from before. This acts as our storage. Electrons are essentially
trapped inside this layer. When we apply the reference voltage at the control gate, the electrons trapped in the floating gate oppose the
electric field of the control gate. Effectively this increases the voltage needed to create the inversion layer in the channel. This means
that different voltage ranges map to our different states. For SLC it would look something like this:

| State | Electrons on Floating Gate | Threshold Voltage (Vt) | Bit Value |
|-------|---------------------------|------------------------|-----------|
| Erased | None | ~-3V to -1V | `1` |
| Programmed | Many | ~1V to 3V | `0` |

And for MLC:

| State | Electrons on Floating Gate | Threshold Voltage (Vt) | Bit Value |
|-------|---------------------------|------------------------|-----------|
| L0 (Erased) | None | ~-3V to -1V | `11` |
| L1 | Some | ~-0.5V to 0.5V | `10` |
| L2 | More | ~1V to 2V | `01` |
| L3 | Most | ~2.5V to 3.5V | `00` |

Note that these are just values for reference. Actual values are likely different, but the concept remains the same. Different voltage bands
correspond to on/off states read from the cell.

#### Fowler Nordheim Tunneling

You may have noticed something odd in a previous sentence. We said that electrons get stuck in the floating gate, but there is a
an oxide barrier in the way. How do they get through? This is where we step into some fairly magical physics. The exact details
of this are a bit out of scope for this article. If you'd like an in-depth but approachable history of the science and the modern
applications I'd suggest giving Andrew Walker's article[^fn_tunneling_medium] a read. It's a great read!

The short and (very) simplified explanation is that applying large electric field across the oxide shrinks the effective width of
the barrier. This gives electrons in the channel a higher probability of moving through the tunnel oxide and into the floating gate.
This corresponds to our write. Reversing the field forces the electrons back across the barrier and empties the floating gate. This
is our erased state. More or less electrons trapped in the gate through this process puts us in the other voltage bands needed
for the multi bit flash types.

Without an applied field the energy barrier is tall and wide, giving electrons almost no probability of crossing:

```text
TODO: replace with diagram
Energy
  |
  |    [oxide barrier]
  |    _______________
  |   |               |
  |   |               |
e-|-->|               |
  |   |               |
  |___|_______________|___
       tunnel oxide
```

With a strong electric field applied, the energy landscape tilts. The electron only needs to tunnel through the narrow triangular
tip before reaching an allowed region a much shorter distance, and therefore a much higher tunneling probability:

```text
TODO: replace with diagram
Energy
  |
  |    [oxide barrier]
  |    /|
  |   / |
  |  /  |
e-|->   |
  |      \
  |_______\___
       tunnel oxide
```

It's important to note that this is just referring to the energy profile of the barrier. It does not physically deform.

If you want to get a more mathy and in-depth understanding, I'd recommend the wiki[^fn_wiki] as a good place to start.

A consequence of the above diagrams is that there is a small chance that electrons damage the oxide barrier by tunneling
into it directly. This leads to an increase in control gate voltage needed to open the channel, as electrons stuck in
the oxide layer oppose thecontrol voltage being applied. This leads to potentially reading the voltage as in the wrong
band, and thus the wrong bits are read[^nand_wearout].

## Wear Leveling

With the above info in mind we can establish that there are a certain number of program and erase operations we can
perform on each block. In fact the term for this is program/erase (P/E) cycles. It is a rating provided by the
manufacturer that gives an estimate of how many cycles each block can withstand before failing. Obviously this
can't be an exact science as we now know that the physical process by which the block degrades is entirely
probabilistic. It does however give a good frame of reference, and a good target to plan around if you want
to avoid failures.

You may be starting to wonder about write patterns to say a single file. If there is a single file that is
constantly written to, won't those underlying blocks get worn out faster? The answer to this is yes! That's
where wear leveling comes in. Through a few approaches we can mitigate issues cause by this common pattern
by making a mapping of the logical blocks backing the file to the physical blocks[^ftl_codecapsule] on the
NAND itself. By providing this abstraction the same logical block behind a file can get moved around to
share the load between nearby blocks.

TODO: add share the load gif

The Flash Translation Layer (FTL) on the eMMC controller is responsible for maintaining these mappings.
While this is mostly opaque from the host side, there are a few common strategies

### Dynamic Wear Leveling

Dynamic wear leveling[^wear_leveling_kioxia] is simply taking blocks that change often and cycling them through free blocks. It
compares available blocks and finds the ones with the lowest P/E counts to move the data to on the next
write. This works well if all files on your device are constantly written to, but in practice we know
this isn't true. Many files can sit untouched for the entire life time of a device. Think of a static
config file. This is where static wear leveling comes in.

TODO: Add dynamic wear leveling gif

### Static Wear Leveling

Static wear leveling accounts for the above problem by setting a threshold where differences between
the highest and lowest erase count blocks is breached. This triggers a migration of less worn blocks
over to the more worn areas. This ensures that blocks do not fail when there is capacity on others,
but it does come with some write amplification. Because you're migrating blocks over each of those
is another write. The tuning of this algorithm typically involves deciding how aggressive you want
to be at that level

Most FTL implementations implement a hybrid approach getting the best of both.

TODO: Add hybrid wear leveling gif.

## Detecting and Monitoring Wear

### Understanding TBW

### Tracking Bytes Written Per Device

### Attributing Individual Processes

## Practical Recommendations

## Conclusion

<!-- Interrupt Keep START -->
{% include newsletter.html %}

{% include submit-pr.html %}
<!-- Interrupt Keep END -->

{:.no_toc}

## References

<!-- prettier-ignore-start -->
[^nand_101]: [Micron TN-29-19: NAND Flash 101 Introduction](https://user.eng.umd.edu/~blj/CS-590.26/micron-tn2919.pdf)
[^nor_nand_guide]: [Micron NOR/NAND Flash Guide](https://assets.micron.com/adobe/assets/urn:aaid:aem:e98bc653-e42d-45a7-9b8c-03cb3d488f05/renditions/original/as/nor-nand-flash-guide.pdf)
[^ftl_survey]: [Chung et al. — "A Survey of Flash Translation Layer" (2009)](https://www.sciencedirect.com/science/article/abs/pii/S1383762109000356)
[^jesd84]: [JEDEC JESD84-B51 eMMC 5.1 Specification](https://www.jedec.org/standards-documents/docs/jesd84-b51)
[^cnx_wear]: [CNX Software — Wear Estimation for eMMC Flash](https://www.cnx-software.com/2019/08/16/wear-estimation-emmc-flash-memory/)
[^emmc_health]: [Checking eMMC Health on Linux](https://www.toomanyatoms.com/computer/eMMC_health.html)
[^proc_io]: [proc_pid_io(5) — Linux man page](https://man7.org/linux/man-pages/man5/proc_pid_io.5.html)
[^proc_fs]: [Linux Kernel docs — /proc Filesystem](https://docs.kernel.org/filesystems/proc.html)
[^flash_reliability]: [Schroeder et al. — "Flash Reliability in Production: The Expected and the Unexpected" FAST '16](https://www.usenix.org/system/files/conference/fast16/fast16-papers-schroeder.pdf)
[^fn_tunneling_3d]: [The Memory Guy — "How Do You Erase and Program 3D NAND?"](https://thememoryguy.com/how-do-you-erase-and-program-3d-nand/)
[^fn_tunneling_medium]: [Andrew Walker — "The Triumph of Quantum Mechanics at the Heart of Solid State Data Storage"](https://medium.com/@AndrewJanWalker/the-triumph-of-quantum-mechanics-at-the-heart-of-solid-state-data-storage-baa0c7b5a2ca)
[^nand_wearout]: [TechTarget — "What is NAND Flash Wear-Out?"](https://www.techtarget.com/searchstorage/definition/NAND-flash-wear-out)
[^fn_wiki]: [Fowler Nordheim Tunneling](https://en.wikipedia.org/wiki/Field_electron_emission#Fowler%E2%80%93Nordheim_tunneling)
[^wear_leveling_macronix]: [Macronix AN0289 — Wear Leveling in NAND Flash Memory](https://www.macronix.com/Lists/ApplicationNote/Attachments/1913/AN0289V1-Wear%20Leveling%20in%20NAND%20Flash%20Memory.pdf)
[^wear_leveling_kioxia]: [Kioxia Tech Brief — Understanding Wear Leveling in NAND Flash](https://americas.kioxia.com/content/dam/kioxia/en-us/business/memory/mlc-nand/asset/KIOXIA_Managed_Flash_BOS_P3_Understanding_Wear_Leveling_Tech_Brief.pdf)
[^ftl_flashdba]: [flashdba.com — Understanding Flash: The Flash Translation Layer](https://flashdba.com/2014/09/17/understanding-flash-the-flash-translation-layer/)
[^ftl_codecapsule]: [Code Capsule — Coding for SSDs Part 3: Pages, Blocks, and the FTL](https://codecapsule.com/2014/02/12/coding-for-ssds-part-3-pages-blocks-and-the-flash-translation-layer/)
<!-- prettier-ignore-end -->
