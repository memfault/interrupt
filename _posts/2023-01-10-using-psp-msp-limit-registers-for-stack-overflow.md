---
title: "Stack Overflows:  A thing of the past"
description: "A step-by-step guide on configuring and leveraging the ARMv8 MSP and PSP limit registers to protect against stack overflows on the Renesas DA1469x. "
tag: [cortex-m]
author: jonkurtz
---

Stack overflows have notoriously plagued the development processes.  They often can go undetected and can present themselves in obscure ways.  We have implemented software mechanisms to protect against them, but these have limitations and still don't protect against all conditions.  

With the maturity of the ARM architecture, wouldn't it be better to have a fool-proof mechanism for detecting overflows?

<!-- excerpt start -->

We will explore using the MSP Limit and the PSP Limit Registers on the ARM Cortex-M33 architecture to detect stack overflows.

We will walk through an implementation on the Renesas DA1469x and look at practical examples of detecting stack overflows.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Basic Terminology

The ARM Cortex-M33 introduced two new stack limit registers, PSPLIM and MSPLIM [^m33-psplim_msplim].

ARM has included this in its ARMv8 specification, so any processors before this will not have support.

## How does it work?

The best part about this new feature is how easy it is to use and how it takes the guesswork out of debugging stack overflows.  

We need to set the PSPLIM, and the MSPLIM registers to the boundary of the stack.  If the MSP == MSPLIM register or the PSP == PSPLIM register, a UsageFault is created.  The UsageFault Status Register [^m33-usfr] contains a sticky bit in position four to indicate that a stack overflow occurred.  

Having hardware protection for the PSP and MSP allows flexibility within an OS.  For example, we can protect the MSP during exceptions and interrupts.  We can also switch out the PSPLIM value on a context switch to safeguard each task's stack.  If you need a refresher on context switching, check a previous post [here](2019-10-30-cortex-m-rtos-context-switching.md).

If no RTOS is present, we can still monitor the msp, as this will protect your whole application.

## Implementing the Limit Registers

For an implementation example on the DA1469x, you can access this at Renesas's Github [^ARMV8_Guards].

### Initializing the MSPLIM Register

We use the MSR instruction to write to these registers, which requires us to be in privileged mode.  In this case, we will set the MSP Limit in the Reset_Handler:

```c
Reset_Handler:

        ldr     r0, =__StackLimit
        add     r0, r0, #dg_configMSP_PADDING
        msr     MSPLIM, r0
```

Specifically to the DA1469x, we also need to place the same initialization in the Wakeup_Reset_Handler:

```c
Wakeup_Reset_Handler:

    ldr     r0, =__StackLimit
    add     r0, r0, #dg_configMSP_PADDING
    msr     MSPLIM, r0

```
The DA1469x sleep architecture doesn't pertain to all Cortex-M33 architectures.  After initialization, the DA1469x replaces the Reset_Handler with the Wakeup_Reset_Handler to handle the restoration of Cortex-M33 registers.    

There are two definitions provided elsewhere in the project.   __StackLimit is defined in vector_table_da1469x.S:

```c
.section .stack
                .align  3
                .globl  __StackTop
                .globl  __StackLimit
__StackLimit:
                .space  Stack_Size
                .size   __StackLimit, . - __StackLimit
__StackTop:
                .size   __StackTop, . - __StackTop
```
This definition helps us find the stack limit for setting the MSP.

We also added padding to this value.  You will find this value in a configuration file:

```c
#define dg_configMSP_PADDING                    (16)
```

When the MSPLIM is equal to the MSP, the UsageFault exception is triggered.   We want to make space on the stack, so the fault handler can also push items to the stack.  If we don't make space for the fault handler, nested exceptions can occur as the MSPLIM register continuously becomes exceeded.  

The alternative would be to use a naked function[^gcc_attributes].  However, I prefer to add padding as it provides more flexibility in the fault handler and allows for Memfault hooks!
 

### Initializing the PSPLIM Register

On the DA1469x SDK, it makes use of FreeRTOS.  The psp is used for each task's stack so we can set up the PSPLIM register to protect against a task overflow.  This implementation is superior to FreeRTOS's stack overflow check[^1] for the following reasons:

1.  FreeRTOS only checks the watermark on a context switch.  Therefore, if a thread overflows the stack and isn't yielding, it can corrupt memory, access null pointers, etc. 

2.  FreeRTOS does not recommend using this feature in production environments because of the context switch overhead[^2].

Implementing this is more straightforward.  First, we must adjust the PSPLIM during a context switch in FreeRTOS.

In tasks.c we create the following above vTaskSwitchContext:

```c
void vTaskSwitchStackGuard(void)
{
        volatile uint32_t end_of_stack_val = =(uint32_t)pxCurrentTCB->pxStack;
     __set_PSPLIM( end_of_stack_val);
}
```

Next, we add the call immediately after the context switch:

```c
void xPortPendSVHandler( void ){

...

"   bl vTaskSwitchContext               \n"
"   bl vTaskSwitchStackGuard                        \n"

...
}
```

That's all we need to do for the PSP.  

### Setting up the UsageFault_Handler

