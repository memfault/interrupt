PROJECT_NAME     := aes_ctr
ROOT_DIR         := $(abspath .)
BUILD_DIR        := $(ROOT_DIR)/build

COMPILER ?= arm-none-eabi

SRC_APP_FILES += \
	$(ROOT_DIR)/app.c \
	$(ROOT_DIR)/system_app.c \
	
SRC_BOOT_FILE += \
	$(ROOT_DIR)/boot.c \
	$(ROOT_DIR)/system_boot.c \
	$(ROOT_DIR)/Decrypting-EngineAES/aes_ctr.c \
	$(ROOT_DIR)/Decrypting-EngineAES/flash.c \

ASM_SOURCES = $(ROOT_DIR)/startup_app.s

OBJ_APP_FILES = $(patsubst $(ROOT_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_APP_FILES))
OBJ_APP_FILES += $(patsubst $(ROOT_DIR)/%.s,$(BUILD_DIR)/%.o,$(ASM_SOURCES))

OBJ_BOOT_FILES = $(patsubst $(ROOT_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_BOOT_FILE))
OBJ_BOOT_FILES += $(patsubst $(ROOT_DIR)/%.s,$(BUILD_DIR)/%.o,$(ASM_SOURCES))

INCLUDE_PATHS += \
	-I$(ROOT_DIR)/include \
	-I$(ROOT_DIR)/include/CMSIS/Include \
	-I$(ROOT_DIR)/Decrypting-EngineAES \
	
CFLAGS += \
        -fdata-sections \
        -ffunction-sections \
        -Wall \
        -O0 \
        -Og \
        -gdwarf-2 \

CPUFLAGS += \
	    -mcpu=cortex-m4 \
        -mthumb \
        -mfloat-abi=hard \
        -mfpu=fpv4-sp-d16 \

LDFLAGS +=  \
        --specs=rdimon.specs \
        --specs=nano.specs \
        -lm \
        -lnosys \
        -Wl,--gc-sections \
        -Xlinker -print-memory-usage -Xlinker \

LDFLAGS_APP  = $(LDFLAGS) -T app.ld
LDFLAGS_BOOT = $(LDFLAGS) -T boot.ld

TARGET_ELF_BOOT = $(BUILD_DIR)/boot.elf
TARGET_ELF_APP  = $(BUILD_DIR)/app.elf

DEP_DIR = $(BUILD_DIR)/dep
DEP_CFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$(*F).d

SYMBOLS = -DSTM32L4S5xx

all: $(TARGET_ELF_APP) $(TARGET_ELF_BOOT) $(BUILD_DIR)/app.bin $(BUILD_DIR)/boot.bin

-include $(wildcard $(DEP_DIR)/*.d)

$(TARGET_ELF_APP): $(OBJ_APP_FILES)
	@echo "Linking library app"
	@$(COMPILER)-gcc $(CPUFLAGS) $(CFLAGS) $(LDFLAGS_APP) $^ -o $@ -Wl,-lc
	@echo "Generated $(patsubst $(ROOT_DIR)/%,%,$@)"

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@arm-none-eabi-objcopy -O binary -S $< $@	

$(TARGET_ELF_BOOT): $(OBJ_BOOT_FILES)
	@echo "Linking library boot"
	@$(COMPILER)-gcc $(CPUFLAGS) $(CFLAGS) $(LDFLAGS_BOOT) $^ -o $@ -Wl,-lc
	@echo "Generated $(patsubst $(ROOT_DIR)/%,%,$@)"

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@arm-none-eabi-objcopy -O binary -S $< $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(DEP_DIR):
	@mkdir -p $(DEP_DIR)

$(OBJ_APP_FILES): $(SRC_APP_FILES) Makefile
$(OBJ_BOOT_FILES): $(SRC_BOOT_FILES) Makefile

$(BUILD_DIR)/%.o: $(ROOT_DIR)/%.c | $(BUILD_DIR) $(DEP_DIR)
	@echo "Compiling $(patsubst $(ROOT_DIR)/%,%,$<)"
	@mkdir -p $(dir $@)
	@$(COMPILER)-gcc $(DEP_CFLAGS) $(CPUFLAGS) $(CFLAGS) $(INCLUDE_PATHS) $(SYMBOLS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(ROOT_DIR)/%.s | $(BUILD_DIR) $(DEP_DIR)
	@echo "Compiling $(patsubst $(ROOT_DIR)/%,%,$<)"
	@$(COMPILER)-as $(CPUFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

encryption:
	python $(ROOT_DIR)/Encrypting-PythonAES/aes_ctr.py $(BUILD_DIR)/app.bin $(BUILD_DIR)/Cipherapp.bin

flash:
	JLink.exe -device STM32L4S5VI -speed 4000 -si swd -autoconnect 1