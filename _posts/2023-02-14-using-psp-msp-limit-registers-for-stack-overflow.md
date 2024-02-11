---
title: "A Guide to Using ARM Stack Limit Registers"
description:
  "A step-by-step guide on configuring and leveraging the ARMv8 MSP and PSP
  limit registers to protect against stack overflows on the Renesas DA1469x. "
tags: [cortex-m, arm, stack]
author: jonkurtz
---

Stack overflows have notoriously plagued the development processes. They often
can go undetected and can present themselves in obscure ways. We have
implemented software mechanisms to protect against them, but these have
limitations and still don't protect against all conditions.

With the maturity of the ARM architecture, wouldn't it be better to have a
fool-proof mechanism for detecting overflows?

<!-- excerpt start -->

We will explore using the MSP Limit and the PSP Limit Registers on the ARM
Cortex-M33 architecture to detect stack overflows. We will walk through an
implementation on the Renesas DA1469x and look at practical examples of
detecting stack overflows. Additionally, we will look at supplementary options
for scenarios that the MSPLIM and PSPLIM features fall short.

<!-- excerpt end -->

{% include newsletter.html %}

{% include toc.html %}

## Basic Terminology

The ARM Cortex-M33 introduced two new stack limit registers, PSPLIM and MSPLIM
[^m33-psplim_msplim]. ARM has included this in its ARMv8 specification, so any
processors before this will not have support.

## How does it work?

The best part about this new feature is how easy it is to use and how it takes
the guesswork out of debugging stack overflows.

We need to set the PSPLIM, and the MSPLIM registers to the boundary of the
stack. If the MSP == MSPLIM register or the PSP == PSPLIM register, a UsageFault
is generated. The UsageFault Status Register [^m33-usfr] contains a sticky bit
in position four to indicate that a stack overflow occurred.

Having hardware protection for the PSP and MSP allows flexibility within an OS.
For example, we can protect the MSP during exceptions and interrupts. We can
also switch out the PSPLIM value on a context switch to safeguard each task's
stack. If you need a refresher on context switching, check a previous post
[here]({% post_url 2019-10-30-cortex-m-rtos-context-switching %}).

If no RTOS is present, we can still monitor the MSP, as this will protect your
whole application.

## Implementing the Limit Registers

For an implementation example on the DA1469x, you can access this at Renesas's
Github [^ARMV8_Guards].

### Initializing the MSPLIM Register

We use the MSR instruction to write to these registers, which requires us to be
in privileged mode. In this case, we will set the MSP Limit in the
Reset_Handler:

```c
Reset_Handler:

        ldr     r0, =__StackLimit
        add     r0, r0, #dg_configMSP_PADDING
        msr     MSPLIM, r0
```

Specifically to the DA1469x, we also need to place the same initialization in
the Wakeup_Reset_Handler:

```c
Wakeup_Reset_Handler:

    ldr     r0, =__StackLimit
    add     r0, r0, #dg_configMSP_PADDING
    msr     MSPLIM, r0

```

The DA1469x sleep architecture doesn't pertain to all Cortex-M33 architectures.
After initialization, the DA1469x replaces the Reset_Handler with the
Wakeup_Reset_Handler to handle the restoration of Cortex-M33 registers.

There are two definitions provided elsewhere in the project. \_\_StackLimit is
defined in vector_table_da1469x.S:

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

We also added padding to this value. You will find this value in a configuration
file:

```c
#define dg_configMSP_PADDING                    (16)
```

When the MSPLIM is equal to the MSP, the UsageFault exception is triggered. The
padding is required to enable pushing items to the stack on Exception entry. If
we don't make space for the fault handler, nested exceptions can occur as the
MSPLIM register would be continuously exceeded, usually resulting in a LOCKUP.

The alternative would be to use a naked function[^gcc_attributes]. However, I
prefer to add padding as it provides more flexibility in the fault handler and
allows for Memfault hooks!

### Initializing the PSPLIM Register

On the DA1469x SDK, it makes use of FreeRTOS. The psp is used for each task's
stack so we can set up the PSPLIM register to protect against a task overflow.
This implementation is superior to FreeRTOS's stack overflow check[^1] for the
following reasons:

1.  FreeRTOS only checks the watermark on a context switch. Therefore, if a
    thread overflows the stack and isn't yielding, it can corrupt memory, access
    null pointers, etc.

2.  FreeRTOS does not recommend using this feature in production environments
    because of the context switch overhead[^2].

Implementing this is more straightforward. First, we must adjust the PSPLIM
during a context switch in FreeRTOS.

