#pragma once

//! @file
//!
//! @brief
//! A (contrived) accelerometer driver used for illustrating a memory
//! corruption bug

#include <inttypes.h>
#include <stdbool.h>

//! Processes raw accelerometer readings
void accel_process_reading(int x, int y, int z);

//! Prototype for handler invoked each time accel data
//! is processed
typedef void (*AccelSampleProcessedCallback)(void);

//! Register handler to be called when an accel
//! sample is processed
void accel_register_watcher(
    AccelSampleProcessedCallback data_processed_cb);
