ROOT_DIR := $(abspath ..)
Q ?= @

IMGTOOL_PY := $(ROOT_DIR)/external/mcuboot/scripts/imgtool.py

COMPILER ?= arm-none-eabi-gcc

COMMON_DIR = $(ROOT_DIR)/common
COMMON_SRC_DIR = $(COMMON_DIR)/src

APPLICATION_DIR = $(ROOT_DIR)/application
BUILD_DIR  := $(APPLICATION_DIR)/build
PROJECT_SRC_DIR = $(APPLICATION_DIR)/src

SRC_FILES += \
  $(PROJECT_SRC_DIR)/log_port.c \
  $(PROJECT_SRC_DIR)/main.c \
  $(PROJECT_SRC_DIR)/minimal_nrf52_uart.c \
  $(PROJECT_SRC_DIR)/minimal_nrf52_flash.c \
  $(PROJECT_SRC_DIR)/shell.c \
  $(PROJECT_SRC_DIR)/shell_commands.c \
  $(PROJECT_SRC_DIR)/shell_port.c \
  $(PROJECT_SRC_DIR)/startup.c \


MCUBOOT_DIR := $(ROOT_DIR)/external/mcuboot/boot/bootutil
MCUBOOT_SRC_DIR := $(MCUBOOT_DIR)/src
MCUBOOT_INC_DIR := $(MCUBOOT_DIR)/include
SRC_FILES += \
    $(MCUBOOT_SRC_DIR)/bootutil_misc.c \
    $(COMMON_SRC_DIR)/mcuboot_port.c

INCLUDE_PATHS += \
  -I$(MCUBOOT_INC_DIR) \
  -I$(ROOT_DIR)/bootloader/config


OBJ_FILES := $(patsubst $(ROOT_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

INCLUDE_PATHS += \
  -I$(APPLICATION_DIR)/include \
  -I$(COMMON_DIR)/include

CFLAGS += \
  -mcpu=cortex-m4 \
  -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
  -mthumb \
  -g3 \
  -nostdlib \
  -ffunction-sections -fdata-sections \
  -Wall \
  -Werror \
  -fdebug-prefix-map=$(ROOT_DIR)=. \
  -Os

TARGET ?= nrf52-app
LDSCRIPT = $(APPLICATION_DIR)/$(TARGET).ld

TARGET_ELF = $(BUILD_DIR)/$(TARGET).elf

TARGET_BIN = $(BUILD_DIR)/$(TARGET)-no-header.bin
MCUBOOT_IMG0_BIN = $(BUILD_DIR)/$(TARGET).bin

LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,-Map,$(BUILD_DIR)/$(TARGET).map
LDFLAGS += --specs=nano.specs

DEP_DIR = $(BUILD_DIR)/dep
DEP_CFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$(*F).d

all: $(MCUBOOT_IMG0_BIN)

flash: $(MCUBOOT_IMG0_BIN)
	arm-none-eabi-gdb-py --eval-command="target remote localhost:2331"  \
	    --ex="mon reset" --ex="restore $(MCUBOOT_IMG0_BIN) binary 0x8000" --ex="mon reset"  --se=$(TARGET_ELF)

-include $(wildcard $(DEP_DIR)/*.d)

clean:
	rm -rf $(BUILD_DIR)

$(MCUBOOT_IMG0_BIN): $(TARGET_BIN)
	@python $(IMGTOOL_PY) sign --header-size 0x200 --align 8 -S 131072 -v 1.0.0 --pad-header $(TARGET_BIN) $(MCUBOOT_IMG0_BIN)
	@echo "Generated $(patsubst $(ROOT_DIR)/%,%,$@)"

$(TARGET_BIN): $(TARGET_ELF)
	@arm-none-eabi-objcopy $< $@ -O binary
	@echo "Generated $(patsubst $(ROOT_DIR)/%,%,$@)"

$(TARGET_ELF): $(OBJ_FILES) $(LDSCRIPT)
	@echo "Linking library"
	@arm-none-eabi-gcc $(CFLAGS) $(LDFLAGS) $(OBJ_FILES) -o $@ -Wl,-lc -Wl,-lgcc
	@echo "Generated $(patsubst $(ROOT_DIR)/%,%,$@)"

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(DEP_DIR):
	@mkdir -p $(DEP_DIR)

$(OBJ_FILES): Makefile

$(BUILD_DIR)/%.o: $(ROOT_DIR)/%.c | $(BUILD_DIR) $(DEP_DIR) $(FREERTOS_PORT_ROOT)
	@echo "Compiling $(patsubst $(ROOT_DIR)/%,%,$<)"
	@mkdir -p $(dir $@)
	$(Q) arm-none-eabi-gcc $(DEP_CFLAGS) $(CFLAGS) $(INCLUDE_PATHS) -c -o $@ $<
