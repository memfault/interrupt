#include "memfault/panics/platform/coredump.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "memfault/core/compiler.h"
#include "memfault/core/math.h"

const sMfltCoredumpRegion *__wrap_memfault_platform_coredump_get_regions(
    const sCoredumpCrashInfo *crash_info, size_t *num_regions) {
   static sMfltCoredumpRegion s_coredump_regions[1];

   s_coredump_regions[0] = MEMFAULT_COREDUMP_MEMORY_REGION_INIT((void *)0x20000000,
      64*1024);
   *num_regions = 1;
   return &s_coredump_regions[0];
}
