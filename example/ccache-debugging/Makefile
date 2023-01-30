# the main program and default target
main: main.o

# a static, but generated, header file
generated_header.h:
	echo "#define GENERATED_HEADER_VALUE 1234" > $@

# set this generated header as a prerequisite to main.c; using -MMD etc is
# insufficient because those are produced after compilation
# https://gcc.gnu.org/onlinedocs/gcc-12.2.0/gcc/Preprocessor-Options.html#index-M
main.c: generated_header.h

# Set a random UUID as a compilation option, but permit user overidding
BUILD_UUID ?= $(shell cat /proc/sys/kernel/random/uuid)
main.o: CFLAGS+=-DBUILD_UUID=\"$(BUILD_UUID)\"

# force using ccache; normally would rely on the ccache symlinks, but for this
# example it's nice to be explicit
main.o: main.c
	ccache $(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f main main.o generated_header.h
