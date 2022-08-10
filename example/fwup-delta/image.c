#include "image.h"
#include "crc32.h"
#include "memory_map.h"

#include <sha2.h>
#include <micro-ecc/uECC.h>
#include <libopencm3/cm3/scb.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>


// Private key generated with `openssl ecparam -name secp256k1 -genkey -noout -out private.pem`
// Public key generated with `openssl ec -in private.pem -pubout -out public.pem`
static const uint8_t PUBKEY[] = {
  0xd0, 0xe6, 0xa7, 0xa5, 0x4e, 0x33, 0x0e, 0xbb, 0xd9, 0x9e, 0xe6, 0x8f, 0x59,
  0xff, 0xb6, 0xc1, 0x19, 0x76, 0x28, 0x60, 0x88, 0x16, 0x6a, 0x17, 0x8b, 0x7b,
  0xe0, 0x66, 0xcf, 0x7b, 0x71, 0x0d, 0xf5, 0xcc, 0x95, 0x76, 0x22, 0xae, 0x0e,
  0xa4, 0xef, 0x49, 0xbd, 0x07, 0x2a, 0x71, 0x49, 0x84, 0x49, 0x78, 0xeb, 0x34,
  0xe5, 0x78, 0xb3, 0xa7, 0x96, 0x48, 0x89, 0x7c, 0x4f, 0xd1, 0x7e, 0xa5
  // 64 bytes
};

static void prv_start_image(void *pc, void *sp) {
    __asm("           \n\
          msr msp, r1 \n\
          bx r0       \n\
    ");
}

static void prv_sha256(const void *buf, uint32_t size, uint8_t *hash_out)
{
  cf_sha256_context ctx;
  cf_sha256_init(&ctx);
  cf_sha256_update(&ctx, buf, size);
  cf_sha256_digest_final(&ctx, hash_out);
}

const image_hdr_t *image_get_header(image_slot_t slot) {
    const image_hdr_t *hdr = NULL;

    switch (slot) {
        case IMAGE_SLOT_1:
            hdr = (const image_hdr_t *)&__slot1rom_start__;
            break;
        case IMAGE_SLOT_2:
            hdr = (const image_hdr_t *)&__slot2rom_start__;
            break;
        default:
            break;
    }

    if (hdr && hdr->image_magic == IMAGE_MAGIC) {
        return hdr;
    } else {
        return NULL;
    }
}

int image_validate(image_slot_t slot, const image_hdr_t *hdr) {
    void *addr = (slot == IMAGE_SLOT_1 ? &__slot1rom_start__ : &__slot2rom_start__);
    addr += sizeof(image_hdr_t);
    uint32_t len = hdr->data_size;
    uint32_t a = crc32(addr, len);
    uint32_t b = hdr->crc;
    if (a != b) {
        printf("CRC Mismatch: %lx vs %lx\n", a, b);
        return -1;
    }

    return 0;
}

int image_check_signature(image_slot_t slot, const image_hdr_t *hdr) {
    void *addr = (slot == IMAGE_SLOT_1 ? &__slot1rom_start__ : &__slot2rom_start__);
    addr += sizeof(image_hdr_t);
    uint32_t len = hdr->data_size;

    uint8_t hash[CF_SHA256_HASHSZ];
    prv_sha256(addr, len, hash);

    const struct uECC_Curve_t *curve = uECC_secp256k1();
    if (!uECC_valid_public_key(PUBKEY, curve)) {
        return -1;
    }

    if (!uECC_verify(PUBKEY, hash, CF_SHA256_HASHSZ, hdr->ecdsa_sig, curve)) {
        return -1;
    }

    return 0;
}

void image_start(const image_hdr_t *hdr) {
    const vector_table_t *vectors = (const vector_table_t *)hdr->vector_addr;
    SCB_VTOR = (uint32_t)vectors;
    prv_start_image(vectors->reset, vectors->initial_sp_value);
     __builtin_unreachable();
}
