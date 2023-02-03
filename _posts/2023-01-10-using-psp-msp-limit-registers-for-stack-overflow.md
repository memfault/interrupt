---
title: "Stack Overflows:  A thing of the past"
description: "A step by step guide on how to configure and leverage the ARMv8 MSP and PSP limit registers to protect against stack overflows, on the Renesas DA1469x. "
tag: [cortex-m]
author: Jon Kurtz
---

Stack overflows have notoriously plagued the development processes.  They often can go undetected, and can present themselves in obscure ways.  Software mechanisms have been implemented to protect against them, but these have limitations and still don't protect against all conditions.  

With the maturity of the ARM architecture, wouldn't it be better to a 'fool-proof' mechanism for detecting overflows?

<!-- excerpt start -->

In this post, we will explore using the MSP Limit and the PSP Limit Registers on the ARM Cortex-M33 architecture to detect stack overflows.

We will walk through the implementation on the Renesas DA1469x and an look at practical examples of detecting stack overlows.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Basic Terminology

The ARM Cortex-M33 introduced two new stack limit registers, PSPLIM and MSPLIM [^m33-psplim_msplim].

ARM has included this in it's ARMv8 specification, so any processors before this, will not have support.

## How does it work?
The best part about this new feature, is how easy it is to use, and how it takes the guess work out of stack overflow.  

The PSPLIM and the MSPLIM registers, just need to be set to the boundary of the stack.  If the MSP == MSPLIM register or the PSP == PSPLIM register, a UsageFault is created.  The UsageFault Status Register [^m33-usfr] contains a sticky bit in position four to indicate that a stack overflow occurred.  

With the PSP and MSP protection available, this allows us flexibility within an OS.  We can protect the MSP for exceptions and interrupts.  We can also switch out the PSPLIM value on a context switch to protect each task's stack.  If you need a refresher on context switching, check a previous post [here](2019-10-30-cortex-m-rtos-context-switching.md).

## Implementing the Limit Registers

For an implementation example on the DA1469x, you can access an example at Renesas's Github [^ARMV8_Guards].

### Implementing the MSPLIM Register

## References

[^m33-psplim_msplim]: [Cortex M33 MTB TRM](https://developer.arm.com/documentation/100231/0002/)

[^m33-usfr]: [Cortex M33 USFR](https://developer.arm.com/documentation/100235/0004/the-cortex-m33-peripherals/system-control-block/configurable-fault-status-register)

[^ARMV8_Guards]: [DA1469x Github Example](https://github.com/dialog-semiconductor/BLE_SDK10_examples/tree/main/features/armv8_stack_overflow_guards)


[^1]: [DA14695 Development Kit – USB](https://www.dialog-semiconductor.com/products/da14695-development-kit-usb)
[^2]: [DA1469x Datasheet](https://www.dialog-semiconductor.com/sites/default/files/da1469x_datasheet_3v2.pdf)
[^3]: [FreeRTOS Kernel Stack Overflow Check](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/a8a9c3ea3e34c10c6818f654883dac3dbdae12d1/tasks.c#L3052)


