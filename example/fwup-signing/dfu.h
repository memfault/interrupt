#pragma once

#include "image.h"
#include <stdint.h>

int dfu_invalidate_image(image_slot_t slot);

int dfu_commit_image(image_slot_t slot, const image_hdr_t *hdr);

int dfu_write_data(image_slot_t slot, uint8_t *data, uint32_t len);