All that's left is doing some work in the UsageFault_Handler.  We will declare the UsageFault_Handler in exceptions_handler.s and call a separate handler afterward.  

First, we declare an application handler:

```c
__RETAINED_CODE void UsageFault_HandlerC(uint8_t stack_pointer_mask)
{

    volatile uint16_t usage_fault_status_reg __UNUSED;

    usage_fault_status_reg = (SCB->CFSR &         SCB_CFSR_USGFAULTSR_Msk) >> SCB_CFSR_USGFAULTSR_Pos;

    hw_watchdog_freeze(); 

    if(usage_fault_status_reg & (SCB_CFSR_STKOF_Msk >> SCB_CFSR_USGFAULTSR_Pos))
    {
        
        while(1){}
    }

        while (1) {}
}

```

Next, let's add our UsageFault_Handler into the exceptions_handler.S:
the
```c
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH)
            .section text_retained
#endif
    .align  2
    .thumb
    .thumb_func
    .globl  UsageFault_Handler
    .type   UsageFault_Handler, %function
UsageFault_Handler:
    ldr     r2,=UsageFault_HandlerC
    mrs     r1, msp
    mrs     r0, MSPLIM
    cmp     r0, r1
    beq     UsageFault_with_MSP_Overflow
    mrs     r1, psp
    mrs     r0, PSPLIM
    cmp     r0, r1
    beq     UsageFault_with_PSP_Overflow
    mov     r0, #0
    bx      r2
UsageFault_with_PSP_Overflow:
    mov     r0, #2
    bx      r2
UsageFault_with_MSP_Overflow:
    ldr     r1, =__StackLimit
    msr     MSPLIM, r1
    mov     r0, #1
    bx      r2
```

Since the USFR does not indicate if the psp or the msp caused the fault, I decided to add some detection in assembly.  I prefer doing this in assembly to ensure no stack pushes before the application handler call.  

* 0 - General UsageFault 
* 1 - MSP Overflow
* 2 - PSP Overflow (Task Overflow)

In this function, we are checking the msp and the psp registers against the limit registers.  If the msp matches the msplim register, we restore the msplim to __StackLimit (Removing the padding we placed initially) and then call our application fault handler.

### Testing our Implementation

We need a small piece of code to test the implementation.  In this case, there is a macro for causing an overflow for the MSP or the PSP:

```c
#define TOGGLE_MSP_OVERFLOW (0)     //0 Creates an application overflow in FreeRTOS task, 1 creates it on the MSP
```

When the button is pressed, depending on this macro setting, it calls a recursive function either in interrupt context or in our main task.

```c
static void test_overflow_func(void)
{
        test_overflow_func();
}

static void _wkup_key_cb(void)
{
    BaseType_t need_switch; 
    /* Clear the WKUP interrupt flag!!! */
    hw_wkup_reset_interrupt();

#if TOGGLE_MSP_OVERFLOW > 0
    test_overflow_func();
#endif
        
        
    xTaskNotifyFromISR(overFlow_handle, BUTTON_PRESS_NOTIF, eSetBits, &need_switch); 
    portEND_SWITCHING_ISR(need_switch);
}

...

void prvTestOverFlowTask( void *pvParameters )
{
    _wkup_init();

    overFlow_handle = xTaskGetCurrentTaskHandle();

    for ( ;; ) {
        uint32_t notif;
        /*
        * Wait on any of the notification bits, then clear them all
        */
        xTaskNotifyWait(0, 0xFFFFFFFF, &notif, portMAX_DELAY);

        /* Notified from BLE manager? */
        if (notif & BUTTON_PRESS_NOTIF) {
                test_overflow_func();
        }
    }
}
```

After pressing the button, We should see the UsageFault_HandlerC get called in our application code. 

## Closing

The PSPLIM and the MSPLIM registers are great new features from ARM and a much-needed addition to the architecture.  We hope you found this helpful and can implement this in your application.  Implementing these features should prevent many development headaches and safeguard your application in the field!

## References

[^m33-psplim_msplim]: [Cortex M33 MSPLIM PSPLIM TRM](https://developer.arm.com/documentation/100231/0002/)

[^m33-usfr]: [Cortex M33 USFR](https://developer.arm.com/documentation/100235/0004/the-cortex-m33-peripherals/system-control-block/configurable-fault-status-register)

[^ARMV8_Guards]: [DA1469x Github Example](https://github.com/dialog-semiconductor/BLE_SDK10_examples/tree/main/features/armv8_stack_overflow_guards)

[^gcc_attributes]: [GCC Attributes](https://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Function-Attributes.html#Function-Attributes)

[^1]: [FreeRTOS Kernel Stack Overflow Check](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/a8a9c3ea3e34c10c6818f654883dac3dbdae12d1/tasks.c#L3052)

[^2]: [FreeRTOS Stack Overflow Check](https://www.freertos.org/Stacks-and-stack-overflow-checking.html)

[^3]: [DA14695 Development Kit – USB](https://www.dialog-semiconductor.com/products/da14695-development-kit-usb)

[^4]: [DA1469x Datasheet](https://www.dialog-semiconductor.com/sites/default/files/da1469x_datasheet_3v2.pdf)



