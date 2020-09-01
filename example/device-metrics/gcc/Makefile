ROOT_DIR := $(abspath ..)

# FIXME: We don't want to -c with an absolute path but
# if we do that the .d deps generated have a path relative to
# root dir for the .c deps. So we add ROOT_DIR to vpath to work around the issue for now
VPATH = $(ROOT_DIR)

GCC_DIR := $(ROOT_DIR)/gcc

ROOT_DIR_SRC := $(ROOT_DIR)/src
CONFIG_DIR := $(ROOT_DIR)/config

BUILD_DIRNAME ?= build
BUILD_DIR  := $(GCC_DIR)/$(BUILD_DIRNAME)
Q ?= @

FREERTOS_ROOT_DIR := $(ROOT_DIR)/freertos_kernel

COMPILER ?= arm-none-eabi-gcc

CPU_CFLAGS = \
  -mcpu=cortex-m4 \
  -mfloat-abi=hard -mfpu=fpv4-sp-d16 \

FREERTOS_PORT ?= ARM_CM4F

FREERTOS_PORT_ROOT += \
  $(FREERTOS_ROOT_DIR)/portable/GCC/$(FREERTOS_PORT)

FREERTOS_PORT_COMMON_DIR = \
  $(FREERTOS_ROOT_DIR)/portable/common/

FREERTOS_KERNEL_SOURCES += \
  $(FREERTOS_ROOT_DIR)/tasks.c \
  $(FREERTOS_ROOT_DIR)/queue.c \
  $(FREERTOS_ROOT_DIR)/list.c \
  $(FREERTOS_ROOT_DIR)/timers.c \
  $(FREERTOS_PORT_ROOT)/port.c \
  $(FREERTOS_ROOT_DIR)/portable/MemMang/heap_4.c

PROJECT_SRC_DIR = $(GCC_DIR)

SRC_FILES += \
  $(ROOT_DIR_SRC)/main.c \
  $(ROOT_DIR_SRC)/startup.c \
  $(PROJECT_SRC_DIR)/gcc_startup.c \
  $(FREERTOS_KERNEL_SOURCES)

INCLUDE_PATHS += \
  $(FREERTOS_ROOT_DIR)/include \
  $(ROOT_DIR)/include \
  $(ROOT_DIR)/src/device_metrics \
  $(FREERTOS_PORT_ROOT) \
  $(CONFIG_DIR)

SRC_FILES += \
  $(ROOT_DIR_SRC)/example_assert.c \
  $(ROOT_DIR_SRC)/example_log.c \
  $(ROOT_DIR_SRC)/uart_deprecated.c \
  $(ROOT_DIR_SRC)/device_metrics/metrics.c \

# https://community.memfault.com/t/reproducible-firmware-builds-interrupt/112/12
SRC_FILES := $(sort $(SRC_FILES))

INCLUDES = $(foreach d, $(INCLUDE_PATHS), -I$d)

CFLAGS += \
  $(CPU_CFLAGS) \
  -mthumb \
  -g3 \
  -nostdlib \
  -ffunction-sections -fdata-sections \
  -Wall \
  -Werror \
  -Os \
  -fdebug-prefix-map=$(ROOT_DIR)=. \
  -std=gnu99

# GCC 4.9
CFLAGS += -Wno-missing-braces

TARGET ?= nrf52
LDSCRIPT = $(GCC_DIR)/$(TARGET).ld
TARGET_ELF = $(BUILD_DIR)/$(TARGET).elf

LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,--gc-sections,-Map,$(BUILD_DIR)/$(TARGET).map -Wl,--build-id -Wl,--nmagic

DEP_DIR = $(BUILD_DIR)/dep
DEP_CFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$(*F).d

OBJ_FILES := $(patsubst $(ROOT_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

all: $(TARGET_ELF)

-include $(wildcard $(DEP_DIR)/*.d)

clean:
	rm -rf $(BUILD_DIR)

$(TARGET_ELF): $(OBJ_FILES) $(LDSCRIPT)
	@echo "Linking library"
	@arm-none-eabi-gcc $(CFLAGS) $(LDFLAGS) $(OBJ_FILES)  -o $@ -Wl,-lc -Wl,-lgcc
	@echo "Generated $(patsubst $(ROOT_DIR)/%,%,$@)"

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(DEP_DIR):
	@mkdir -p $(DEP_DIR)

# FreeRTOS should really be included as a submodule in a real project
# Let's do a cheap hack and clone it here instead
$(FREERTOS_PORT_ROOT):
	git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git $(FREERTOS_ROOT_DIR)


$(FREERTOS_KERNEL_SOURCES): $(FREERTOS_PORT_ROOT)

$(OBJ_FILES): Makefile

$(BUILD_DIR)/%.o: $(ROOT_DIR)/%.c | $(BUILD_DIR) $(DEP_DIR) $(FREERTOS_PORT_ROOT)
	@echo "Compiling $*.c"
	@mkdir -p $(dir $@)
	$(Q) cd $(ROOT_DIR) && arm-none-eabi-gcc $(DEP_CFLAGS) $(CFLAGS) $(INCLUDES) -c -o $@ $*.c
