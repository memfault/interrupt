#pragma once

//! @file
//! In the interest of brevity, the cmsis defines this minimal example app utilizes
//! Most vendor SDKs ship with the full CMSIS library:
//!   https://github.com/ARM-software/CMSIS_5

#define __IM volatile const
#define __OM volatile
#define __IOM volatile

#define __enable_irq() __asm("cpsie i" : : : "memory")
#define __disable_irq() __asm("cpsid i" : : : "memory")
#define __IM volatile const
#define __OM volatile
#define __IOM volatile

#if defined(__GNUC__) || defined(__clang__) || defined(__ICCARM__)
#define __ISB(...) __asm volatile ("isb 0xF":::"memory")
#define __DSB(...) __asm volatile ("dsb 0xF":::"memory")


#elif defined(__TI_ARM__)

#define __ISB(...) __asm(" isb")
#define __DSB(...) __asm(" dsb")

#elif defined(__CC_ARM)

#define __ISB() do {                            \
    __schedule_barrier();                       \
    __isb(0xF);                                 \
    __schedule_barrier();                       \
  } while (0)

#define __DSB() do {                            \
    __schedule_barrier();                       \
    __dsb(0xF);                                 \
    __schedule_barrier();                       \
  } while (0)

#else
#error "Not supported isb/dsb port"
#endif
