#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
  bool enabled;
  uint32_t revision;
  size_t num_code_comparators;
  size_t num_literal_comparators;
} sFpbConfig;

void fpb_dump_breakpoint_config(void);
void fpb_dump_config(void);
void fpb_get_config(sFpbConfig *config);

typedef struct {
  bool enabled;
  uint8_t replace;
  // start address
  uint32_t address;
} sFpbCompConfig;

bool fpb_get_comp_config(size_t comp_id, sFpbCompConfig *config);
bool fpb_set_breakpoint(size_t comp_id, uint32_t addr);
bool fpb_remap_function(size_t comp_id, uint32_t orig_instr_addr,
                        uint32_t new_instr_addr);

void fpb_enable(void);
void fpb_disable(void);
