#include "imu_time.h"

#include <math.h>
#include <stdio.h>
#include <time.h>

ImuTimespecs gTimes = {0};
ImuTimespecs gPrevTimes = {0};

inline double TimespecToDouble(timespec ts) {
  return ts.tv_sec + ts.tv_nsec / 1e9;
}
inline double TimespecDiff(timespec ts1, timespec ts2) {
  return fabs((ts2.tv_sec - ts1.tv_sec) + (ts2.tv_nsec - ts1.tv_nsec) / 1e9);
}
inline void GetMonotonic(timespec* ts_ptr) {
  clock_gettime(CLOCK_MONOTONIC, ts_ptr);
}

void PrintDebugTimes() {
  if (TimespecDiff(gTimes.curr_time, gPrevTimes.curr_time) > 0.0017)
    printf(
        "DEBUGINFO:\n"
        "prev:%9f +%9f +%9f +%9f +%9f = %9f\n"
        "curr:%9f +%9f +%9f +%9f +%9f =%9f\n"
        "diff:%9f\n\n",
        TimespecDiff(gPrevTimes.curr_time, gTimes.start_time),
        TimespecDiff(gPrevTimes.spi_time, gPrevTimes.curr_time),
        TimespecDiff(gPrevTimes.parse_time, gPrevTimes.spi_time),
        TimespecDiff(gPrevTimes.log_time, gPrevTimes.parse_time),
        TimespecDiff(gPrevTimes.stdin_time, gPrevTimes.log_time),
        TimespecDiff(gPrevTimes.stdin_time, gTimes.start_time),

        TimespecDiff(gTimes.curr_time, gTimes.start_time),
        TimespecDiff(gTimes.spi_time, gTimes.curr_time),
        TimespecDiff(gTimes.parse_time, gTimes.spi_time),
        TimespecDiff(gTimes.log_time, gTimes.parse_time),
        TimespecDiff(gTimes.stdin_time, gTimes.log_time),
        TimespecDiff(gTimes.stdin_time, gTimes.start_time),

        TimespecDiff(gTimes.curr_time, gPrevTimes.curr_time));
}

void UpdatePrevTimespecs() {
  gPrevTimes.curr_time = gTimes.curr_time;
  gPrevTimes.spi_time = gTimes.spi_time;
  gPrevTimes.parse_time = gTimes.parse_time;
  gPrevTimes.log_time = gTimes.log_time;
  gPrevTimes.stdin_time = gTimes.stdin_time;
}