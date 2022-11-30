#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    LEVEL_NONE = 0,
    LEVEL_LOW,
    LEVEL_MEDIUM,
    LEVEL_HIGH,

} LEVEL_enum_t;

typedef struct
{
    LEVEL_enum_t level;
    int8_t priority;
    char message[64];
    bool isReady;
} my_struct_t;
