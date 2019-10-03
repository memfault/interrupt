#pragma once

#include <stdint.h>

#include "lfs.h"

void kv_store_init(lfs_t *lfs);

bool kv_store_write(const char *key, const void *val, uint32_t len);

bool kv_store_read(const char *key, void *buf, uint32_t buf_len, uint32_t *len_read);

bool kv_store_delete(const char *key);
