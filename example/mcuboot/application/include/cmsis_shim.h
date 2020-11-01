#pragma once

//! @file
//! In the interest of brevity, the cmsis defines this minimal example app utilizes
//! Most vendor SDKs ship with the full CMSIS library:
//!   https://github.com/ARM-software/CMSIS_5

#define __enable_irq() __asm("cpsie i" : : : "memory")
#define __disable_irq() __asm("cpsid i" : : : "memory")
#define __IM volatile const
#define __OM volatile
#define __IOM volatile

#define __ISB(...) __asm volatile ("isb 0xF":::"memory")
#define __DSB(...) __asm volatile ("dsb 0xF":::"memory")
