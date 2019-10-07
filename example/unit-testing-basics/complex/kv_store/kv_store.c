#include "kv_store.h"
#include "analytics/analytics.h"
#include "mutex/mutex.h"

#include "lfs.h"
#include <stdio.h>

#define KV_DIR "/kv"

static lfs_file_t s_file;
static char s_fname[256];
static lfs_t *s_lfs_ptr;
static Mutex *s_mutex;

static const char *prv_prefix_fname(const char *key) {
  snprintf(s_fname, sizeof(s_fname), "%s/%s", KV_DIR, key);
  return s_fname;
}

void kv_store_init(lfs_t *lfs) {
  s_lfs_ptr = lfs;
  lfs_mkdir(s_lfs_ptr, "/kv");
  s_mutex = mutex_create();
}

bool kv_store_write(const char *key, const void *val, uint32_t len) {
  mutex_lock(s_mutex);

  lfs_file_open(s_lfs_ptr, &s_file, prv_prefix_fname(key), LFS_O_WRONLY | LFS_O_CREAT);
  uint32_t rv = lfs_file_write(s_lfs_ptr, &s_file, val, len);
  lfs_file_close(s_lfs_ptr, &s_file);

  mutex_unlock(s_mutex);

  analytics_inc(kSettingsFileWrite);

  return (rv == len);
}

bool kv_store_read(const char *key, void *buf, uint32_t buf_len, uint32_t *len_read) {
  mutex_lock(s_mutex);

  int rv = lfs_file_open(s_lfs_ptr, &s_file, prv_prefix_fname(key), LFS_O_RDONLY);
  if (rv < 0) {
    mutex_unlock(s_mutex);
    return false;
  }

  uint32_t len = lfs_file_size(s_lfs_ptr, &s_file);
  if (buf_len < len) {
    mutex_unlock(s_mutex);
    return false;
  }

  len = lfs_file_read(s_lfs_ptr, &s_file, buf, buf_len);
  lfs_file_close(s_lfs_ptr, &s_file);
  *len_read = len;

  mutex_unlock(s_mutex);

  analytics_inc(kSettingsFileRead);

  return true;
}

bool kv_store_delete(const char *key) {
  mutex_lock(s_mutex);

  lfs_remove(s_lfs_ptr, prv_prefix_fname(key));

  mutex_unlock(s_mutex);

  analytics_inc(kSettingsFileDelete);

  return true;
}
