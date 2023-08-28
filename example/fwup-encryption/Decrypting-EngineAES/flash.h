#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "stm32l4s5xx.h"
#include "stm32l4xx.h"

bool flash_unlock(void);
bool flash_lock(void);
void flash_write(uint32_t Address, uint32_t Data);
void flash_erase(uint32_t ErasePage, uint32_t NbPages);