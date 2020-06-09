ROOT_DIR := $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
NRF_SDK_DIR := $(ROOT_DIR)
PCA10056_BLINKY_ARMGCC_DIR := $(NRF_SDK_DIR)/nrf5_sdk/examples/peripheral/blinky/pca10056/blank/armgcc
PCA10056_BLINKY_ARMGCC_ELF := $(PCA10056_BLINKY_ARMGCC_DIR)/_build/nrf52840_xxaa.out

GDB_EXE = "arm-none-eabi-gdb-py"
GCC_EXE = "arm-none-eabi-gcc"

# Defaults to arguments
JLINK_GDB_PORT := 2331
JLINK_TELNET_PORT := 19021

# Argument to various commands
ARG_ELF_FILE ?= $(PCA10056_BLINKY_ARMGCC_ELF)
ARG_GDB_PORT ?= $(JLINK_GDB_PORT)
ARG_TELNET_PORT ?= $(JLINK_TELNET_PORT)

# To get nrf52 SDK to pick up GCC in our $PATH
export GNU_INSTALL_ROOT := 

# Check environment
ifeq (,$(shell which $(GDB_EXE)))
$(error "Couldn't find GDB. This tool can be found here: https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads")
endif

ifeq (,$(shell which JLinkGDBServer))
$(error "Couldn't find `JLinkGDBServer`. This tool can be found here: https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack")
endif

build:
	cd $(PCA10056_BLINKY_ARMGCC_DIR) && $(MAKE) -C .

console:
	JLinkRTTClient -LocalEcho Off -RTTTelnetPort $(ARG_TELNET_PORT)

gdbserver:
	JLinkGDBServer -if swd -device nRF52840_xxAA -speed auto -port $(ARG_GDB_PORT) -RTTTelnetPort $(ARG_TELNET_PORT)

flash:
	$(GDB_EXE) --eval-command="target remote localhost:$(ARG_GDB_PORT)" --ex="mon reset" --ex="load" --ex="mon reset" --se=$(ARG_ELF_FILE)

debug:
	$(GDB_EXE) --eval-command="target remote localhost:$(ARG_GDB_PORT)" --se=$(ELF_FILE)
