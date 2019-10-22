BUILDDIR = build

ifeq ($(USE_CLANG),1)
CLANG_COMPILER ?= clang
CC = $(CLANG_COMPILER)

# Only compile the code when using clang (don't try and link)
COMPILER_SPECIFIC_CFLAGS = -c
else

GCC_COMPILER ?= arm-none-eabi-gcc
GCC_VERSION := $(strip $(shell $(GCC_COMPILER) -dumpversion))
EXPECTED_GCC_VERSION := 8.3.1
ifneq ($(GCC_VERSION),$(EXPECTED_GCC_VERSION))
$(error Examples were all compiled against $(EXPECTED_GCC_VERSION). You are using $(GCC_VERSION))
endif
CC = $(GCC_COMPILER)

LDFLAGS = \
  -Map=$(BUILDDIR)/output.map \
  $(EXTRA_LDFLAGS) \
  -lgcc

COMPILER_SPECIFIC_CFLAGS = \
  -mthumb \
  -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
  -mcpu=cortex-m4 \
  -o $(BUILDDIR)/examples.elf

endif

SRCS = \
  float_promotion.c \
  main.c \
  newlib_stubs.c \
  short_enum.c \
  simple_for_loop.c  \
  simple_math.c \
  stack_usage.c \
  tentative_global.c \

EXTRA_LDFLAGS ?=
EXTRA_CFLAGS ?=

CFLAGS = \
  -nostdlib \
  -Os \
  $(EXTRA_CFLAGS) \
  $(COMPILER_SPECIFIC_CFLAGS)

COMMA := ,
LDFLAGS := $(patsubst %,-Wl$(COMMA)%,$(LDFLAGS))

# Note: For a real project you would _not_ want to compile & link all the files at once
# This is inefficient because it means every invocation of make will require _all_ the c
# files to be rebuilt even if nothing has changed.
all: | $(BUILDDIR)
	@$(CC) $(CFLAGS) $(SRCS) $(LDFLAGS)

$(BUILDDIR):
	@mkdir -p $(BUILDDIR)
