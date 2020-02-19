//! @file
//! Mock implementations for sensors to simulate different types of errors

#include <stdint.h>
#include <stdbool.h>

int i2c_read_temp(uint32_t *temp) {
  // A temperature sensor that always returns an
  // error when we try to read it. :sadpanda:
  int rv = -1;
  *temp = 0;
  return rv;
}

bool spi_flash_erase_complete(void) {
  // A NOR flash chip connected over SPI which
  // never completes an erase. :saddestcat:
  return false;
}
