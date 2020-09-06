.SECONDARY:

PROJECT = fwup-example
BUILD_DIR = build
Q ?= @

CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OCPY = arm-none-eabi-objcopy
MKDIR = mkdir
GIT=git
ECHO=@echo
CAT=cat
PYTHON ?= python
PK_PEM_PATH ?= ./private.pem


GIT_SHA := \"$(shell $(GIT) rev-parse --short HEAD)\"

SRCS_BOOT = boot.c

SRCS_APP = \
  app.c \
  app_shell_commands.c \
  shell/src/shell.c \

SRCS_LOADER = \
  crc32.c \
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

APP_BIN = \
  $(BUILD_DIR)/app_bin.o

SRCS_BOOT += $(SRCS_SHARED)
SRCS_APP += $(SRCS_SHARED)
SRCS_LOADER += $(SRCS_SHARED)

INCLUDES = . \
		   shell/include
DEFINES += STM32F4 \
		   GIT_SHA=$(GIT_SHA)

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

INCLUDES += $(OPENCM3_INCLUDES)
INCLUDES += $(CIFRA_INCLUDES)
INCLUDES += $(MICROECC_INCLUDES)
CFLAGS += $(foreach i,$(INCLUDES),-I$(i))
CFLAGS += $(foreach d,$(DEFINES),-D$(d))
LDSCRIPT = stm32f429i-discovery.ld

.PHONY: all
all: $(BUILD_DIR)/$(PROJECT).bin

$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT)-boot.bin  $(BUILD_DIR)/$(PROJECT)-loader.bin $(BUILD_DIR)/$(PROJECT)-app.bin
	$(ECHO) "  CAT		$@"
	$(Q)$(CAT) $^ > $@

$(BUILD_DIR)/$(PROJECT)-boot.bin: $(BUILD_DIR)/$(PROJECT)-boot.elf
	$(ECHO) "  OBJCOPY	$@"
	$(Q)$(OCPY) $^ $@ -O binary

$(BUILD_DIR)/$(PROJECT)-%.bin: $(BUILD_DIR)/$(PROJECT)-%.elf $(PK_PEM_PATH)
	$(ECHO) "  OBJCOPY	$@"
	$(Q)$(OCPY) $< $@ -O binary
	$(ECHO) "  PATCH_IMAGE	$@"
	$(Q)$(PYTHON) patch_image_header.py $@ $(PK_PEM_PATH) > /dev/null

# https://interrupt.memfault.com/blog/gnu-binutils#converting-binaries-into-an-object-o-file
$(BUILD_DIR)/app_bin.o: $(BUILD_DIR)/$(PROJECT)-app.bin
	$(Q)$(OCPY) -I binary -O elf32-littlearm -B arm --add-section app_binary=$< $< $@

$(BUILD_DIR)/$(PROJECT)-app.elf: $(SRCS_APP) $(OPENCM3_LIB)
	$(ECHO) "  LD		$@"
	$(Q)$(MKDIR) -p $(BUILD_DIR)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS_APP) $^ -o $@

$(BUILD_DIR)/$(PROJECT)-boot.elf: $(SRCS_BOOT) $(OPENCM3_LIB)
	$(ECHO) "  LD		$@"
	$(Q)$(MKDIR) -p $(BUILD_DIR)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS_BOOT) $^ -o $@

$(BUILD_DIR)/$(PROJECT)-loader.elf: $(SRCS_LOADER) $(OPENCM3_LIB) $(APP_BIN)
	$(ECHO) "  LD		$@"
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

$(OPENCM3_LIB): $(OPENCM3_PATH)
	$(ECHO) "Building libopencm3"
	$(Q)$(MAKE) -s -C $(OPENCM3_PATH)

.PHONY: clean
clean:
	$(ECHO) "  CLEAN		rm -rf $(BUILD_DIR)"
	$(Q)rm -rf $(BUILD_DIR)
