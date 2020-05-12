#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
  kProtocolCode_Ok = 0,
  kProtocolCode_MalformedMsg = 1,
  kProtocolCode_CommandNotFound = 2,

  kProtocolCode_32bit = 0x7FFFFFFF, 
} eProtocolCode;

//! Call this when a character is received. The character is processed synchronously.
eProtocolCode protocol_handle(
    const uint8_t *buffer, size_t len,
    uint8_t *resp_buffer, size_t *resp_len);

