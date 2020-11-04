#include <string.h>

void graphic_boot(uint16_t *graphics_buf, size_t graphics_buf_len) {
  memset(graphics_buf, 0x5a, graphics_buf_len);
}
