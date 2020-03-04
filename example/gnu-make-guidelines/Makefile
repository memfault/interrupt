# Makefile for building the 'example' binary from C sources

# Verbose flag
ifeq ($(V),1)
Q :=
else
Q := @
endif

# The build folder, for all generated output. This should normally be included
# in a .gitignore rule
BUILD_FOLDER := build

# Default all rule will build the 'example' target, which here is an executable
.PHONY:
all: $(BUILD_FOLDER)/example

# List of C source files. Putting this in a separate variable, with a file on
# each line, makes it easy to add files later (and makes it easier to see
# additions in pull requests). Larger projects might use a wildcard to locate
# source files automatically.
SRC_FILES = \
    src/example.c \
    src/main.c

# Generate a list of .o files from the .c files. Prefix them with the build
# folder to output the files there
OBJ_FILES = $(addprefix $(BUILD_FOLDER)/,$(SRC_FILES:.c=.o))

# Generate a list of depfiles, used to track includes. The file name is the same
# as the object files with the .d extension added
DEP_FILES = $(addsuffix .d,$(OBJ_FILES))

# Flags to generate the .d dependency-tracking files when we compile.  It's
# named the same as the target file with the .d extension
DEPFLAGS = -MMD -MP -MF $@.d

# Include the dependency tracking files
-include $(DEP_FILES)

# List of include dirs. These are put into CFLAGS.
INCLUDE_DIRS = \
    src/

# Prefix the include dirs with '-I' when passing them to the compiler
CFLAGS += $(addprefix -I,$(INCLUDE_DIRS))

# Set some compiler flags we need. Note that we're appending to the CFLAGS
# variable
CFLAGS += \
    -std=c11 \
    -Wall \
    -Werror \
    -ffunction-sections -fdata-sections \
    -Og \
    -g3

# Our project requires some linker flags: garbage collect sections, output a
# .map file
LDFLAGS += \
    -Wl,--gc-sections,-Map,$@.map

# Set LDLIBS to specify linking with libm, the math library
LDLIBS += \
    -lm

# The rule for compiling the SRC_FILES into OBJ_FILES
$(BUILD_FOLDER)/%.o: %.c
	@echo Compiling $(notdir $<)
	@# Create the folder structure for the output file
	@mkdir -p $(dir $@)
	$(Q) $(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# The rule for building the executable "example", using OBJ_FILES as
# prerequisites. Since we're not relying on an implicit rule, we need to
# explicity list CFLAGS, LDFLAGS, LDLIBS
$(BUILD_FOLDER)/example: $(OBJ_FILES)
	@echo Linking $(notdir $@)
	$(Q) $(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Remove debug information for a smaller executable. An embedded project might
# instead using [arm-none-eabi-]objcopy to convert the ELF file to a raw binary
# suitable to be written to an embedded device
STRIPPED_OUTPUT = $(BUILD_FOLDER)/example-stripped

$(STRIPPED_OUTPUT): $(BUILD_FOLDER)/example
	@echo Stripping $(notdir $@)
	$(Q)objcopy --strip-debug $^ $@

# Since all our generated output is placed into the build folder, our clean rule
# is simple. Prefix the recipe line with '-' to not error if the build folder
# doesn't exist (the -f flag for rm also has this effect)
.PHONY: clean
clean:
	- rm -rf $(BUILD_FOLDER)
