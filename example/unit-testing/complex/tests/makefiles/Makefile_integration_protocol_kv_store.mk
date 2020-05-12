COMPONENT_NAME=integration_protocol_kv_store

SRC_FILES = \
  $(PROJECT_SRC_DIR)/littlefs/lfs.c \
  $(PROJECT_SRC_DIR)/littlefs/lfs_util.c \
  $(PROJECT_SRC_DIR)/littlefs/emubd/lfs_emubd.c \
  $(PROJECT_SRC_DIR)/kv_store/kv_store.c \
  $(PROJECT_SRC_DIR)/protocol/protocol.c \
  $(PROJECT_SRC_DIR)/protocol/registry.c \
  $(PROJECT_SRC_DIR)/kv_store/kv_store_protocol_handlers.c \
  $(UNITTEST_ROOT)/fakes/fake_mutex.c \

TEST_SRC_FILES = \
  $(UNITTEST_SRC_DIR)/test_integration_protocol_kv_store.cpp

include $(CPPUTEST_MAKFILE_INFRA)
