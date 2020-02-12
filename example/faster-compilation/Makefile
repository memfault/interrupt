# STM32 Makefile for GNU toolchain and openocd
#
# This Makefile fetches the Cube firmware package from ST's' Github page.
# This includes: CMSIS, STM32 HAL, BSPs, USB drivers and examples.
#
# Usage:
#     make cube         Download and unzip Cube firmware
#     make all          Compile the binary
#
# Original Author:
#   Copyright     2015 Steffen Vogel
#   License       http://www.gnu.org/licenses/gpl.txt GNU Public License
#   Author        Steffen Vogel <post@steffenvogel.de>
#   Link          http://www.steffenvogel.de
#
# Edited to simplify the use case for compiling a single example

TARGET = example

EXAMPLE = Applications/LwIP/LwIP_HTTP_Server_Netconn_RTOS

BOARD           = STM32F429ZI-Nucleo
BOARD_LD_DIR    = STM32F429ZI_NUCLEO_144
BOARD_XX_UPPER  = STM32F4xx_Nucleo_144
BOARD_XX_LOWER  = stm32f4xx_nucleo_144
BSP_BASE        = $(BOARD_XX_LOWER)

MCU_FAMILY    = stm32f4xx
MCU_XX_LOWER  = stm32f429xx
MCU_XX_UPPER  = STM32F429xx
MCU_UPPER     = STM32F429ZI

LDFILE = $(EXAMPLE)/SW4STM32/$(BOARD_LD_DIR)/$(MCU_UPPER)Tx_FLASH.ld

STMCUBE_DIR = stm32cube

# Project Source Files
SRCS = main.c \
       system_$(MCU_FAMILY).c \
       stm32f4xx_it.c \
       stm32f4xx_hal_timebase_tim.c \
       app_ethernet.c \
       ethernetif.c \
       httpserver-netconn.c \
       extra.c

# BSP Source Files
SRCS += $(BSP_BASE).c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma2d.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_eth.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c_ex.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_ltdc.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sdram.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c \
        $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_fmc.c \

BSP_DIR    = $(STMCUBE_DIR)/Drivers/BSP/$(BOARD_XX_UPPER)
HAL_DIR    = $(STMCUBE_DIR)/Drivers/STM32F4xx_HAL_Driver
CMSIS_DIR  = $(STMCUBE_DIR)/Drivers/CMSIS

DEV_DIR    = $(CMSIS_DIR)/Device/ST/STM32F4xx

STM32CUBE_URL   = https://github.com/STMicroelectronics/STM32CubeF4/archive/v1.24.1.zip

###############################################################################
# Toolchain

PREFIX     = arm-none-eabi
CC         = $(PREFIX)-gcc
AR         = $(PREFIX)-ar
OBJCOPY    = $(PREFIX)-objcopy
OBJDUMP    = $(PREFIX)-objdump
SIZE       = $(PREFIX)-size
GDB        = $(PREFIX)-gdb

###############################################################################
# Options

# Defines
DEFS       = -D$(MCU_XX_UPPER) -DUSE_HAL_DRIVER -DUSE_STM32F4XX_NUCLEO_144

# Pre-compiled headers search path
INCS = -Iobj/src

