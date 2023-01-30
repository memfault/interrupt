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

typedef enum {
    IMAGE_VERSION_1 = 1,
    IMAGE_VERSION_2 = 2,
    IMAGE_VERSION_CURRENT = IMAGE_VERSION_2,
} image_version_t;

typedef struct __attribute__((packed)) {
    uint16_t image_magic;
    uint16_t image_hdr_version;
    uint32_t crc;
    uint32_t data_size;
    uint8_t image_type;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_patch;
    uint32_t vector_addr;
    uint32_t reserved;
    char git_sha[8];
    uint8_t ecdsa_sig[64];
} image_hdr_t;

const image_hdr_t *image_get_header(image_slot_t slot);

int image_validate(image_slot_t slot, const image_hdr_t *hdr);

int image_check_signature(image_slot_t slot, const image_hdr_t *hdr);

void image_start(const image_hdr_t *hdr) __attribute__((noreturn));