In tasks.c we create the following above vTaskSwitchContext:

```c
void vTaskSwitchStackGuard(void)
{
    volatile uint32_t end_of_stack_val = (uint32_t)pxCurrentTCB->pxStack;
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

All that's left is doing some work in the UsageFault_Handler. We will declare
the UsageFault_Handler in exceptions_handler.s and call a separate handler
afterward.

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

Since the USFR does not indicate if the psp or the msp caused the fault, I
decided to add some detection in assembly. I prefer doing this in assembly to
ensure no stack pushes before the application handler call.

- 0 - General UsageFault
- 1 - MSP Overflow
- 2 - PSP Overflow (Task Overflow)

In this function, we are checking the MSP and the PSP registers against the
limit registers. If the MSP matches the MSPLIM register, we restore the MSPLIM
to `__StackLimit` (Removing the padding we placed initially) and then call our
application fault handler.

### Testing our Implementation

We need a small piece of code to test the implementation. In our example, there
is a macro provided for causing an overflow for the MSP or the PSP:

```c
#define TOGGLE_MSP_OVERFLOW (0)     //0 Creates an application overflow in FreeRTOS task, 1 creates it on the MSP
```

When the button is pressed, depending on this macro setting, it calls a
recursive function either in interrupt context or in our main task.

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

After pressing the button, we should see the UsageFault_HandlerC get called in
our application code.

## Limitations and Further Improvements

The MSPLIM and PSPLIM registers will help against most stack overflows.
Unfortunately, they do not protect us from local buffers corrupting the stack.
We will look at the most common; buffer overflow. A buffer overflow occurs when
a fixed buffer is allocated on the stack, and the program starts writing to
memory addresses outside this boundary. This results in corrupted data and can
even change the return address of a function, causing undesired execution of
application code.

There are different ways to handle this condition on other architectures. For
example, Zephyr uses the MPU to guard the PSP on each thread. Here, we will
discuss stack canaries.

### Stack Canary

Stack Canaries are widely implemented as a means of code hardening. A function
will place a value (canary) on the end of a stack frame and will check the value
is intact before it returns. This mechanism protects against buffer overflow
attacks, where malicious source code could overflow the buffer to redirect the
return address to its function.

This same idea can also be used to guard against buffer overflows in our
application.

### FreeRTOS Buffer Overflow protection

FreeRTOS implements a means for overflow detection, as discussed in
[Initializing the PSPLIM Register](#initializing-the-psplim-register). This uses
the concept of a canary, which will periodically check the value during a
context switch.

FreeRTOS has two different configurations that follow this concept:

```c
#if( ( configCHECK_FOR_STACK_OVERFLOW > 1 ) && ( portSTACK_GROWTH < 0 ) )

    #define taskCHECK_FOR_STACK_OVERFLOW()                                                              \
    {                                                                                                   \
        const uint32_t * const pulStack = ( uint32_t * ) pxCurrentTCB->pxStack;                         \
        const uint32_t ulCheckValue = ( uint32_t ) 0xa5a5a5a5;                                          \
                                                                                                        \
        if( ( pulStack[ 0 ] != ulCheckValue ) ||                                                \
            ( pulStack[ 1 ] != ulCheckValue ) ||                                                \
            ( pulStack[ 2 ] != ulCheckValue ) ||                                                \
            ( pulStack[ 3 ] != ulCheckValue ) )                                             \
        {                                                                                               \
            vApplicationStackOverflowHook( ( TaskHandle_t ) pxCurrentTCB, pxCurrentTCB->pcTaskName );   \
        }                                                                                               \
    }

#endif /* #if( configCHECK_FOR_STACK_OVERFLOW > 1 ) */
/*-----------------------------------------------------------*/

#if( ( configCHECK_FOR_STACK_OVERFLOW > 1 ) && ( portSTACK_GROWTH > 0 ) )

    #define taskCHECK_FOR_STACK_OVERFLOW()                                                                                              \
    {                                                                                                                                   \
    int8_t *pcEndOfStack = ( int8_t * ) pxCurrentTCB->pxEndOfStack;                                                                     \
    static const uint8_t ucExpectedStackBytes[] = { tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE,     \
                                                    tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE,     \
                                                    tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE,     \
                                                    tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE,     \
                                                    tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE, tskSTACK_FILL_BYTE };   \
                                                                                                                                        \
                                                                                                                                        \
        pcEndOfStack -= sizeof( ucExpectedStackBytes );                                                                                 \
                                                                                                                                        \
        /* Has the extremity of the task stack ever been written over? */                                                               \
        if( memcmp( ( void * ) pcEndOfStack, ( void * ) ucExpectedStackBytes, sizeof( ucExpectedStackBytes ) ) != 0 )                   \
        {                                                                                                                               \
            vApplicationStackOverflowHook( ( TaskHandle_t ) pxCurrentTCB, pxCurrentTCB->pcTaskName );                                   \
        }                                                                                                                               \
    }

