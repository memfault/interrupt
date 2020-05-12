// test configuration options
#ifndef LFS_READ_SIZE
#define LFS_READ_SIZE 16
#endif

#ifndef LFS_PROG_SIZE
#define LFS_PROG_SIZE LFS_READ_SIZE
#endif

#ifndef LFS_BLOCK_SIZE
#define LFS_BLOCK_SIZE 512
#endif

#ifndef LFS_BLOCK_COUNT
#define LFS_BLOCK_COUNT 1024
#endif

#ifndef LFS_BLOCK_CYCLES
#define LFS_BLOCK_CYCLES 1024
#endif

#ifndef LFS_CACHE_SIZE
#define LFS_CACHE_SIZE (64 % LFS_PROG_SIZE == 0 ? 64 : LFS_PROG_SIZE)
#endif

#ifndef LFS_LOOKAHEAD_SIZE
#define LFS_LOOKAHEAD_SIZE 16
#endif

// lfs declarations
static lfs_t lfs;
static lfs_emubd_t bd;
// other declarations for convenience
static lfs_file_t file;
static lfs_dir_t dir;
static struct lfs_info info;
static uint8_t buffer[1024];
static char path[1024];

const struct lfs_config cfg = {
  .context = &bd,
  .read  = &lfs_emubd_read,
  .prog  = &lfs_emubd_prog,
  .erase = &lfs_emubd_erase,
  .sync  = &lfs_emubd_sync,

  .read_size      = LFS_READ_SIZE,
  .prog_size      = LFS_PROG_SIZE,
  .block_size     = LFS_BLOCK_SIZE,
  .block_count    = LFS_BLOCK_COUNT,
  .block_cycles   = LFS_BLOCK_CYCLES,
  .cache_size     = LFS_CACHE_SIZE,
  .lookahead_size = LFS_LOOKAHEAD_SIZE,
};
