ROOT_DIR := $(abspath .)
BUILD_DIR  := $(ROOT_DIR)/build
Q ?= @

GCC_COMPILER ?= arm-none-eabi-gcc
GCC_VERSION := $(strip $(shell $(GCC_COMPILER) -dumpversion))
EXPECTED_GCC_VERSION := 9.2.1
ifneq ($(GCC_VERSION),$(EXPECTED_GCC_VERSION))
$(error Examples were all compiled against $(EXPECTED_GCC_VERSION). You are using $(GCC_VERSION))
endif

FREERTOS_ROOT_DIR := $(ROOT_DIR)/freertos_kernel

COMPILER ?= arm-none-eabi-gcc

FREERTOS_PORT ?= ARM_CM4F

FREERTOS_PORT_ROOT += \
  $(FREERTOS_ROOT_DIR)/portable/GCC/$(FREERTOS_PORT)

FREERTOS_KERNEL_SOURCES += \
  $(FREERTOS_ROOT_DIR)/tasks.c \
  $(FREERTOS_ROOT_DIR)/queue.c \
  $(FREERTOS_ROOT_DIR)/list.c \
  $(FREERTOS_ROOT_DIR)/timers.c \
  $(FREERTOS_PORT_ROOT)/port.c \
  $(FREERTOS_ROOT_DIR)/portable/MemMang/heap_1.c \

PROJECT_SRC_DIR = $(ROOT_DIR)

SRC_FILES += \
  $(PROJECT_SRC_DIR)/main.c \
  $(PROJECT_SRC_DIR)/inline_example.c \
  $(PROJECT_SRC_DIR)/test_enum_to_str_v1.c \
  $(PROJECT_SRC_DIR)/test_enum_to_str_v2.c \
  $(PROJECT_SRC_DIR)/startup.c \
  $(PROJECT_SRC_DIR)/minimal_heap.c \
  $(FREERTOS_KERNEL_SOURCES)

MYLIB_DIR := $(ROOT_DIR)/mylib
MYLIB_SRC_FILES += \
  $(MYLIB_DIR)/file1.c \
  $(MYLIB_DIR)/file2.c \

MYLIB_TARGET = $(BUILD_DIR)/libmylib.a

MYLIB_OBJ_FILES := $(patsubst $(ROOT_DIR)/%.c,$(BUILD_DIR)/%.o,$(MYLIB_SRC_FILES))

OBJ_FILES := $(patsubst $(ROOT_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

INCLUDE_PATH_DIRS = \
  $(FREERTOS_PORT_ROOT) \
  $(FREERTOS_ROOT_DIR)/include \
  $(ROOT_DIR) \

INCLUDE_PATH_DIRS += $(ROOT_DIR)/include

INCLUDE_PATH_DIRS += $(ROOT_DIR)/config

INCLUDE_PATHS = $(foreach d, $(INCLUDE_PATH_DIRS), -I$d)

CFLAGS += \
  -mcpu=cortex-m4 \
  -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
  -mthumb \
  -g3 \
  -nostdlib \
  -ffunction-sections -fdata-sections \
  -Wall \
  -Werror \
  -Os

TARGET ?= nrf52
LDSCRIPT = $(ROOT_DIR)/$(TARGET).ld
TARGET_ELF = $(BUILD_DIR)/$(TARGET).elf
TARGET_BIN = $(BUILD_DIR)/$(TARGET).bin

LDFLAGS += -T$(LDSCRIPT) -Wl,--build-id

LDFLAGS += -Wl,-L$(BUILD_DIR) -Wl,-lmylib
LDFLAGS += -Wl,--gc-sections,-Map,$(BUILD_DIR)/$(TARGET).map

DEP_DIR = $(BUILD_DIR)/dep
DEP_CFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$(*F).d

all: $(TARGET_BIN)

-include $(wildcard $(DEP_DIR)/*.d)

clean:
	rm -rf $(BUILD_DIR)

$(TARGET_BIN): $(TARGET_ELF)
	@echo "Generating binary"
	$(Q) arm-none-eabi-objcopy -O binary $< $@
	@echo "Generated $(patsubst $(ROOT_DIR)/%,%,$@)"

$(TARGET_ELF): $(OBJ_FILES) $(LDSCRIPT) $(MYLIB_TARGET)
	@echo "Linking library"
	@arm-none-eabi-gcc $(CFLAGS) $(LDFLAGS) $(OBJ_FILES) -o $@ -Wl,-lc
	@echo "Generated $(patsubst $(ROOT_DIR)/%,%,$@)"

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(DEP_DIR):
	@mkdir -p $(DEP_DIR)

$(MYLIB_TARGET): $(MYLIB_OBJ_FILES)
	@echo "Generating $@"
	rm -f $@
	arm-none-eabi-ar rcs $@ $(MYLIB_OBJ_FILES)

# FreeRTOS should really be included as a submodule in a real project
# Let's do a cheap hack and clone it here instead
$(FREERTOS_PORT_ROOT):
	git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git $(FREERTOS_ROOT_DIR)

$(BUILD_DIR)/%.o: $(ROOT_DIR)/%.c Makefile | $(BUILD_DIR) $(DEP_DIR) $(FREERTOS_PORT_ROOT)
	@echo "Compiling $*.c"
	@mkdir -p $(dir $@)
	$(Q) arm-none-eabi-gcc $(DEP_CFLAGS) $(CFLAGS) $(INCLUDE_PATHS) -c -o $@ $*.c
