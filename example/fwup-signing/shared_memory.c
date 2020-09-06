#include <stdio.h>
#include <string.h>

#include "memory_map.h"
#include "shared_memory.h"

const uint32_t MAGIC = 0xbadcafe;

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t flags;
    uint8_t boot_counter;
} shared_memory_t;

shared_memory_t shared_memory __attribute__((section(".shared_memory")));

enum {
    DFU_REQUESTED = 1 << 0,
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

bool shared_memory_is_dfu_requested(void) {
    return prv_get_flag(DFU_REQUESTED);
}

void shared_memory_set_dfu_requested(bool yes) {
    prv_set_flag(DFU_REQUESTED, yes);
}

void shared_memory_increment_boot_counter(void) {
    shared_memory.boot_counter++;
}

void shared_memory_clear_boot_counter(void) {
    shared_memory.boot_counter = 0;
}

uint8_t shared_memory_get_boot_counter(void) {
    return shared_memory.boot_counter;
}

