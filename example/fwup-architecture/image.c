#include "image.h"
#include "memory_map.h"

#include <libopencm3/cm3/scb.h>
#include <stdint.h>
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

const vector_table_t *image_get_vectors(image_slot_t slot) {
    const image_hdr_t *hdr = image_get_header(slot);
    if (hdr) {
        return (const vector_table_t *)hdr->vector_addr;
    }

    return NULL;
}

void image_boot_vectors(const vector_table_t *vectors) {
    SCB_VTOR = (uint32_t)vectors;
    prv_start_image(vectors->reset, vectors->initial_sp_value);
     __builtin_unreachable();
}
