COMPONENT_NAME=test_shell_commands

SRC_FILES = \
  $(PROJECT_SRC_DIR)/shell_commands.c \

TEST_SRC_FILES = \
  $(UNITTEST_SRC_DIR)/test_shell_commands.cpp

include $(CPPUTEST_MAKFILE_INFRA)
