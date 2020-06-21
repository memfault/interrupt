#pragma once

#include <stdbool.h>

void shared_memory_init(void);
bool shared_memory_is_dfu_requested(void);
void shared_memory_set_dfu_requested(bool yes);
