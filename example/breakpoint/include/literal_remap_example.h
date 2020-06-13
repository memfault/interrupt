#pragma once

#include <stdint.h>

typedef struct {
  uint32_t version;
} sExampleContext;

const sExampleContext g_example_context;

void literal_remap_example(const sExampleContext *context);
