#include <stddef.h>
#include <stdint.h>

void kv_store_read_protocol_cmd(
    const uint8_t *buffer, size_t len,
    uint8_t *resp_buffer, size_t *resp_len);

void kv_store_write_protocol_cmd(
    const uint8_t *buffer, size_t len,
    uint8_t *resp_buffer, size_t *resp_len);
