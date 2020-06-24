#include "image.h"
#include "crc32.h"
#include "memory_map.h"

#include <libopencm3/cm3/scb.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

static void prv_start_image(void *pc, void *sp) {
    __asm("           \n\
          msr msp, r1 \n\
          bx r0       \n\
    ");
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
    if (a == b) {
        return 0;
    } else {
        printf("CRC Mismatch: %lx vs %lx\n", a, b);
        return -1;
    }
}

void image_start(const image_hdr_t *hdr) {
    const vector_table_t *vectors = (const vector_table_t *)hdr->vector_addr;
    SCB_VTOR = (uint32_t)vectors;
    prv_start_image(vectors->reset, vectors->initial_sp_value);
     __builtin_unreachable();
}