# Include search paths (-I)
INCS += -Isrc \
        -I$(BSP_DIR) \
        -I$(CMSIS_DIR)/Include \
        -I$(DEV_DIR)/Include \
        -I$(HAL_DIR)/Inc \
        -I$(STMCUBE_DIR)/Drivers/BSP/Components/Common \
        -I$(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/include/ \
        -I$(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/system \
        -I$(STMCUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
        -I$(STMCUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/include \
        -I$(STMCUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS \
        -I$(STMCUBE_DIR)/Utilities \
        -I$(STMCUBE_DIR)/Utilities/Log \

SRCS += $(STMCUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/croutine.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/list.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/queue.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/tasks.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/timers.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/api/api_lib.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/api/api_msg.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/api/err.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/api/netbuf.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/api/netdb.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/api/netifapi.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/api/sockets.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/api/tcpip.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/apps/httpd/fs.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/apps/httpd/httpd.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/ipv4/autoip.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/ipv4/dhcp.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/ipv4/etharp.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/ipv4/icmp.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/ipv4/igmp.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/ipv4/ip4.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/ipv4/ip4_addr.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/ipv4/ip4_frag.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/def.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/dns.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/inet_chksum.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/init.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/ip.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/mem.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/memp.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/netif.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/pbuf.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/raw.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/stats.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/sys.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/tcp.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/tcp_in.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/tcp_out.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/timeouts.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/core/udp.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/src/netif/ethernet.c \
        $(STMCUBE_DIR)/Middlewares/Third_Party/LwIP/system/OS/sys_arch.c \

# Single pre-compiled header
PCHS       = src/precompiled.h

# Library search paths
LIBS       = -L$(CMSIS_DIR)/Lib

# Compiler flags
CFLAGS     = -Wall -g -std=c99 -Os
CFLAGS    += -mlittle-endian -mcpu=cortex-m4 -march=armv7e-m -mthumb
CFLAGS    += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS    += -ffunction-sections -fdata-sections
CFLAGS    += -Wno-implicit-function-declaration
CFLAGS    += $(INCS) $(DEFS)

# Linker flags
LDFLAGS    = -Wl,--gc-sections -Wl,-Map=$(TARGET).map $(LIBS) -T$(MCU_XX_LOWER).ld

LDFLAGS   += --specs=rdimon.specs

# Source search paths
VPATH      = ./src
VPATH     += $(BSP_DIR)
VPATH     += $(HAL_DIR)/Src
VPATH     += $(DEV_DIR)/Source/

OBJS       = $(addprefix obj/,$(SRCS:.c=.o))
DEPS       = $(addprefix dep/,$(SRCS:.c=.d))
PCHS_SRCS  = $(addprefix obj/,$(PCHS))
PCHS_OBJS  = $(addprefix obj/,$(PCHS:.h=.h.gch))

# Prettify output
V = 0
ifeq ($V, 0)
  Q = @
  P = > /dev/null
endif

###################################################

.PHONY: all dirs program debug template clean

all: $(TARGET).bin

-include $(DEPS)

dirs: dep obj stm32cube
dep obj src:
	@echo "[MKDIR]   $@"
	$Qmkdir -p $@

obj/%.h.gch : $(PCHS) | dirs
	@echo "[CCH]      $(notdir $<)"
	@mkdir -p $(@D)
	$Q$(CC) $(CFLAGS) -c -o $@ $< -MMD -MF dep/$(*F).d

obj/%.o : %.c | dirs $(PCHS_OBJS)
	@echo "[CC]      $(notdir $<)"
	@mkdir -p $(@D)
	$Q$(CC) $(CFLAGS) -include $(PCHS) -c -o $@ $< -MMD -MF dep/$(*F).d

$(TARGET).elf: $(OBJS)
	@echo "[LD]      $(TARGET).elf"
	$Q$(CC) $(CFLAGS) $(LDFLAGS) src/startup_$(MCU_XX_LOWER).s $^ -o $@
	@echo "[OBJDUMP] $(TARGET).lst"
	$Q$(OBJDUMP) -St $(TARGET).elf >$(TARGET).lst
	@echo "[SIZE]    $(TARGET).elf"
	$(SIZE) $(TARGET).elf

$(TARGET).bin: $(TARGET).elf
	@echo "[OBJCOPY] $(TARGET).bin"
	$Q$(OBJCOPY) -O binary $< $@

stm32cube:
	rm -rf $(STMCUBE_DIR)
	wget -O /tmp/cube.zip $(STM32CUBE_URL)
	unzip -q /tmp/cube.zip
	mv STM32Cube* $(STMCUBE_DIR)
	chmod -R u+w $(STMCUBE_DIR)
	rm -f /tmp/cube.zip

clean:
	@echo "[RM]      $(TARGET).bin"; rm -f $(TARGET).bin
	@echo "[RM]      $(TARGET).elf"; rm -f $(TARGET).elf
	@echo "[RM]      $(TARGET).map"; rm -f $(TARGET).map
	@echo "[RM]      $(TARGET).lst"; rm -f $(TARGET).lst
	@echo "[RMDIR]   dep"          ; rm -fr dep
	@echo "[RMDIR]   obj"          ; rm -fr obj

