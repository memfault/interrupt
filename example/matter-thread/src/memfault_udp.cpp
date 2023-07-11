/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */


#include <stdio.h>
#include <zephyr.h>

#include <dk_buttons_and_leds.h>

#include <net/socket.h>

extern "C" {
#include "memfault/core/platform/device_info.h"
#include <memfault/core/data_packetizer.h>
#include <memfault/core/trace_event.h>
#include <memfault/metrics/metrics.h>
#include <memfault/ports/zephyr/http.h>

#include "memfault_ncs.h"
}

#include <logging/log.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceInstanceInfoProvider.h>

LOG_MODULE_REGISTER(memfault_sample, CONFIG_KERNEL_LOG_LEVEL);

static int client_fd;
static struct sockaddr_storage host_addr;
static struct k_work_delayable memfault_chunk_sender_work;

static char udp_message[CONFIG_UDP_DATA_UPLOAD_SIZE_BYTES];
char *udp_message_cursor = udp_message;
size_t udp_message_remaining_bytes = CONFIG_UDP_DATA_UPLOAD_SIZE_BYTES;

typedef struct UdpMessageChunkSection {
  char *start_addr;
  size_t size;
} sUdpMessageChunkSection;

static sUdpMessageChunkSection udp_message_chunk_section;

int append_to_udp_message(const char *section, size_t section_size) {
  if (section_size > udp_message_remaining_bytes) {
    LOG_DBG("Message too big, increase CONFIG_UDP_DATA_UPLOAD_SIZE_BYTES");
    return 1;
  }
  strncpy(udp_message_cursor, section, udp_message_remaining_bytes);
  udp_message_remaining_bytes -= section_size;
  udp_message_cursor += section_size;
  return 0;
}

void memfault_init_udp_message(void) {
  sMemfaultDeviceInfo device_info;
  memfault_platform_get_device_info(&device_info);

  append_to_udp_message(CONFIG_UDP_DATA_UPLOAD_VERSION_PREFIX,
                        sizeof(CONFIG_UDP_DATA_UPLOAD_VERSION_PREFIX));
  append_to_udp_message(CONFIG_MEMFAULT_NCS_PROJECT_KEY,
                        sizeof(CONFIG_MEMFAULT_NCS_PROJECT_KEY));
  append_to_udp_message(device_info.device_serial,
                        strlen((char *)device_info.device_serial) + 1);

  LOG_DBG("Finished initialization of UDP message buffer");

  udp_message_chunk_section = (sUdpMessageChunkSection){
      .start_addr = udp_message_cursor,
      .size = udp_message_remaining_bytes,
  };
}

#define UDP_IP_HEADER_SIZE 28
void memfault_chunk_sender_work_fn(struct k_work *work) {
  size_t chunk_buffer_len = udp_message_chunk_section.size;
  size_t size_of_prelude = CONFIG_UDP_DATA_UPLOAD_SIZE_BYTES - chunk_buffer_len;
  const bool success = memfault_packetizer_get_chunk(
      udp_message_chunk_section.start_addr, &chunk_buffer_len);

  // LOG_INF("memfault_chunk_sender_work_fn");

  // under-documented edge-case: success but no data
  if (success && (chunk_buffer_len > 0)) {
    int err;
    size_t udp_message_size = size_of_prelude + chunk_buffer_len;

    LOG_INF("Transmitting UDP/IP payload of %d bytes to the ",
           udp_message_size + UDP_IP_HEADER_SIZE);
    LOG_INF("IP address %s, port number %d\n", CONFIG_UDP_SERVER_ADDRESS_STATIC,
           CONFIG_UDP_SERVER_PORT);

    err = zsock_send(client_fd, udp_message, udp_message_size, 0);
    if (err < 0) {
      LOG_INF("Failed to transmit UDP packet, %d\n", errno);
    } else {
      LOG_INF("Sent UDP packet");
    }
  } else {
    LOG_INF("No Memfault chunks to upload!\n");
  }

  k_work_schedule(&memfault_chunk_sender_work,
                  K_SECONDS(CONFIG_UDP_DATA_UPLOAD_FREQUENCY_SECONDS));
}

void init_memfault_chunks_sender(void) {
  k_work_init_delayable(&memfault_chunk_sender_work,
                        memfault_chunk_sender_work_fn);
}

void server_disconnect(void) { (void)zsock_close(client_fd); }

int memfault_server_init(void) {
  struct sockaddr_in6 *server6 = ((struct sockaddr_in6 *)&host_addr);

  server6->sin6_family = AF_INET6;
  server6->sin6_port = htons(CONFIG_UDP_SERVER_PORT);

  zsock_inet_pton(AF_INET6, CONFIG_UDP_SERVER_ADDRESS_STATIC, &server6->sin6_addr);

  CHIP_ERROR status = CHIP_NO_ERROR;

/*
        constexpr size_t kMaxLen                = DeviceLayer::ConfigurationManager::kMaxHardwareVersionStringLength;
        char hardwareVersionString[kMaxLen + 1] = { 0 };
        status = GetDeviceInstanceInfoProvider()->GetHardwareVersionString(hardwareVersionString, sizeof(hardwareVersionString));

        constexpr size_t kMaxLen                = DeviceLayer::ConfigurationManager::kMaxSoftwareVersionStringLength;
        char softwareVersionString[kMaxLen + 1] = { 0 };
        status = ConfigurationMgr().GetSoftwareVersionString(softwareVersionString, sizeof(softwareVersionString));
*/

  constexpr size_t kMaxLen             = chip::DeviceLayer::ConfigurationManager::kMaxSerialNumberLength;
  char serialNumberString[kMaxLen + 1] = { 0 };
  status = chip::DeviceLayer::GetDeviceInstanceInfoProvider()->GetSerialNumber(serialNumberString, sizeof(serialNumberString));
  if (status == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND || status == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
  {
      serialNumberString[0] = '\0';
  }
  memfault_ncs_device_id_set(serialNumberString, sizeof(serialNumberString));

  return 0;
}

int memfault_server_connect(void) {
  int err = -1;

  client_fd = zsock_socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        if (client_fd < 0) {
    LOG_ERR("Failed to create UDP socket: %d\n", errno);
    server_disconnect();
    return client_fd;
  }

  err = zsock_connect(client_fd, (struct sockaddr *)&host_addr,
                sizeof(struct sockaddr_in6));
  if (err < 0) {
    LOG_ERR("Failed to connect: %d\n", errno);
    server_disconnect();
    return err;
  }

  return 0;
}

void memfault_schedule() {
	k_work_schedule(&memfault_chunk_sender_work, K_NO_WAIT);
}
