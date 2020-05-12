COMPONENT_NAME=kv_store_protocol_handlers

SRC_FILES = \
  $(PROJECT_SRC_DIR)/kv_store/kv_store_protocol_handlers.c \

TEST_SRC_FILES = \
  $(UNITTEST_SRC_DIR)/test_kv_store_protocol_handlers.cpp

include $(CPPUTEST_MAKFILE_INFRA)
