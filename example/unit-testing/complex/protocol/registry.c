#include "protocol/registry.h"
#include "kv_store/kv_store_protocol_handlers.h"

#include <stddef.h>


static const sProtocolCommand s_protocol_commands[] = {
  {1000 /* 0x03E8 */, kv_store_read_protocol_cmd},
  {1001 /* 0x03E9 */, kv_store_write_protocol_cmd},
};

const sProtocolCommand *const g_protocol_commands = s_protocol_commands;
const size_t g_num_protocol_commands = 2;
