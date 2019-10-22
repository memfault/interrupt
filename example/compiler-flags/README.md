# Introduction

A small example application used to demonstrate the impact of various compilation flags

## Usage

Arguments:

- `EXTRA_CFLAGS`="compilation flags to build with"
- `EXTRA_LDFLAGS`="ld flags to build with"
- `USE_CLANG=1` will compile with clang instead of gcc

USAGE:

```
EXTRA_CFLAGS="-g -Wformat=2" make
main.c: In function 'print_user_provided_buffer':
main.c:82:3: warning: format not a string literal and no format arguments [-Wformat-security]
   printf(buf);
```
