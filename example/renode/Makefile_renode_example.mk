PROJECT = renode-example
BUILD_DIR = bin

CFILES = renode-example.c

DEVICE=stm32f407vgt6
OOCD_FILE = board/stm32f4discovery.cfg

OPENCM3_DIR=libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
