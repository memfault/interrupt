#pragma once

#include "example_project/compiler.h"

EXAMPLE_PROJ_FUNC_ACQUIRES_LOCK(flash)
void flash_lock(void);

EXAMPLE_PROJ_FUNC_RELEASES_LOCK(flash)
void flash_unlock(void);

EXAMPLE_PROJ_FUNC_ACQUIRES_LOCK(accel)
void accel_lock(void);

EXAMPLE_PROJ_FUNC_RELEASES_LOCK(accel)
void accel_unlock(void);

EXAMPLE_PROJ_FUNC_REQUIRES_LOCK_HELD(accel)
extern void example_func_requires_accel(void);
extern int do_some_work_while_holding_locks(void);


void example_locks_boot(void);
