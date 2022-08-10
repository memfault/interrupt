.SECONDARY:

PROJECT = fwup-delta
BUILD_DIR = build
Q ?= @

CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OCPY = arm-none-eabi-objcopy
MKDIR = mkdir
GIT=git
ECHO=@echo
CAT=cat
PYTHON ?= python3
PK_PEM_PATH ?= ./private.pem
JDIFF ?= jdiff


GIT_SHA := \"$(shell $(GIT) rev-parse --short HEAD)\"

SRCS_BOOT = boot.c

SRCS_APP = \
  app.c \
  app_shell_commands.c \
  shell/src/shell.c \

SRCS_LOADER = \
  crc32.c \
  simple_fileio.c \
  dfu.c \
  loader.c \
  loader_shell_commands.c \
  shell/src/shell.c

SRCS_SHARED = \
  clock.c \
  crc32.c \
  gpio.c \
  image.c \
  shared_memory.c \
  syscalls.c \
  usart.c \
  $(MICROECC_SOURCES) \
  $(CIFRA_SOURCES)

PATCH_FILE = 0001-add-app-print.patch

PATCH_BIN = \
  $(BUILD_DIR)/patch_bin.o

SRCS_BOOT += $(SRCS_SHARED)
SRCS_APP += $(SRCS_SHARED)
SRCS_LOADER += $(SRCS_SHARED)

INCLUDES = . \
		   shell/include
DEFINES += STM32F4 \
	GIT_SHA=$(GIT_SHA) \
	FULL_FAT_ASSERT

CFLAGS += \
  -mcpu=cortex-m4 \
  -mfloat-abi=hard \
  -mfpu=fpv4-sp-d16 \
  -mthumb \
  -Wall \
  -Werror \
  -std=gnu11 \
  -O0 \
  -g \
  -ffunction-sections \
  -fdata-sections

LDFLAGS += \
  -static \
  -nostartfiles \
  -specs=nano.specs \
  -Wl,--gc-sections \
  -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group \
  -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map

LDFLAGS_BOOT = $(LDFLAGS) -T boot.ld -DBOOT
LDFLAGS_LOADER = $(LDFLAGS) -T loader.ld -DLOADER
LDFLAGS_APP = $(LDFLAGS) -T app.ld -DAPP

OPENCM3_PATH = ./libopencm3
OPENCM3_INCLUDES = $(OPENCM3_PATH)/include
OPENCM3_LIB = $(OPENCM3_PATH)/lib/libopencm3_stm32f4.a

CIFRA_PATH = ./cifra/src
CIFRA_SOURCES = $(CIFRA_PATH)/sha256.c \
				$(CIFRA_PATH)/blockwise.c
CIFRA_INCLUDES = $(CIFRA_PATH) $(CIFRA_PATH)/ext

MICROECC_PATH = ./micro-ecc
MICROECC_SOURCES = $(MICROECC_PATH)/uECC.c
MICROECC_INCLUDES = $(MICROECC_PATH)

JANPATCH_PATH = ./janpatch
JANPATCH_INCLUDES = $(JANPATCH_PATH)

INCLUDES += $(OPENCM3_INCLUDES)
INCLUDES += $(CIFRA_INCLUDES)
INCLUDES += $(MICROECC_INCLUDES)
INCLUDES += $(JANPATCH_INCLUDES)
CFLAGS += $(foreach i,$(INCLUDES),-I$(i))
CFLAGS += $(foreach d,$(DEFINES),-D$(d))
LDSCRIPT = stm32f429i-discovery.ld

.PHONY: all
all: $(BUILD_DIR)/$(PROJECT).bin

$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT)-boot.bin  $(BUILD_DIR)/$(PROJECT)-loader.bin $(BUILD_DIR)/$(PROJECT)-app.bin
	$(ECHO) "  CAT	  $@"
	$(Q)$(CAT) $^ > $@

