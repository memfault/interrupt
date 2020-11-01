#pragma once

#include <stdbool.h>

//! Get's the next available char on the input stream
bool shell_port_getchar(char *c_out);

__attribute__((noreturn))
void shell_processing_loop(void);
