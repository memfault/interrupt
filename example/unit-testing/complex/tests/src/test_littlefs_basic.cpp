#include "CppUTest/MemoryLeakDetectorMallocMacros.h"
#include "CppUTest/MemoryLeakDetectorNewMacros.h"
#include "CppUTest/TestHarness.h"

extern "C" {
  #include <string.h>
  #include <stddef.h>
  #include <unistd.h>

  #include "lfs.h"
  #include "emubd/lfs_emubd.h"

  #include "defs/lfs_default_config.h"
}

TEST_GROUP(TestFiles) {
  void setup() {
    const char *d = "blocks_test_files";
    unlink(d);
    lfs_emubd_create(&cfg, d);
  }

  void teardown() {
    lfs_emubd_destroy(&cfg);
  }
};

TEST(TestFiles, Test_SimpleFileTest) {
  LONGS_EQUAL(0, lfs_format(&lfs, &cfg));
  LONGS_EQUAL(0, lfs_mount(&lfs, &cfg));
  LONGS_EQUAL(0, lfs_file_open(&lfs, &file, "hello", LFS_O_WRONLY | LFS_O_CREAT));
  lfs_size_t size = strlen("Hello World!\n");
  uint8_t wbuffer[1024];
  memcpy(wbuffer, "Hello World!\n", size);
  LONGS_EQUAL(size, lfs_file_write(&lfs, &file, wbuffer, size));
  LONGS_EQUAL(0, lfs_file_close(&lfs, &file));
  
  LONGS_EQUAL(0, lfs_file_open(&lfs, &file, "hello", LFS_O_RDONLY));
  size = strlen("Hello World!\n");
  uint8_t rbuffer[1024];
  LONGS_EQUAL(size, lfs_file_read(&lfs, &file, rbuffer, size));
  memcmp(rbuffer, wbuffer, size);
  LONGS_EQUAL(0, lfs_file_close(&lfs, &file));
  LONGS_EQUAL(0, lfs_unmount(&lfs));
}
