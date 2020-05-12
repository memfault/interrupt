COMPONENT_NAME=littlefs_basic

SRC_FILES = \
  $(PROJECT_SRC_DIR)/littlefs/lfs.c \
  $(PROJECT_SRC_DIR)/littlefs/lfs_util.c \
  $(PROJECT_SRC_DIR)/littlefs/emubd/lfs_emubd.c \

TEST_SRC_FILES = \
  $(UNITTEST_SRC_DIR)/test_littlefs_basic.c

include $(CPPUTEST_MAKFILE_INFRA)
