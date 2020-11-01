#pragma once

#include <inttypes.h>
#include <stddef.h>

//! Initialized graphics subsystem
//!
//! @param buf pointer to beginning of graphics buffer array
//! @pram len length of graphics buffer array
void graphics_boot(uint16_t *buf, size_t len);
