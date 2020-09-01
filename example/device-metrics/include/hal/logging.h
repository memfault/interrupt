#pragma once

typedef enum {
  kExampleLogLevel_Debug = 0,
  kExampleLogLevel_Info,
  kExampleLogLevel_Warning,
  kExampleLogLevel_Error,
  // Convenience definition to get the number of possible levels
  kExampleLogLevel_NumLevels,
} eExampleLogLevel;

void example_log_str(eExampleLogLevel level, const char *msg, size_t msg_len);
void example_log(eExampleLogLevel level, const char *fmt, ...);

#define _EXAMPLE_LOG_IMPL(_level, ...)                                         \
  example_log(_level, __VA_ARGS__)


#define EXAMPLE_LOG_DEBUG(...)                                          \
  _EXAMPLE_LOG_IMPL(kExampleLogLevel_Debug, __VA_ARGS__)

#define EXAMPLE_LOG_INFO(...)                                           \
  _EXAMPLE_LOG_IMPL(kExampleLogLevel_Info, __VA_ARGS__)

#define EXAMPLE_LOG_WARN(...)                                           \
  _EXAMPLE_LOG_IMPL(kExampleLogLevel_Warning, __VA_ARGS__)

#define EXAMPLE_LOG_ERROR(...)                                          \
  _EXAMPLE_LOG_IMPL(kExampleLogLevel_Error, __VA_ARGS__)
