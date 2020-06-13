#include "hal/logging.h"

#include "literal_remap_example.h"

const sExampleContext g_example_context = {
  .version = 1,
};

void literal_remap_example(const sExampleContext *context) {
  EXAMPLE_LOG("Context: %p: version=%d at addr 0x%p", context, context->version, context->version);
}
