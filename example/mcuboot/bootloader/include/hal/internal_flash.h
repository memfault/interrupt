#pragma once

#include <stddef.h>
#include <stdint.h>

void example_internal_flash_write(uint32_t addr, const void *buf, size_t length);
void example_internal_flash_read(uint32_t addr, void *buf, size_t length);
void example_internal_flash_erase_sector(uint32_t addr);
