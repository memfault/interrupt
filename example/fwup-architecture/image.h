#pragma once

#include <stdint.h>

typedef enum {
    IMAGE_TYPE_LOADER = 0x1,
    IMAGE_TYPE_APP = 0x2,
    IMAGE_TYPE_UPDATER = 0x3,
} image_type_t;

typedef struct __attribute__((packed)) {
    uint8_t image_type;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_patch;
    uint32_t vector_addr;
} image_hdr_t;

