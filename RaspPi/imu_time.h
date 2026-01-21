#pragma once

#include <time.h>

typedef struct timespec timespec;

typedef struct {
  timespec start_time;
  timespec prev_time;
  timespec curr_time;
  timespec spi_time;
  timespec parse_time;
  timespec log_time;
  timespec stdin_time;
} ImuTimespecs;

// Stores timespec structs for various different times in the recording & sampling cycles.
extern ImuTimespecs gTimes;
extern ImuTimespecs gPrevTimes;

double TimespecToDouble(timespec ts);
double TimespecDiff(timespec ts1, timespec ts2);
void GetMonotonic(timespec* ts_ptr);
void PrintDebugTimes();
void UpdatePrevTimespecs();
