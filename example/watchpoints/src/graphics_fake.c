#include "graphics.h"

void graphics_boot(uint16_t *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    buf[i] = 0xffee;
  }
}