#endif /* #if( configCHECK_FOR_STACK_OVERFLOW > 1 ) */
```

Both methods check for an expected value at the end of the stack. If this value
is overwritten, then vApplicationStackOverflowHook is called, and the
application should record and reset. Unfortunately, the periodicity is
non-deterministic, as it relies on a context switch. Periodic checks lead to a
race condition when a task doesn't yield in time. You can test this from the
previous example by setting the following:

```c
#define dg_configARMV8_USE_STACK_GUARDS         (0)
#define #define TOGGLE_MSP_OVERFLOW             (0)
```

In this example, prvTestOverFlowTask will not yield, so FreeRTOS does not catch
this condition.

### Compiler Enabled Overflow Detection

Compilers have started enabling SSP (Stack Smashing Protection) libraries. The
library options will allow the compiler to use canaries within function calls.
We're going to look at GCC's implementation[^5] specifically. GCC provides the
following compiler flags:

- **-fstack-protector**: This includes functions that call **alloca** and
  functions with buffers larger than or equal to 8 bytes. The guards are
  initialized when a function is entered and then checked when the function
  exits.

- **-fstack-protector-strong** - Like -fstack-protector but includes additional
  functions to be protected — those that have local array definitions, or have
  references to local frame addresses.

- **-fstack-protector-all**: all functions are protected.

- **-fstack-protector-explicit**: protects those functions which have the
  stack_protect attribute

### GCC SSP Example

Let's take a look at using the ssp library in GCC. First, let's add the compiler
flag from the previous example: -fstack-protector. The ssp library has two
externs that we define as follows.

```c
uint32_t__stack_chk_guard = 0xDEADBEEF;

void __stack_chk_fail(void) { /* will be called if guard/canary gets corrupted */

    __ASM volatile ("cpsid i" : : : "memory");

    hw_watchdog_freeze();                           // Stop WDOG
    while(1){}
}
```

Let's also add the vulnerability in our application and add it to the
prvTestOverFlowTask:

```c
__attribute__((optimize("O0"))) static uint8_t stack_buffer_test(uint16_t iters)
{
    uint8_t buffer[16];
    uint16_t i;

    for(i = 0; i < iters; i++)
    {
            buffer[i] = 0xaa;
    }

    return buffer[8];
}

