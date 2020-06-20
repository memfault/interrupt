#pragma once

void shared_memory_init(void);
bool shared_memory_is_update_requested(void);
void shared_memory_set_update_requested(bool yes);
bool shared_memory_is_update_complete(void);
void shared_memory_set_update_complete(bool yes);
