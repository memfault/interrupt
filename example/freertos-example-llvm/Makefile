ROOT_DIR := $(abspath .)
BUILD_DIR  := $(ROOT_DIR)/build
Q ?= @

FREERTOS_ROOT_DIR := $(ROOT_DIR)/freertos_kernel

# CFLAGS which can provided from the CLI
# They will be appended last and will consequently override
# any pre-existing settings
CLI_CFLAG_OVERRIDES ?=
CLI_LDFLAG_OVERRIDES ?=

# Set things up to use the arm-none-eabi-gcc that is on
# our path by default but allow things to be easily overridden, i.e:
#
# COMPILER=<PATH_TO_OTHER_GCC_VERSION>/arm-none-eabi-gcc make
# COMPILER=clang make
COMPILER ?= arm-none-eabi-gcc
CC = $(COMPILER)

$(info $(CC))

FREERTOS_PORT ?= ARM_CM4F

FREERTOS_PORT_ROOT += \
  $(FREERTOS_ROOT_DIR)/portable/GCC/$(FREERTOS_PORT)

FREERTOS_KERNEL_SOURCES += \
  $(FREERTOS_ROOT_DIR)/tasks.c \
  $(FREERTOS_ROOT_DIR)/queue.c \
  $(FREERTOS_ROOT_DIR)/list.c \
  $(FREERTOS_ROOT_DIR)/timers.c \
  $(FREERTOS_PORT_ROOT)/port.c \
  $(FREERTOS_ROOT_DIR)/portable/MemMang/heap_1.c

PROJECT_SRC_DIR = $(ROOT_DIR)/src

SRC_FILES += \
  $(PROJECT_SRC_DIR)/builtin_scanbuild_examples.c \
  $(PROJECT_SRC_DIR)/main.c \
  $(PROJECT_SRC_DIR)/memory_leak_examples.c \
  $(PROJECT_SRC_DIR)/memory_pool.c \
  $(PROJECT_SRC_DIR)/mutex_examples.c \
  $(PROJECT_SRC_DIR)/stub_functions.c \
  $(PROJECT_SRC_DIR)/freertos_lock_port.c \
  $(PROJECT_SRC_DIR)/startup.c \
  $(FREERTOS_KERNEL_SOURCES)

OBJ_FILES := $(patsubst $(ROOT_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

INCLUDE_PATHS += \
  -I$(FREERTOS_ROOT_DIR)/include \
  -I$(ROOT_DIR)/include \
  -I$(FREERTOS_PORT_ROOT)

# Note: These arguments will be used by clang to select
# the specific architecture target when we use the
# generic --target=arm-none-eabi
ARCH_CFLAGS += \
  -mthumb -mcpu=cortex-m4 \
  -mfloat-abi=hard -mfpu=fpv4-sp-d16

CC_VERSION_INFO ="$(shell $(CC) -v 2>&1 | head -2)"

ifneq '' '$(findstring clang,$(CC_VERSION_INFO))'
$(info ===Clang Compiler Detected===)

# Default assumes an GNU ARM toolchain is on your path
# but an explicit path can also be provided by doing
# ARM_GNU_CC=<PATH_TO_GCC> make
ARM_GNU_CC ?= arm-none-eabi-gcc

ARM_CORTEXM_SYSROOT = \
  $(shell $(ARM_GNU_CC) $(ARCH_CFLAGS) -print-sysroot 2>&1)

# The directory where Newlib's libc.a & libm.a reside
# for the specific target architecture
ARM_CORTEXM_MULTI_DIR = \
  $(shell $(ARM_GNU_CC) $(ARCH_CFLAGS) -print-multi-directory 2>&1)

# Pick up builtins needed for compilation
ARM_CORTEXM_BUILTINS ?= \
  $(shell $(ARM_GNU_CC) $(ARCH_CFLAGS) -print-libgcc-file-name 2>&1)

COMPILER_SPECIFIC_CFLAGS += \
  --target=arm-none-eabi \
  --sysroot=$(ARM_CORTEXM_SYSROOT) \
  -Oz


#-Wthread-safety \
#  -Wthread-safety-beta

COMPILER_SPECIFIC_LDFLAGS += \
  -L$(ARM_CORTEXM_SYSROOT)/lib/$(ARM_CORTEXM_MULTI_DIR) \
  $(ARM_CORTEXM_BUILTINS)

endif

ifneq '' '$(findstring gcc,$(CC_VERSION_INFO))'
$(info ===GCC Compiler Detected===)

COMPILER_SPECIFIC_CFLAGS += \
  -Os

COMPILER_SPECIFIC_LDFLAGS += \
  -Wl,-lgcc

endif


CFLAGS += \
  -g3 \
  -ffunction-sections -fdata-sections \
  -Wall \
  -Werror \
  -nostdlib \
  $(COMPILER_SPECIFIC_CFLAGS) \
  $(ARCH_CFLAGS) \
  $(CLI_CFLAG_OVERRIDES)



TARGET ?= nrf52
LDSCRIPT = $(ROOT_DIR)/$(TARGET).ld
TARGET_ELF = $(BUILD_DIR)/$(TARGET).elf

LDFLAGS += $(COMPILER_SPECIFIC_LDFLAGS)
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,--gc-sections,-Map,$(BUILD_DIR)/$(TARGET).map
LDFLAGS += -Wl,-lc -lm
LDFLAGS += $(CLI_LDFLAG_OVERRIDES)

LINKER_OVERRIDE ?=

DEP_DIR = $(BUILD_DIR)/dep
DEP_CFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$(*F).d

all: $(TARGET_ELF)

-include $(wildcard $(DEP_DIR)/*.d)

clean:
	rm -rf $(BUILD_DIR)

$(TARGET_ELF): $(OBJ_FILES) $(LDSCRIPT)
	@echo "Linking library"
	$(Q) $(CC) $(CFLAGS) $(OBJ_FILES) $(LDFLAGS) -o $@
	@echo "Generated $(patsubst $(ROOT_DIR)/%,%,$@)"

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(DEP_DIR):
	@mkdir -p $(DEP_DIR)

# FreeRTOS should really be included as a submodule in a real project
# Let's do a cheap hack and clone it here instead
$(FREERTOS_PORT_ROOT):
	git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git $(FREERTOS_ROOT_DIR)

$(OBJ_FILES): $(SRC_FILES) Makefile

$(BUILD_DIR)/%.o: $(ROOT_DIR)/%.c | $(BUILD_DIR) $(DEP_DIR) $(FREERTOS_PORT_ROOT)
	@echo "Compiling $(patsubst $(ROOT_DIR)/%,%,$<)"
	@mkdir -p $(dir $@)
	$(Q) $(CC) $(DEP_CFLAGS) $(CFLAGS) $(INCLUDE_PATHS) -c -o $@ $<
