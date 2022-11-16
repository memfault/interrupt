
STRIP ?= strip
OBJCOPY ?= objcopy

.PHONY: all
all: \
    hello \
    hello.debug \
    hello.stripped \
    hello.debug.stripped \
    hello.debug.ld-compressed \
    hello.debug.compressed \
    hello.debug.compressed-zlib-gnu


# The base binary, no debug symbols enabled
hello: hello.c
	$(CC) $(CFLAGS) -o $@ $<

# The base binary + debug symbols
hello.debug: hello.c
	$(CC) $(CFLAGS) -ggdb3 -g -o $@ $<

# The debug binary, stripped
hello.stripped: hello.debug
	$(STRIP) -o $@ $<

# The debug binary, with debug symbols extracted and linked to the stripped
# binary
hello.debug.stripped: hello.debug
	$(OBJCOPY) --only-keep-debug $^ $@.dbg
	$(STRIP) -o $@ $<
	$(OBJCOPY) --add-gnu-debuglink=$@.dbg $@

# The debug binary with debug info compressed by ld
hello.debug.ld-compressed: hello.c
	$(CC) $(CFLAGS) -ggdb3 -Wl,--compress-debug-sections=zlib -o $@ $<

# The debug binary with debug sections compressed with objcopy
hello.debug.compressed: hello.debug
	$(OBJCOPY) --compress-debug-sections=zlib $< $@

# Very similar to above, but "The debug sections are renamed to begin with
# .zdebug instead of .debug"
hello.debug.compressed-zlib-gnu: hello.debug
	$(OBJCOPY) --compress-debug-sections=zlib-gnu $< $@
