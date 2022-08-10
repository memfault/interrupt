#include "dfu.h"
#include "memory_map.h"
#include "shared_memory.h"

#include <libopencm3/stm32/f4/flash.h>
#include <stdio.h>
#include <string.h>

int dfu_invalidate_image(image_slot_t slot) {
    // We just write 0s over the image header
    uint32_t addr = (uint32_t)(slot == IMAGE_SLOT_1 ?
                               &__slot1rom_start__ : &__slot2rom_start__);
    for (int i = 0; i < sizeof(image_hdr_t); ++i) {
        flash_program_byte(addr + i, 0);
    }

    return 0;
}

int dfu_commit_image(image_slot_t slot, const image_hdr_t *hdr) {
    uint32_t addr = (uint32_t)(slot == IMAGE_SLOT_1 ?
                               &__slot1rom_start__ : &__slot2rom_start__);
    uint8_t *data_ptr = (uint8_t *)hdr;
    for (int i = 0; i < sizeof(image_hdr_t); ++i) {
        flash_program_byte(addr + i, data_ptr[i]);
    }

    // new app -- reset the boot counter
    shared_memory_clear_boot_counter();

    return 0;
}

int dfu_read(image_slot_t slot, void *ptr, long int offset, size_t count) {
    void *addr = (slot == IMAGE_SLOT_1 ? &__slot1rom_start__ : &__slot2rom_start__);
    addr += offset;
    // FIXME this needs slot overflow checks
    memcpy(ptr, addr, count);
    return count;
}

int dfu_write(image_slot_t slot, const void *ptr, long int offset, size_t count) {
    uint32_t addr = (uint32_t)(slot == IMAGE_SLOT_1 ? &__slot1rom_start__ : &__slot2rom_start__);
    addr += offset;
    // FIXME this needs slot overflow checks
    flash_program(addr, ptr, count);
    return count;
}

int dfu_write_data(image_slot_t slot, uint8_t *data, uint32_t len) {
    uint32_t addr;
    int start_sector;
    int end_sector;

    switch (slot) {
        case IMAGE_SLOT_1:
            addr = (uint32_t)&__slot1rom_start__;
            start_sector = 1;
            end_sector = 4;
            return -1;
        case IMAGE_SLOT_2:
            addr = (uint32_t)&__slot2rom_start__;
            start_sector = 5;
            end_sector = 11;
            break;
        default:
            return -1;
    }

    addr += sizeof(image_hdr_t);

    for (int sector = start_sector; sector <= end_sector; ++sector) {
        // XXX -- Renode implements STM32 flash as generic Memory
        flash_erase_sector(sector, 0);
    }
    flash_program(addr, data, len);

    return 0;
}
