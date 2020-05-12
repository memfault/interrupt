#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C" {
  #include <string.h>
  #include <stddef.h>

  #include "protocol/protocol.h"
  #include "protocol/registry.h"

  #include "lfs.h"
  #include "emubd/lfs_emubd.h"

  #include "defs/lfs_default_config.h"
  #include "kv_store/kv_store.h"


  #include "stubs/stub_analytics.h"
  #include "fakes/fake_mutex.h"
}


static uint8_t s_resp_buffer[1024];
static const size_t s_resp_buffer_len = sizeof(s_resp_buffer);


TEST_GROUP(TestIntegrationProtocolKvStore) {
  void setup() {
    fake_mutex_init();

    lfs_emubd_create(&cfg, "blocks");
    lfs_format(&lfs, &cfg);
    lfs_mount(&lfs, &cfg);

    kv_store_init(&lfs);
    memset(s_resp_buffer, 0, s_resp_buffer_len);
  }

  void teardown() {
    lfs_emubd_destroy(&cfg);
    lfs_unmount(&lfs);

    CHECK(fake_mutex_all_unlocked());
  }
};

TEST(TestIntegrationProtocolKvStore, Read) {
  const uint8_t in_bytes[] = {
      0xE8, 0x03, 0x00, 0x00,       // Code (1000)
      0x06, 0x00, 0x00, 0x00,       // Payload Size (6)
      'h', 'e', 'l', 'l', 'o', '\0' // Payload ("hello")
  };

  const uint8_t val_bytes[] = {
      'w', 'o', 'r', 'l', 'd'       // Payload
  };

  bool success = kv_store_write("hello", val_bytes, sizeof(val_bytes));
  CHECK_TRUE(success);

  size_t len = s_resp_buffer_len;
  eProtocolCode rv = protocol_handle(
      in_bytes, sizeof(in_bytes), 
      s_resp_buffer, &len);

  CHECK_EQUAL(kProtocolCode_Ok, rv);
  MEMCMP_EQUAL(val_bytes, s_resp_buffer, sizeof(val_bytes));
}


TEST(TestIntegrationProtocolKvStore, Write) {
  const uint8_t in_bytes[] = {
      0xE9, 0x03, 0x00, 0x00,        // Code (1001)
      0x0B, 0x00, 0x00, 0x00,        // Payload Size (11)
      'h', 'e', 'l', 'l', 'o', '\0', // Payload ("hello")
      'w', 'o', 'r', 'l', 'd'
  };

  const uint8_t val_bytes[] = {
      'w', 'o', 'r', 'l', 'd'       // Payload
  };

  const uint8_t out_bytes[] = {
      1       // Payload
  };

  size_t len = s_resp_buffer_len;
  eProtocolCode rv = protocol_handle(
      in_bytes, sizeof(in_bytes), 
      s_resp_buffer, &len);

  CHECK_EQUAL(kProtocolCode_Ok, rv);
  MEMCMP_EQUAL(out_bytes, s_resp_buffer, sizeof(out_bytes));

  uint32_t val_len;
  kv_store_read("hello", s_resp_buffer, sizeof(s_resp_buffer), &val_len);
  MEMCMP_EQUAL(val_bytes, s_resp_buffer, sizeof(val_bytes));
}
