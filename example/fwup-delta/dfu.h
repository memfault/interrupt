#pragma once

#include "image.h"
#include <stddef.h>
#include <stdint.h>

int dfu_invalidate_image(image_slot_t slot);

int dfu_commit_image(image_slot_t slot, const image_hdr_t *hdr);

int dfu_read(image_slot_t slot, void *ptr, long int offset, size_t count);

int dfu_write(image_slot_t slot, const void *ptr, long int offset, size_t count);

int dfu_write_data(image_slot_t slot, uint8_t *data, uint32_t len);
