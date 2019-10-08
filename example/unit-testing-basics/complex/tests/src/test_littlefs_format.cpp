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

TEST_GROUP(TestFormat) {
  void setup() {
    const char *d = "blocks_test_format";
    unlink(d);
    lfs_emubd_create(&cfg, d);
  }

  void teardown() {
    lfs_emubd_destroy(&cfg);
  }
};

TEST(TestFormat, Test_BasicFormatting) {
    LONGS_EQUAL(0, lfs_format(&lfs, &cfg));
}
