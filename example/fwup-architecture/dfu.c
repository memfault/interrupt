#include "dfu.h"
#include "memory_map.h"

#include <libopencm3/stm32/f4/flash.h>

int dfu_erase_image(image_slot_t slot) {
    // We just write 0s over the image header
    uint32_t addr = (uint32_t)(slot == IMAGE_SLOT_1 ?
                               &__slot1rom_start__ : &__slot2rom_start__);
    for (int i = 0; i < sizeof(image_hdr_t); ++i) {
        flash_program_byte(addr + i, 0);
    }

    return 0;
}

int dfu_validate_image(image_slot_t slot, image_hdr_t *hdr) {
    return 0;
}

int dfu_commit_image(image_slot_t slot, image_hdr_t *hdr) {
    uint32_t addr = (uint32_t)(slot == IMAGE_SLOT_1 ?
                               &__slot1rom_start__ : &__slot2rom_start__);
    uint8_t *data_ptr = (uint8_t *)hdr;
    for (int i = 0; i < sizeof(image_hdr_t); ++i) {
        flash_program_byte(addr + i, data_ptr[i]);
    }

    return 0;
}

int dfu_write_data(image_slot_t slot, uint8_t *data, uint32_t len) {
    uint32_t addr;
    int sector;

    switch (slot) {
        case IMAGE_SLOT_1:
            // Not implemented
            return -1;
        case IMAGE_SLOT_2:
            if (len > 128 * 1024) {
                // too large
                return -1;
            }
            addr = (uint32_t)&__slot2rom_start__;
            sector = 5;
            break;
        default:
            return -1;
    }

    addr += sizeof(image_hdr_t);

    // FIXME -- Renode implements STM32 flash as generic Memory
    flash_erase_sector(sector, 0);
    flash_program(addr, data, len);

    return 0;
}
