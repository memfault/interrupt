#include "fakes/fake_mutex.h"

#include "mutex/mutex.h"

#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#define NUM_MUTEXES 256

typedef struct Mutex {
  uint8_t lock_count;
} Mutex;

static Mutex s_mutexes[NUM_MUTEXES];
static uint32_t s_mutex_index;

// Fake Helpers

void fake_mutex_init(void) {
  memset(s_mutexes, 0, sizeof(s_mutexes));
}

bool fake_mutex_all_unlocked(void) {
  for (int i = 0; i < NUM_MUTEXES; i++) {
    if (s_mutexes[i].lock_count > 0) {
      return false;
    }
  }
  return true;
}

// Implementation

Mutex *mutex_create(void) {
  assert(s_mutex_index < NUM_MUTEXES);
  return &s_mutexes[s_mutex_index++];
}

void mutex_lock(Mutex *mutex) {
  mutex->lock_count++;
}

void mutex_unlock(Mutex *mutex) {
  mutex->lock_count--;
}
