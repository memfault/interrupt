#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C" {
  #include <string.h>
  #include <stddef.h>

  #include "protocol/protocol.h"
  #include "protocol/registry.h"
}


static uint8_t s_resp_buffer[1024];
static const size_t s_resp_buffer_len = sizeof(s_resp_buffer);

static void prv_command_hello(const uint8_t *buffer, size_t len,
    uint8_t *resp_buffer, size_t *resp_len) {

  mock()
      .actualCall(__func__)
      .withParameter("len", len)
      .withMemoryBufferParameter("buffer", buffer, len);
  // Ignore resp buffer, nothing to check
}

static const sProtocolCommand s_protocol_commands[] = {
  {1234, prv_command_hello},
};

const sProtocolCommand *const g_protocol_commands = s_protocol_commands;
const size_t g_num_protocol_commands = 1;


TEST_GROUP(TestProtocolParser) {
  void setup() {
  }

  void teardown() {
    mock().checkExpectations();
    mock().clear();
  }
};


TEST(TestProtocolParser, Hello) {
  const uint8_t in_bytes[] = {
      0xD2, 0x04, 0x00, 0x00, // Code (1234)
      0x04, 0x00, 0x00, 0x00, // Payload Size (4)
      0xFF, 0xFF, 0xFF, 0xFF, // Payload (junk)
  };

  const uint8_t payload_bytes[] = {
      0xFF, 0xFF, 0xFF, 0xFF, // Payload
  };

  mock()
      .expectOneCall("prv_command_hello")
      .withMemoryBufferParameter("buffer",
                                 payload_bytes,
                                 sizeof(payload_bytes))
      .withParameter("len", sizeof(payload_bytes))
      .ignoreOtherParameters();

  size_t len = s_resp_buffer_len;
  eProtocolCode rv = protocol_handle(
      in_bytes, sizeof(in_bytes),
      s_resp_buffer, &len);

  CHECK_EQUAL(kProtocolCode_Ok, rv);
}

TEST(TestProtocolParser, MessageTooShort) {
  const uint8_t stream[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  size_t len = s_resp_buffer_len;
  eProtocolCode rv = protocol_handle(stream, sizeof(stream), s_resp_buffer, &len);
  CHECK_EQUAL(kProtocolCode_MalformedMsg, rv);
}


TEST(TestProtocolParser, NullMessageData) {
  size_t len = s_resp_buffer_len;
  eProtocolCode rv = protocol_handle(NULL, 0, s_resp_buffer, &len);
  CHECK_EQUAL(kProtocolCode_MalformedMsg, rv);
}
