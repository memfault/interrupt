#include "protocol/protocol.h"
#include "protocol/registry.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

eProtocolCode protocol_handle(
    const uint8_t *buffer, size_t len,
    uint8_t *resp_buffer, size_t *resp_len) {
  // Need at least a command and size
  if (len < 8) {
    return kProtocolCode_MalformedMsg;
  }
  uint8_t *iter = (void *)buffer;

  // Parse the desired command code
  uint32_t code = *(uint32_t *)(void *)iter;
  iter += sizeof(uint32_t);
  len -= sizeof(uint32_t);

  // Parse the desired command code
  uint32_t payload_size = *(uint32_t *)(void *)iter;
  iter += sizeof(uint32_t);
  len -= sizeof(uint32_t);

  // Find the right handler
  for (const sProtocolCommand *command = g_protocol_commands; \
    command < &g_protocol_commands[g_num_protocol_commands]; \
    ++command) {
    if (code == command->code) {
      command->handler(iter, len, resp_buffer, resp_len);
      return kProtocolCode_Ok;
    }
  }

  // Failed to find a command
  return kProtocolCode_CommandNotFound;
}
