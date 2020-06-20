PROJECT = fwup-example
BUILD_DIR = build

CC=arm-none-eabi-gcc
LD=arm-none-eabi-ld
OCPY=arm-none-eabi-objcopy
ODUMP=arm-none-eabi-objdump
SZ=arm-none-eabi-size
MKDIR=mkdir

SRCS_BOOT = boot.c
SRCS_APP = app.c
SRCS_LOADER = loader.c
SRCS_SHARED = syscalls.c \
			  shared_memory.c \
			  clock.c \
			  gpio.c \
			  usart.c

SRCS_BOOT += $(SRCS_SHARED)
SRCS_APP += $(SRCS_SHARED)
SRCS_LOADER += $(SRCS_SHARED)

INCLUDES = .
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
	cat $^ > $@

$(BUILD_DIR)/$(PROJECT)-%.bin: $(BUILD_DIR)/$(PROJECT)-%.elf
	$(OCPY) $< $@ -O binary
	$(SZ) $<

$(BUILD_DIR)/$(PROJECT)-app.elf: $(SRCS_APP) $(OPENCM3_LIB)
	$(MKDIR) -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS_APP) $^ -o $@

$(BUILD_DIR)/$(PROJECT)-boot.elf: $(SRCS_BOOT) $(OPENCM3_LIB)
	$(MKDIR) -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS_BOOT) $^ -o $@

$(BUILD_DIR)/$(PROJECT)-loader.elf: $(SRCS_LOADER) $(OPENCM3_LIB)
	$(MKDIR) -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS_LOADER) $^ -o $@

$(OPENCM3_LIB): 
	$(MAKE) -C $(OPENCM3_PATH)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)