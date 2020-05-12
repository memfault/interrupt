#include "kv_store_protocol_handlers.h"
#include "kv_store.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

void kv_store_read_protocol_cmd(
    const uint8_t *buffer, size_t len,
    uint8_t *resp_buffer, size_t *resp_len) {

  // Forgo error checking for simplicity
  const char *key = (char *)buffer;
  // Simply read the value into the resp_buffer
  bool success = false;
  int retries_left = 3;
  while (--retries_left >= 0) {
    success = kv_store_read(key, resp_buffer, (uint32_t)*resp_len, (uint32_t *)resp_len);
    if (success){
      break;
    }
  }
}

void kv_store_write_protocol_cmd(
    const uint8_t *buffer, size_t len,
    uint8_t *resp_buffer, size_t *resp_len) {

  // Buffer should be in format:
  // <key_bytes>\0<value_bytes>

  // Forgo error checking for simplicity
  const char *key = (char *)buffer;
  size_t key_len = strlen(key);

  // Compute val_ptr and val_len. 1 is the \0 char
  const uint8_t *val_ptr = buffer + key_len + 1;
  const size_t val_len = len - key_len - 1;

  kv_store_write(key, val_ptr, val_len);
  // Write an ACK of sorts
  *resp_buffer = 1;
  *resp_len = 1;
}
