// Basic includes.
#include <gpiod.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "cli.h"
#include "csv.h"
#include "imu.h"
#include "imu_time.h"
#include "libgpiod_imu_interrupt.h"
#include "priority_manager.h"
#include "spi.h"

const int kImuIntPin = 25; // Adjust as needed.

int main(void)
{
  SetMaxPriority();                  // Makes program run with less stalling.
  SigIntHandlerSetup();                    // Handle Ctrl+C terminal interrupt.
  InitSpiDevice();                   // Init spi device.
  GpioSetup(kImuIntPin);             // Init IMU interrupt pin.
  printf("Program Initialized\n\n"); // Status message.

  // Record loop.
  while (true)
  {
    // Intro message.
    printf("Press enter to start and stop recording\n");

    // Wait for record command.
    while (stdin_has_data_poll() == false)
      ;

    // Flush stdin.
    while (stdin_has_data_poll() == true)
      getchar();

    // Create/open file.
    gImuCsvFd = OpenNewCsv();

    // Print csv headers.
    fprintf(gImuCsvFd, "Time, ax, ay, az, gx, gy, gz\n");

    // Get the recording monotonic time at start.
    GetMonotonic(&gTimes.start_time);
    gPrevTimes.curr_time = gTimes.start_time;

    // IMU loop.
    printf("Recording...\n");
    while (true)
    {
      // Check stdin buffer for recording stop command.
      if (stdin_has_data_poll())
      {
        // Close csv file.
        fclose(gImuCsvFd);
        gImuCsvFd = NULL;
        // Flush stdin.
        while (stdin_has_data_poll() == true)
          getchar();
        break;
      }

      // Wait for IMU interrupt.
      if (GpioGetEvent() == false)
      {
        continue;
      }

      // Counter [0, 999].
      static int count = 999;
      if (++count == 1000)
        count = 0;

      // Get current time.
      GetMonotonic(&gTimes.curr_time);

      // Perform the SPI transfer.
      ImuSample_t imu_data = SpiImuReadParse();
      // Add time data.
      imu_data.t = TimespecDiff(gTimes.curr_time, gTimes.start_time);

      // Post-SPI time.
      GetMonotonic(&gTimes.spi_time);
      // Debug post-parse time.
      GetMonotonic(&gTimes.parse_time);

      // Log received data.
      int chars_printed = fprintf(gImuCsvFd, "%f, %d, %d, %d, %d, %d, %d\n",
                                  imu_data.t,
                                  imu_data.ax, imu_data.ay, imu_data.az,
                                  imu_data.gx, imu_data.gy, imu_data.gz);
      assert(chars_printed > 1);

      // Post-log time.
      GetMonotonic(&gTimes.log_time);

      // Post-stdin time.
      GetMonotonic(&gTimes.stdin_time);

      // Print debug info.
      PrintDebugTimes(1.5); // 1.5ms is the lower cutoff to print debug info.
      // Update prev timespecs.
      UpdatePrevTimespecs();
    }
    printf("RECORDING ENDED\n\n");
  }
}
