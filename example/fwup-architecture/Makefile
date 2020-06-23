.SECONDARY:

PROJECT = fwup-example
BUILD_DIR = build
Q ?= @

CC=arm-none-eabi-gcc
LD=arm-none-eabi-ld
OCPY=arm-none-eabi-objcopy
ODUMP=arm-none-eabi-objdump
MKDIR=mkdir
XXD=xxd
GIT=git
ECHO=@echo
CAT=cat
CRC=crc32
DD=dd

SRCS_BOOT = boot.c
SRCS_APP = app.c \
		   shell/src/shell.c \
		   app_shell_commands.c
SRCS_LOADER = loader.c \
			  dfu.c \
			  shell/src/shell.c \
			  loader_shell_commands.c \
			  $(BUILD_DIR)/app_bin.c
SRCS_SHARED = syscalls.c \
			  shared_memory.c \
			  clock.c \
			  image.c \
			  gpio.c \
			  usart.c

SRCS_BOOT += $(SRCS_SHARED)
SRCS_APP += $(SRCS_SHARED)
SRCS_LOADER += $(SRCS_SHARED)

INCLUDES = . \
		   shell/include
DEFINES += STM32F4

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
	-Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group \
	-Wl,-Map=$(BUILD_DIR)/$(PROJECT).map

LDFLAGS_BOOT = $(LDFLAGS) -T boot.ld
LDFLAGS_LOADER = $(LDFLAGS) -T loader.ld
LDFLAGS_APP = $(LDFLAGS) -T app.ld

OPENCM3_PATH = ./libopencm3
OPENCM3_INCLUDES = $(OPENCM3_PATH)/include
OPENCM3_LIB = $(OPENCM3_PATH)/lib/libopencm3_stm32f4.a

INCLUDES += $(OPENCM3_INCLUDES)
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

$(BUILD_DIR)/$(PROJECT)-%.bin: $(BUILD_DIR)/$(PROJECT)-%.hdr $(BUILD_DIR)/$(PROJECT)-%.data
	$(ECHO) "  CAT		$@"
	$(Q)$(CAT) $^ > $@

$(BUILD_DIR)/$(PROJECT)-%.hdr: $(BUILD_DIR)/$(PROJECT)-%.elf $(BUILD_DIR)/$(PROJECT)-%.crc
	$(ECHO) "  OBJCOPY	$@"
	$(Q)$(OCPY) $(word 1, $^) --dump-section .image_hdr=$@
	$(ECHO) "  DD		$@"
	$(Q)$(DD) if=$(word 2, $^) of=$@ obs=1 seek=4 conv=notrunc 2>1

$(BUILD_DIR)/$(PROJECT)-%.crc: $(BUILD_DIR)/$(PROJECT)-%.data
	$(ECHO) "  CRC		$@"
	$(Q)$(CRC) $^ | $(XXD) -r -p > $@

$(BUILD_DIR)/$(PROJECT)-%.data: $(BUILD_DIR)/$(PROJECT)-%.elf
	$(ECHO) "  OBJCOPY	$@"
	$(Q)$(OCPY) $^ $@ -O binary --remove-section=.image_hdr

$(BUILD_DIR)/app_bin.c: $(BUILD_DIR)/$(PROJECT)-app.bin
	$(ECHO) "  XXD		$@"
	$(Q)$(XXD) -i $< > $@

$(BUILD_DIR)/$(PROJECT)-app.elf: $(SRCS_APP) $(OPENCM3_LIB)
	$(ECHO) "  LD		$@"
	$(Q)$(MKDIR) -p $(BUILD_DIR)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS_APP) $^ -o $@

$(BUILD_DIR)/$(PROJECT)-boot.elf: $(SRCS_BOOT) $(OPENCM3_LIB)
	$(ECHO) "  LD		$@"
	$(Q)$(MKDIR) -p $(BUILD_DIR)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS_BOOT) $^ -o $@

$(BUILD_DIR)/$(PROJECT)-loader.elf: $(SRCS_LOADER) $(OPENCM3_LIB)
	$(ECHO) "  LD		$@"
	$(Q)$(MKDIR) -p $(BUILD_DIR)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS_LOADER) $^ -o $@

$(OPENCM3_PATH):
	$(ECHO) "Libopencm3 not found, cloning it..."
	$(Q)$(GIT) clone https://github.com/libopencm3/libopencm3.git 2>1

$(OPENCM3_LIB): $(OPENCM3_PATH)
	$(ECHO) "Building libopencm3"
	$(Q)$(MAKE) -s -C $(OPENCM3_PATH)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)