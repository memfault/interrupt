COMPONENT_NAME=kv_store

SRC_FILES = \
  $(PROJECT_SRC_DIR)/littlefs/lfs.c \
  $(PROJECT_SRC_DIR)/littlefs/lfs_util.c \
  $(PROJECT_SRC_DIR)/littlefs/emubd/lfs_emubd.c \
  $(PROJECT_SRC_DIR)/kv_store/kv_store.c \
  $(UNITTEST_ROOT)/fakes/fake_mutex.c \

TEST_SRC_FILES = \
  $(UNITTEST_SRC_DIR)/test_kv_store.c

include $(CPPUTEST_MAKFILE_INFRA)
