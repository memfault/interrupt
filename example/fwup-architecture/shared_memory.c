#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "memory_map.h"
#include "shared_memory.h"

const uint32_t MAGIC = 0xbadcafe;

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t flags;
} shared_memory_t;

shared_memory_t shared_memory __attribute__((section(".shared_memory")));

enum {
    UPDATE_REQUESTED = 1 << 0,
    UPDATE_COMPLETE = 1 << 1,
};

static void prv_set_flag(uint32_t flag, bool value) {
    if (value) {
        shared_memory.flags |= flag;
    } else {
        shared_memory.flags &= ~flag;
    }
}

static bool prv_get_flag(uint32_t flag) {
    return shared_memory.flags & flag;
}

void shared_memory_init(void) {
    if (shared_memory.magic != MAGIC) {
        printf("Shared memory uinitialized, setting magic\n");
        memset(&shared_memory, 0, sizeof(shared_memory_t));
        shared_memory.magic = MAGIC;
    }
}

bool shared_memory_is_update_requested(void) {
    return prv_get_flag(UPDATE_REQUESTED);
}

void shared_memory_set_update_requested(bool yes) {
    prv_set_flag(UPDATE_REQUESTED, yes);
}

bool shared_memory_is_update_complete(void) {
    return prv_get_flag(UPDATE_COMPLETE);
}

void shared_memory_set_update_complete(bool yes) {
    prv_set_flag(UPDATE_COMPLETE, yes);
}

