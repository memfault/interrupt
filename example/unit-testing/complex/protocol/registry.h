#pragma once

#include <stddef.h>
#include <stdint.h>

typedef void (*ProtocolHandler)(const uint8_t *buffer, size_t len,
    uint8_t *resp_buffer, size_t *resp_len);

typedef struct ProtocolCommand {
  uint32_t code;
  ProtocolHandler handler;
} sProtocolCommand;

extern const sProtocolCommand *const g_protocol_commands;
extern const size_t g_num_protocol_commands;
