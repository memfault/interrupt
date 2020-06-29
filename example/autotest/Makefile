.SECONDARY:

PROJECT = renode-example
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

GIT_SHA := \"$(shell $(GIT) rev-parse --short HEAD)\"


SRCS_APP = \
  app.c \
  app_shell_commands.c \
  shell/src/shell.c \
  clock.c \
  gpio.c \
  usart.c \
  syscalls.c \

INCLUDES = \
	. \
	shell/include


MEMFAULT_SDK_ROOT = memfault-firmware-sdk
MEMFAULT_CORE_SRC_DIR = $(MEMFAULT_SDK_ROOT)/components/core/src
MEMFAULT_UTIL_SRC_DIR = $(MEMFAULT_SDK_ROOT)/components/util/src
MEMFAULT_PANICS_SRC_DIR = $(MEMFAULT_SDK_ROOT)/components/panics/src

SRCS_APP += \
  $(MEMFAULT_CORE_SRC_DIR)/arch_arm_cortex_m.c \
  $(MEMFAULT_CORE_SRC_DIR)/memfault_build_id.c \
  $(MEMFAULT_CORE_SRC_DIR)/memfault_core_utils.c \
  $(MEMFAULT_CORE_SRC_DIR)/memfault_data_packetizer.c \
  $(MEMFAULT_CORE_SRC_DIR)/memfault_event_storage.c \
  $(MEMFAULT_CORE_SRC_DIR)/memfault_log.c \
  $(MEMFAULT_CORE_SRC_DIR)/memfault_ram_reboot_info_tracking.c \
  $(MEMFAULT_CORE_SRC_DIR)/memfault_reboot_tracking_serializer.c \
  $(MEMFAULT_CORE_SRC_DIR)/memfault_sdk_assert.c \
  $(MEMFAULT_CORE_SRC_DIR)/memfault_serializer_helper.c \
  $(MEMFAULT_CORE_SRC_DIR)/memfault_trace_event.c \
  $(MEMFAULT_UTIL_SRC_DIR)/memfault_chunk_transport.c \
  $(MEMFAULT_UTIL_SRC_DIR)/memfault_circular_buffer.c \
  $(MEMFAULT_UTIL_SRC_DIR)/memfault_crc16_ccitt.c \
  $(MEMFAULT_UTIL_SRC_DIR)/memfault_minimal_cbor.c \
  $(MEMFAULT_UTIL_SRC_DIR)/memfault_varint.c \
  $(MEMFAULT_PANICS_SRC_DIR)/memfault_coredump.c \
  $(MEMFAULT_PANICS_SRC_DIR)/memfault_coredump_regions_armv7.c \
  $(MEMFAULT_PANICS_SRC_DIR)/memfault_fault_handling_arm.c \
  $(MEMFAULT_SDK_ROOT)/ports/panics/src/memfault_platform_ram_backed_coredump.c

INCLUDES += \
   $(MEMFAULT_SDK_ROOT)/components/core/include \
   $(MEMFAULT_SDK_ROOT)/components/util/include \
   $(MEMFAULT_SDK_ROOT)/components/panics/include

DEFINES += \
	STM32F4 \
	GIT_SHA=$(GIT_SHA) \
  MEMFAULT_EXC_HANDLER_NMI=memfault_nmi_handler \
  MEMFAULT_EXC_HANDLER_HARD_FAULT=memfault_hard_fault_handler \
  MEMFAULT_EXC_HANDLER_MEMORY_MANAGEMENT=memfault_mem_manage_handler \
  MEMFAULT_EXC_HANDLER_BUS_FAULT=memfault_bus_fault_handler \
  MEMFAULT_EXC_HANDLER_USAGE_FAULT=memfault_usage_fault_handler \


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

LDFLAGS_APP = $(LDFLAGS) -T stm32f429i-discovery.ld

OPENCM3_PATH = ./libopencm3
OPENCM3_INCLUDES = $(OPENCM3_PATH)/include
OPENCM3_LIB = $(OPENCM3_PATH)/lib/libopencm3_stm32f4.a

INCLUDES += $(OPENCM3_INCLUDES)
CFLAGS += $(foreach i,$(INCLUDES),-I$(i))
CFLAGS += $(foreach d,$(DEFINES),-D$(d))
LDSCRIPT = stm32f429i-discovery.ld

$(info $(CFLAGS))

.PHONY: all
all: $(BUILD_DIR)/$(PROJECT).elf

$(BUILD_DIR)/$(PROJECT).elf: $(SRCS_APP) $(OPENCM3_LIB) | $(MEMFAULT_SDK_ROOT)
	$(ECHO) "  LD		$@"
	$(Q)$(MKDIR) -p $(BUILD_DIR)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS_APP) $^ -o $@

$(OPENCM3_PATH):
	$(ECHO) "Libopencm3 not found, cloning it..."
	$(Q)$(GIT) clone https://github.com/libopencm3/libopencm3.git 2>1

$(MEMFAULT_SDK_ROOT):
	$(ECHO) "memfault-firmware-sdk not found, cloning it..."
	$(Q)$(GIT) clone https://github.com/memfault/memfault-firmware-sdk.git 2>1

$(OPENCM3_LIB): $(OPENCM3_PATH)
	$(ECHO) "Building libopencm3"
	$(Q)$(MAKE) -s -C $(OPENCM3_PATH) TARGETS=stm32/f4

.PHONY: clean
clean:
	$(ECHO) "  CLEAN		rm -rf $(BUILD_DIR)"
	$(Q)rm -rf $(BUILD_DIR)
