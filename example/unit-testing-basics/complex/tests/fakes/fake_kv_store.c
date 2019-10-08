#include "settings_file.h"

#include "lfs.h"
#include <stdio.h>

void kv_store_init(lfs_t *lfs) {
  // memset RAM buffer
}

bool kv_store_write(const char *key, const void *val, uint32_t len) {
  // Write value into RAM key-value store
}

uint32_t kv_store_read(const char *key, void *buf, uint32_t buf_len) {
  // Read value from RAM key-value store
}

bool kv_store_delete(const char *key) {
  // Delete value from RAM key-value store
}
