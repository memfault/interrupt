#pragma once

#include <libopencm3/cm3/vector.h>
#include <stdint.h>

#define IMAGE_MAGIC 0xcafe

typedef enum {
    IMAGE_TYPE_LOADER = 0x1,
    IMAGE_TYPE_APP = 0x2,
    IMAGE_TYPE_UPDATER = 0x3,
} image_type_t;

typedef enum {
    IMAGE_SLOT_1 = 1,
    IMAGE_SLOT_2 = 2,
    IMAGE_NUM_SLOTS,
} image_slot_t;

typedef struct __attribute__((packed)) {
    uint16_t image_magic;
    uint16_t image_hdr_version;
    uint32_t crc;
    uint8_t image_type;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_patch;
    uint32_t vector_addr;
} image_hdr_t;

const image_hdr_t *image_get_header(image_slot_t slot);

const vector_table_t *image_get_vectors(image_slot_t slot);

void image_boot_vectors(const vector_table_t *vectors) __attribute__((noreturn));
