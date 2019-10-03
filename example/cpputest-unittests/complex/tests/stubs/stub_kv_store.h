#include "settings_file.h"

#include "lfs.h"
#include <stdio.h>

void kv_store_init(lfs_t *lfs) {
}

bool kv_store_write(const char *key, const void *val, uint32_t len) {
  return true;
}

uint32_t kv_store_read(const char *key, void *buf, uint32_t buf_len) {
  return 0;
}

bool kv_store_delete(const char *key) {
  return true;
}
