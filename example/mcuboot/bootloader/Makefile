ROOT_DIR := $(abspath ..)
Q ?= @

COMPILER ?= arm-none-eabi-gcc

COMMON_DIR = $(ROOT_DIR)/common
COMMON_SRC_DIR = $(COMMON_DIR)/src

BOOTLOADER_DIR = $(ROOT_DIR)/bootloader
BUILD_DIR  := $(BOOTLOADER_DIR)/build
PROJECT_SRC_DIR = $(BOOTLOADER_DIR)/src

SRC_FILES += \
  $(PROJECT_SRC_DIR)/log_port.c \
  $(PROJECT_SRC_DIR)/main.c \
  $(PROJECT_SRC_DIR)/minimal_nrf52_uart.c \
  $(PROJECT_SRC_DIR)/minimal_nrf52_flash.c \
  $(PROJECT_SRC_DIR)/shell.c \
  $(PROJECT_SRC_DIR)/shell_commands.c \
  $(PROJECT_SRC_DIR)/shell_port.c \
  $(PROJECT_SRC_DIR)/startup.c \

EXTERNAL_DIR := $(ROOT_DIR)/external

MCUBOOT_DIR := $(EXTERNAL_DIR)/mcuboot/boot/bootutil
MCUBOOT_SRC_DIR := $(MCUBOOT_DIR)/src
MCUBOOT_INC_DIR := $(MCUBOOT_DIR)/include

MCUBOOT_SRC_FILES += \
  $(MCUBOOT_SRC_DIR)/boot_record.c \
  $(MCUBOOT_SRC_DIR)/bootutil_misc.c \
  $(MCUBOOT_SRC_DIR)/caps.c \
  $(MCUBOOT_SRC_DIR)/encrypted.c \
  $(MCUBOOT_SRC_DIR)/fault_injection_hardening.c \
  $(MCUBOOT_SRC_DIR)/fault_injection_hardening_delay_rng_mbedtls.c \
  $(MCUBOOT_SRC_DIR)/image_ec.c \
  $(MCUBOOT_SRC_DIR)/image_ec256.c \
  $(MCUBOOT_SRC_DIR)/image_ed25519.c \
  $(MCUBOOT_SRC_DIR)/image_rsa.c \
  $(MCUBOOT_SRC_DIR)/image_validate.c \
  $(MCUBOOT_SRC_DIR)/loader.c \
  $(MCUBOOT_SRC_DIR)/swap_misc.c \
  $(MCUBOOT_SRC_DIR)/swap_move.c \
  $(MCUBOOT_SRC_DIR)/swap_scratch.c \
  $(MCUBOOT_SRC_DIR)/tlv.c

# Also need mcuboot_config/ on path
MCUBOOT_INC_PATHS := -I$(MCUBOOT_INC_DIR)

TINYCRYPT_DIR := $(ROOT_DIR)/external/mcuboot/ext/tinycrypt/lib
TINYCRYPT_SRC_DIR := $(TINYCRYPT_DIR)/source
TINYCRYPT_INC_DIR := $(TINYCRYPT_DIR)/include

TINYCRYPT_SRC_FILES += \
  $(TINYCRYPT_SRC_DIR)/aes_decrypt.c \
  $(TINYCRYPT_SRC_DIR)/aes_encrypt.c \
  $(TINYCRYPT_SRC_DIR)/cbc_mode.c \
  $(TINYCRYPT_SRC_DIR)/ccm_mode.c \
  $(TINYCRYPT_SRC_DIR)/cmac_mode.c \
  $(TINYCRYPT_SRC_DIR)/ctr_mode.c \
  $(TINYCRYPT_SRC_DIR)/ctr_prng.c \
  $(TINYCRYPT_SRC_DIR)/hmac.c \
  $(TINYCRYPT_SRC_DIR)/hmac_prng.c \
  $(TINYCRYPT_SRC_DIR)/sha256.c \
  $(TINYCRYPT_SRC_DIR)/utils.c

TINYCRYPT_INC_PATHS := -I$(TINYCRYPT_INC_DIR)

SRC_FILES += $(TINYCRYPT_SRC_FILES)
INCLUDE_PATHS += $(TINYCRYPT_INC_PATHS)

SRC_FILES += $(COMMON_SRC_DIR)/mcuboot_port.c

SRC_FILES += $(MCUBOOT_SRC_FILES)
INCLUDE_PATHS += $(MCUBOOT_INC_PATHS)

INCLUDE_PATHS += -I$(COMMON_DIR)/include

OBJ_FILES := $(patsubst $(ROOT_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

INCLUDE_PATHS += \
  -I$(BOOTLOADER_DIR)/include

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

TARGET ?= nrf52-bootloader
LDSCRIPT = $(ROOT_DIR)/bootloader/$(TARGET).ld
TARGET_ELF = $(BUILD_DIR)/$(TARGET).elf

LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,-Map,$(BUILD_DIR)/$(TARGET).map
LDFLAGS += --specs=nano.specs

DEP_DIR = $(BUILD_DIR)/dep
DEP_CFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$(*F).d

all: $(TARGET_ELF)

flash: $(TARGET_ELF)
	arm-none-eabi-gdb-py --eval-command="target remote localhost:2331"  --ex="mon reset" --ex="load" --ex="mon reset"  --se=$(TARGET_ELF)

-include $(wildcard $(DEP_DIR)/*.d)

clean:
	rm -rf $(BUILD_DIR)

$(TARGET_ELF): $(OBJ_FILES) $(LDSCRIPT)
	@echo "Linking library"
	@arm-none-eabi-gcc $(CFLAGS) $(LDFLAGS) $(OBJ_FILES) -o $@ -Wl,-lc -Wl,-lgcc
	@echo "Generated $(patsubst $(ROOT_DIR)/%,%,$@)"

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(DEP_DIR):
	@mkdir -p $(DEP_DIR)

$(OBJ_FILES): Makefile

# MCUboot should really be included as a submodule in a real project
# Let's do a cheap hack and clone it here instead
$(MCUBOOT_DIR):
	mkdir -p $(EXTERNAL_DIR)
	git clone --recurse-submodules=. https://github.com/mcu-tools/mcuboot.git $(EXTERNAL_DIR)/mcuboot
	cd $(EXTERNAL_DIR)/mcuboot && git checkout e512181 && git submodule update --recursive

$(SRC_FILES): $(MCUBOOT_DIR)


$(BUILD_DIR)/%.o: $(ROOT_DIR)/%.c | $(BUILD_DIR) $(DEP_DIR) $(FREERTOS_PORT_ROOT)
	@echo "Compiling $(patsubst $(ROOT_DIR)/%,%,$<)"
	@mkdir -p $(dir $@)
	$(Q) arm-none-eabi-gcc $(DEP_CFLAGS) $(CFLAGS) $(INCLUDE_PATHS) -c -o $@ $<
