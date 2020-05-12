COMPONENT_NAME=protocol

SRC_FILES = \
  $(PROJECT_SRC_DIR)/protocol/protocol.c \

TEST_SRC_FILES = \
  $(UNITTEST_SRC_DIR)/test_protocol.cpp

include $(CPPUTEST_MAKFILE_INFRA)