```

> **NOTE:** \_\_stack_chk_guard should be randomized on startup if using ssp for
> security reasons.

This function has a fixed buffer to pass a value larger than the local buffer.
Let's add a stack_buffer_test(17) to our task and look at the assembly.

```
(gdb) disassemble stack_buffer_test
Dump of assembler code for function stack_buffer_test:
   0x0000ccc8 <+0>: push    {r7, lr}
   0x0000ccca <+2>: sub sp, #32
   0x0000cccc <+4>: add r7, sp, #0
   0x0000ccce <+6>: mov r3, r0
   0x0000ccd0 <+8>: strh    r3, [r7, #6]
   0x0000ccd2 <+10>:    ldr r3, [pc, #68]   ; (0xcd18 <stack_buffer_test+80>)
   0x0000ccd4 <+12>:    ldr r3, [r3, #0]
   0x0000ccd6 <+14>:    str r3, [r7, #28]
   0x0000ccd8 <+16>:    mov.w   r3, #0
   0x0000ccdc <+20>:    movs    r3, #0
   0x0000ccde <+22>:    strh    r3, [r7, #10]
   0x0000cce0 <+24>:    b.n 0xccf4 <stack_buffer_test+44>
   0x0000cce2 <+26>:    ldrh    r3, [r7, #10]
   0x0000cce4 <+28>:    adds    r3, #32
   0x0000cce6 <+30>:    add r3, r7
   0x0000cce8 <+32>:    movs    r2, #170    ; 0xaa
   0x0000ccea <+34>:    strb.w  r2, [r3, #-20]
   0x0000ccee <+38>:    ldrh    r3, [r7, #10]
   0x0000ccf0 <+40>:    adds    r3, #1
   0x0000ccf2 <+42>:    strh    r3, [r7, #10]
   0x0000ccf4 <+44>:    ldrh    r2, [r7, #10]
   0x0000ccf6 <+46>:    ldrh    r3, [r7, #6]
   0x0000ccf8 <+48>:    cmp r2, r3
   0x0000ccfa <+50>:    bcc.n   0xcce2 <stack_buffer_test+26>
   0x0000ccfc <+52>:    ldrb    r3, [r7, #20]
   0x0000ccfe <+54>:    ldr r2, [pc, #24]   ; (0xcd18 <stack_buffer_test+80>)
   0x0000cd00 <+56>:    ldr r1, [r2, #0]
   0x0000cd02 <+58>:    ldr r2, [r7, #28]
   0x0000cd04 <+60>:    eors    r1, r2
   0x0000cd06 <+62>:    mov.w   r2, #0
   0x0000cd0a <+66>:    beq.n   0xcd10 <stack_buffer_test+72>
   0x0000cd0c <+68>:    bl  0xcdb0 <__stack_chk_fail>
   0x0000cd10 <+72>:    mov r0, r3
   0x0000cd12 <+74>:    adds    r7, #32
   0x0000cd14 <+76>:    mov sp, r7
   0x0000cd16 <+78>:    pop {r7, pc}
   0x0000cd18 <+80>:    strh    r4, [r6, #44]   ; 0x2c
   0x0000cd1a <+82>:    movs    r0, #0

```

Here we can see the compiler loading the canary at the end of the stack frame:

```
   0x0000ccd2 <+10>:    ldr r3, [pc, #68]   ; (0xcd18 <stack_buffer_test+80>)
   0x0000ccd4 <+12>:    ldr r3, [r3, #0]
   0x0000ccd6 <+14>:    str r3, [r7, #28]

(gdb) x /1a 0xcd18
0xcd18 <stack_buffer_test+80>:  0x200085b4 <__stack_chk_guard>
(gdb) x /1a 0x200085b4
0x200085b4 <__stack_chk_guard>: 0xdeadbeef
```

Before return, we can see the function checking the canary at the end of the
stack frame and calling \_\_stack_chk_fail if the value is corrupted:

```c
0x0000cd0a <+66>:   beq.n   0xcd10 <stack_buffer_test+72>
0x0000cd0c <+68>:   bl  0xcdb0 <__stack_chk_fail>
```

Running the rest of the example should confirm the call of \_\_stack_chk_fail.

## Practical implementations for GCC stack Canaries

Implementing the ssp library does provide additional overhead in execution time
and code space. A function will add 7 additional instructions to make use of
this feature. The developer should weigh these factors when choosing which
setting to use in GCC.

My preference would be to develop and test with a stricter setting and more
coverage and move to a more relaxed setting when getting closer to production.
For example, you could start your development process with
-fstack-protector-all, and later relax this to -fstack-protector-strong or
-fstack-protector as the code matures.

## Closing

The PSPLIM and the MSPLIM registers are great new features from ARM and a
much-needed addition to the architecture. These can also be supplemented with
other techniques to fortify your application. We hope you found this helpful,
and will be inspired to make use of it in your application. Implementing these
features should prevent many development headaches and safeguard your
application in the field!

## References

[^m33-psplim_msplim]:
    [Cortex M33 MSPLIM PSPLIM TRM](https://developer.arm.com/documentation/100231/0002/)

[^m33-usfr]:
    [Cortex M33 USFR](https://developer.arm.com/documentation/100235/0004/the-cortex-m33-peripherals/system-control-block/configurable-fault-status-register)

[^ARMV8_Guards]:
    [DA1469x Github Example](https://github.com/dialog-semiconductor/BLE_SDK10_examples/tree/main/features/armv8_stack_overflow_guards)

[^gcc_attributes]:
    [GCC Attributes](https://gcc.gnu.org/onlinedocs/gcc-4.3.0/gcc/Function-Attributes.html#Function-Attributes)

[^1]:
    [FreeRTOS Kernel Stack Overflow Check](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/a8a9c3ea3e34c10c6818f654883dac3dbdae12d1/tasks.c#L3052)

[^2]:
    [FreeRTOS Stack Overflow Check](https://www.freertos.org/Stacks-and-stack-overflow-checking.html)

[^3]:
    [DA14695 Development Kit – USB](https://www.dialog-semiconductor.com/products/da14695-development-kit-usb)

[^4]:
    [DA1469x Datasheet](https://www.dialog-semiconductor.com/sites/default/files/da1469x_datasheet_3v2.pdf)

[^5]:
    [GCC Instrumentation Options](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html)
