#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C" {
  #include <string.h>
  #include <stddef.h>

  #include "kv_store/kv_store_protocol_handlers.h"
  #include "kv_store/kv_store.h"
}


static uint8_t s_resp_buffer[1024];
static const size_t s_resp_buffer_len = sizeof(s_resp_buffer);

TEST_GROUP(TestKvStoreProtocolHandlers) {
  void setup() {
  }

  void teardown() {
    mock().clear();
  }
};

bool kv_store_write(
    const char *key, 
    const void *val, 
    uint32_t len) {
  
  return mock()
      .actualCall(__func__)
      .withStringParameter("key", key)
      .withMemoryBufferParameter("val", (const unsigned char *)val, len)
      .withIntParameter("len", len)
      .returnBoolValueOrDefault(true);
}

bool kv_store_read(
    const char *key, 
    void *buf, 
    uint32_t buf_len, 
    uint32_t *len_read) {
  
  return mock()
      .actualCall(__func__)
      .withStringParameter("key", key)
      .withOutputParameter("buf", buf)
      .withOutputParameter("len_read", len_read)
      .returnBoolValueOrDefault(true);
}


TEST(TestKvStoreProtocolHandlers, Write) {
  // Key: "hello"
  // Val: "world"
  const uint8_t in_bytes[] = {
      'h', 'e', 'l', 'l', 'o', '\0', // Key
      'w', 'o', 'r', 'l', 'd'        // Value
  };

  const uint8_t kv_val_bytes[] = {
      'w', 'o', 'r', 'l', 'd'
  };

  const uint8_t out_bytes[] = {
    1
  };

  mock()
      .expectOneCall("kv_store_write")
      .withStringParameter("key", "hello")
      .withMemoryBufferParameter("val", 
                                 kv_val_bytes, 
                                 sizeof(kv_val_bytes))
      .withIntParameter("len", sizeof(kv_val_bytes));

  size_t len = s_resp_buffer_len;
  kv_store_write_protocol_cmd(
      in_bytes, sizeof(in_bytes), 
      s_resp_buffer, &len);

  mock().checkExpectations();
  MEMCMP_EQUAL(out_bytes, s_resp_buffer, sizeof(out_bytes));
}

TEST(TestKvStoreProtocolHandlers, Read) {
  // Key: "hello"
  const uint8_t in_bytes[] = {
      'h', 'e', 'l', 'l', 'o', '\0' // Key
  };

  const uint8_t out_bytes[] = {
      'w', 'o', 'r', 'l', 'd'
  };
  const uint32_t out_len = sizeof(out_bytes);

  mock()
      .expectNCalls(2, "kv_store_read")
      .withStringParameter("key", "hello")
      .withOutputParameterReturning("buf", out_bytes, sizeof(out_bytes))
      .withOutputParameterReturning("len_read", &out_len, sizeof(out_len))
      .andReturnValue(false);

  mock()
      .expectOneCall("kv_store_read")
      .withStringParameter("key", "hello")
      .withOutputParameterReturning("buf", out_bytes, sizeof(out_bytes))
      .withOutputParameterReturning("len_read", &out_len, sizeof(out_len))
      .andReturnValue(true);

  size_t len = s_resp_buffer_len;
  kv_store_read_protocol_cmd(
      in_bytes, sizeof(in_bytes), 
      s_resp_buffer, &len);

  mock().checkExpectations();
  MEMCMP_EQUAL(out_bytes, s_resp_buffer, sizeof(out_bytes));
}
