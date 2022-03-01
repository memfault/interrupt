# Makefile for building + running the example.c application under clang + gcc
# and different optimization levels.

# Silent all build steps by default. Run with --trace to see them.
.SILENT:

# Disable built-in rules
.SUFFIXES:

# Clang and GCC compiler executable names.
#
# The 'clang'/'gcc' prefixes are derived from the target name, to reduce
# boilerplate in this makefile.
clang_CC ?= clang
gcc_CC ?= gcc

CFLAGS += -ggdb3

# Build with each of these optimization levels
TEST_OPTIMIZATION_LEVELS = \
    0 \
    1 \
    2 \
    3 \
    s

# The test targets are of the form: <compiler>.<optimization level>
TEST_TARGETS = \
    $(TEST_OPTIMIZATION_LEVELS:%=clang.%) \
    $(TEST_OPTIMIZATION_LEVELS:%=gcc.%) \

# All targets are phony, we want to run everything every time
.PHONY: all $(TEST_TARGETS)
all: $(TEST_TARGETS)

# Compile + run the example.
$(TEST_TARGETS): example.c
	mkdir -p build/
# Extract compiler name and optimization level from the target name.
	$($(basename $@)_CC) \
		$(CFLAGS) -O$(patsubst .%,%,$(suffix $@)) \
		example.c -o build/$@.out
	./build/$@.out
