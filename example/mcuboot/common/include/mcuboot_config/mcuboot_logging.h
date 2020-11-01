#pragma once

#include "hal/logging.h"

#define MCUBOOT_LOG_MODULE_DECLARE(...)

#define MCUBOOT_LOG_ERR(_fmt, ...) \
  EXAMPLE_LOG("[ERR] " _fmt, ##__VA_ARGS__)
#define MCUBOOT_LOG_WRN(_fmt, ...) \
  EXAMPLE_LOG("[WRN] " _fmt, ##__VA_ARGS__)
#define MCUBOOT_LOG_INF(_fmt, ...) \
  EXAMPLE_LOG("[INF] " _fmt, ##__VA_ARGS__)
#define MCUBOOT_LOG_DBG(_fmt, ...) \
  EXAMPLE_LOG("[DBG] " _fmt, ##__VA_ARGS__)
