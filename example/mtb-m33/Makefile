ROOT_DIR := $(abspath .)
BUILD_DIR  := $(ROOT_DIR)/build
Q ?= @

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
  $(FREERTOS_ROOT_DIR)/portable/MemMang/heap_1.c

PROJECT_SRC_DIR = $(ROOT_DIR)

SRC_FILES += \
  $(PROJECT_SRC_DIR)/main.c \
  $(PROJECT_SRC_DIR)/startup.c \
  $(PROJECT_SRC_DIR)/mtb.c \
  $(FREERTOS_KERNEL_SOURCES)

OBJ_FILES := $(patsubst $(ROOT_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

INCLUDE_PATHS += \
  -I$(FREERTOS_ROOT_DIR)/include \
  -I. \
  -I$(FREERTOS_PORT_ROOT)

CFLAGS += \
  -mcpu=cortex-m33 \
  -mfloat-abi=hard -mfpu=fpv5-sp-d16 \
  -mthumb \
  -g3 \
  -nostdlib \
  -ffunction-sections -fdata-sections \
  -Wall \
  -Werror \
  -Os \
  -fdebug-prefix-map=$(ROOT_DIR)=.

TARGET ?= da1469x
LDSCRIPT = $(ROOT_DIR)/$(TARGET).ld
TARGET_ELF = $(BUILD_DIR)/$(TARGET).elf

LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,-Map,$(BUILD_DIR)/$(TARGET).map

DEP_DIR = $(BUILD_DIR)/dep
DEP_CFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$(*F).d

all: $(TARGET_ELF)

-include $(wildcard $(DEP_DIR)/*.d)

clean:
	rm -rf $(BUILD_DIR)

flash: $(TARGET_ELF)
	arm-none-eabi-gdb-py --eval-command="target remote localhost:2331"  --ex="mon reset" --ex="load" --ex="mon reset" --ex="source mtb_decode.py"  --ex="break main" --ex="continue" --se=$(TARGET_ELF)


$(TARGET_ELF): $(OBJ_FILES)
	@echo "Linking library"
	@arm-none-eabi-gcc $(CFLAGS) $(LDFLAGS) $^ -o $@ -Wl,-lc -Wl,-lgcc
	@echo "Generated $(patsubst $(ROOT_DIR)/%,%,$@)"

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(DEP_DIR):
	@mkdir -p $(DEP_DIR)

# FreeRTOS should really be included as a submodule in a real project
# Let's do a cheap hack and clone it here instead
$(FREERTOS_PORT_ROOT):
	git clone git@github.com:FreeRTOS/FreeRTOS-Kernel.git $(FREERTOS_ROOT_DIR)

$(OBJ_FILES): $(SRC_FILES) Makefile

$(BUILD_DIR)/%.o: $(ROOT_DIR)/%.c | $(BUILD_DIR) $(DEP_DIR) $(FREERTOS_PORT_ROOT)
	@echo "Compiling $(patsubst $(ROOT_DIR)/%,%,$<)"
	@mkdir -p $(dir $@)
	$(Q) arm-none-eabi-gcc $(DEP_CFLAGS) $(CFLAGS) $(INCLUDE_PATHS) -c -o $@ $<
