#include "hal/internal_flash.h"

#include "cmsis_shim.h"

#include <stddef.h>
#include <string.h>

#define NVMC_CONFIG_WEN_Pos (0UL) /*!< Position of WEN field. */
#define NVMC_CONFIG_WEN_Msk (0x3UL << NVMC_CONFIG_WEN_Pos) /*!< Bit mask of WEN field. */
#define NVMC_CONFIG_WEN_Ren (0UL) /*!< Read only access */
#define NVMC_CONFIG_WEN_Wen (1UL) /*!< Write enabled */
#define NVMC_CONFIG_WEN_Een (2UL) /*!< Erase enabled */

/* Bit 0 : NVMC is ready or busy */
#define NVMC_READY_READY_Pos (0UL) /*!< Position of READY field. */
#define NVMC_READY_READY_Msk (0x1UL << NVMC_READY_READY_Pos) /*!< Bit mask of READY field. */
#define NVMC_READY_READY_Busy (0UL) /*!< NVMC is busy (on-going write or erase operation) */
#define NVMC_READY_READY_Ready (1UL) /*!< NVMC is ready */

typedef struct {
  __IM  uint32_t  RESERVED[256];
  __IM  uint32_t  READY;
  __IM  uint32_t  RESERVED1;
  __IM  uint32_t  READYNEXT;
  __IM  uint32_t  RESERVED2[62];
  __IOM uint32_t  CONFIG;

  union {
    __IOM uint32_t ERASEPAGE;
    __IOM uint32_t ERASEPCR1;

  };
  __IOM uint32_t  ERASEALL;
  __IOM uint32_t  ERASEPCR0;

  __IOM uint32_t  ERASEUICR;

  __IOM uint32_t  ERASEPAGEPARTIAL;

  __IOM uint32_t  ERASEPAGEPARTIALCFG;
  __IM  uint32_t  RESERVED3[8];
  __IOM uint32_t  ICACHECNF;
  __IM  uint32_t  RESERVED4;
  __IOM uint32_t  IHIT;
  __IOM uint32_t  IMISS;
} sNrfNvmcConfig;

static sNrfNvmcConfig *const NRF_NVMC = ((sNrfNvmcConfig *)0x4001E000UL);

static void prv_wait_for_flash_read(void) {
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
    }
}

static void prv_write_byte(uint32_t address, uint8_t value) {
  uint32_t byte_shift = address & (uint32_t)0x03;
  uint32_t address32 = address & ~byte_shift; // Address to the word this byte is in.
  uint32_t value32 = (*(uint32_t*)address32 & ~((uint32_t)0xFF << (byte_shift << (uint32_t)3)));
  value32 = value32 + ((uint32_t)value << (byte_shift << 3));

  // Enable write.
  NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos);
  __ISB();
  __DSB();

  *(uint32_t*)address32 = value32;
  prv_wait_for_flash_read();

  NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);
  __ISB();
  __DSB();
}

void example_internal_flash_write(uint32_t addr, const void *buf, size_t length) {
  // NOTE: It would be more efficient to write out words when possible
  const uint8_t *buf_in = (const uint8_t *)buf;
  for (size_t i = 0; i < length; i++) {
    prv_write_byte(addr + i, buf_in[i]);
  }
}

void example_internal_flash_read(uint32_t addr, void *buf, size_t length) {
  memcpy(buf, (void *)addr, length);
}

void example_internal_flash_erase_sector(uint32_t addr) {
  // Enable erase.
  NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Een;
  __ISB();
  __DSB();

  // Erase the page
  NRF_NVMC->ERASEPAGE = addr;
  prv_wait_for_flash_read();

  NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
  __ISB();
  __DSB();
}
