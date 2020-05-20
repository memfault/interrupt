#pragma once

#include <stdint.h>

uint32_t example_project_run_memory_leak_examples(void);


int example_operate_on_pointer(uint8_t *pointer);


int example_divide_by_zero(int z);

void example_run_mutex_examples(void);


uint64_t force_libgcc_builtin_dependency(uint64_t a, uint64_t b);