$(BUILD_DIR)/$(PROJECT)-boot.bin: $(BUILD_DIR)/$(PROJECT)-boot.elf
	$(ECHO) "  OCPY	  $@"
	$(Q)$(OCPY) $^ $@ -O binary

$(BUILD_DIR)/$(PROJECT)-%.bin: $(BUILD_DIR)/$(PROJECT)-%.elf $(PK_PEM_PATH)
	$(ECHO) "  OCPY	  $@"
	$(Q)$(OCPY) $< $@ -O binary
	$(ECHO) "  PATCH	  $@"
	$(Q)$(PYTHON) patch_image_header.py $@ $(PK_PEM_PATH) > /dev/null

# https://interrupt.memfault.com/blog/gnu-binutils#converting-binaries-into-an-object-o-file
$(BUILD_DIR)/patch_bin.o: $(BUILD_DIR)/patch.bin
	$(Q)$(OCPY) -I binary -O elf32-littlearm -B arm --add-section app_patch=$< $< $@

$(BUILD_DIR)/patch.bin: $(BUILD_DIR)/$(PROJECT)-app.bin $(BUILD_DIR)/$(PROJECT)-app-patched.bin
	$(ECHO) "  JDIFF	  $@"
	$(Q)$(JDIFF) $^ $@ &>/dev/null # prints useless warning from emscripten

$(BUILD_DIR)/$(PROJECT)-app.elf: $(SRCS_APP) $(OPENCM3_LIB)
	$(ECHO) "  LD	  $@"
	$(Q)$(MKDIR) -p $(BUILD_DIR)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS_APP) $^ -o $@

$(BUILD_DIR)/$(PROJECT)-app-patched.elf: $(SRCS_APP) $(PATCH_FILE) $(OPENCM3_LIB)
	$(ECHO) "  PATCH	  $@"
	$(Q)$(GIT) apply $(PATCH_FILE)
	$(ECHO) "  LD	  $@"
	$(Q)$(MKDIR) -p $(BUILD_DIR)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS_APP) $(SRCS_APP) $(OPENCM3_LIB) -o $@
	$(Q)$(GIT) apply -R $(PATCH_FILE)

$(BUILD_DIR)/$(PROJECT)-boot.elf: $(SRCS_BOOT) $(OPENCM3_LIB)
	$(ECHO) "  LD	  $@"
	$(Q)$(MKDIR) -p $(BUILD_DIR)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS_BOOT) $^ -o $@

$(BUILD_DIR)/$(PROJECT)-loader.elf: $(SRCS_LOADER) $(OPENCM3_LIB) $(PATCH_BIN) | $(JANPATCH_INCLUDES)
	$(ECHO) "  LD	  $@"
	$(Q)$(MKDIR) -p $(BUILD_DIR)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS_LOADER) $^ -o $@

$(OPENCM3_PATH):
	$(ECHO) "Libopencm3 not found, cloning it..."
	$(Q)$(GIT) clone https://github.com/libopencm3/libopencm3.git 2>1

$(CIFRA_PATH)/%.c:
	$(ECHO) "cifra not found, cloning it..."
	$(Q)$(GIT) clone https://github.com/ctz/cifra 2>1

$(MICROECC_PATH)/%.c:
	$(ECHO) "microecc not found, cloning it..."
	$(Q)$(GIT) clone https://github.com/kmackay/micro-ecc 2>1

$(JANPATCH_PATH):
	$(ECHO) "janpatch not found, cloning it..."
	$(Q)$(GIT) clone https://github.com/janjongboom/janpatch

$(OPENCM3_LIB): $(OPENCM3_PATH)
	$(ECHO) "  BUILD	  libopencm3"
	$(Q)$(MAKE) -s -C $(OPENCM3_PATH) 1>/dev/null


.PHONY: clean
clean:
	$(ECHO) "  CLEAN	  $(BUILD_DIR)"
	$(Q)rm -rf $(BUILD_DIR)
