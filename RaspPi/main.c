// Basic includes.
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <time.h>
#include <unistd.h>

#include "cli.h"
#include "csv.h"
#include "gpio.h"
#include "imu.h"
#include "priority_manager.h"
#include "spi.h"

int main(void) {
  // Makes program run with less stalling.
  SetMaxPriority();

  // Handle Ctrl+C terminal interrupt.
  HandleSigInt();

  // Init spi device.
  int spi_file_desc = InitSpiDevice();

  // Init IMU interrupt pin.
  InitGpio();

  // Status message.
  printf("Program Initialized\n\n");

  // Record loop.
  while (1) {
    // Intro message.
    printf("Press enter to start and stop recording");
    fflush(stdout);

    // Wait for record command.
    while (stdin_has_data_poll() == false);

    // Flush stdin.
    while (stdin_has_data_poll() == true) getchar();

    // Create/open file.
    time_t now = time(NULL);
    char date[20];
    strftime(date, sizeof(date), "%Y-%m-%d_%H-%M-%S", localtime(&now));
    char kTempFileName[50];
    snprintf(kTempFileName, sizeof(kTempFileName), "data/data_%s.csv", date);
    gImuDataCsv = fopen(kTempFileName, "w");
    // Print csv headers.
    fprintf(gImuDataCsv, "Time, ax, ay, az, gx, gy, gz\n");

    // Get the recording monotonic time at start.
    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // IMU loop.
    printf("Recording...\n");
    while (1) {
      // Wait for IMU interrupt.
      if (gIntNegEdge == false) continue;

      // Counter.
      static int count = 999;
      if (++count == 1000) count = 0;

      // Get current time.
      struct timespec curr_time;
      clock_gettime(CLOCK_MONOTONIC, &curr_time);

      // Perform the SPI transfer.
      uint8_t spi_out[13] = {0},
              spi_in[13] = {0};  // 13 is enough to read all IMU data.
      spi_out[0] = 0x1f | 0x80;
      if (spi_transfer(spi_file_desc, spi_out, spi_in, 13) == -1) {
        perror("SPI transfer failed");
        close(spi_file_desc);
        exit(1);
      }

      // Post-SPI time.
      struct timespec spi_time;
      clock_gettime(CLOCK_MONOTONIC, &spi_time);

      // Parse IMU bits.
      ImuSample_t data = {(curr_time.tv_sec - start_time.tv_sec) +
                              (curr_time.tv_nsec - start_time.tv_nsec) / 1e9,
                          (spi_in[1] << 8) + spi_in[2],
                          (spi_in[3] << 8) + spi_in[4],
                          (spi_in[5] << 8) + spi_in[6],
                          (spi_in[7] << 8) + spi_in[8],
                          (spi_in[9] << 8) + spi_in[10],
                          (spi_in[11] << 8) + spi_in[12]};

      // Post-parse time.
      struct timespec parse_time;
      clock_gettime(CLOCK_MONOTONIC, &parse_time);

      // Log received data.
      fprintf(gImuDataCsv, "%f, %d, %d, %d, %d, %d, %d\n", data.t, data.ax,
              data.ay, data.az, data.gx, data.gy, data.gz);

      // Post-log time.
      struct timespec log_time;
      clock_gettime(CLOCK_MONOTONIC, &log_time);

      // Check stdin buffer for recording stop command.
      if (stdin_has_data_poll()) {
        char discard[100] = {0};
        fgets(discard, 99, stdin);
        break;
      }

      // Post-stdin time.
      struct timespec stdin_time;
      clock_gettime(CLOCK_MONOTONIC, &stdin_time);

      // Declare prev time.
      static struct timespec prev_time[5] = {0};

      // Print debug info.
      if (curr_time.tv_sec + curr_time.tv_nsec / 1e9 >
          prev_time[0].tv_sec + prev_time[0].tv_nsec / 1e9 + 0.0017)
        printf(
            "DEBUGINFO:\n"
            "prev:%9f +%9f +%9f +%9f +%9f = %9f\n"
            "curr:%9f +%9f +%9f +%9f +%9f =%9f\n"
            "diff:%9f\n\n",
            (prev_time[0].tv_sec - start_time.tv_sec) +
                (prev_time[0].tv_nsec - start_time.tv_nsec) / 1e9,
            (prev_time[1].tv_sec - prev_time[0].tv_sec) +
                (prev_time[1].tv_nsec - prev_time[0].tv_nsec) / 1e9,
            (prev_time[2].tv_sec - prev_time[1].tv_sec) +
                (prev_time[2].tv_nsec - prev_time[1].tv_nsec) / 1e9,
            (prev_time[3].tv_sec - prev_time[2].tv_sec) +
                (prev_time[3].tv_nsec - prev_time[2].tv_nsec) / 1e9,
            (prev_time[4].tv_sec - prev_time[3].tv_sec) +
                (prev_time[4].tv_nsec - prev_time[3].tv_nsec) / 1e9,
            (prev_time[4].tv_sec - start_time.tv_sec) +
                (prev_time[4].tv_nsec - start_time.tv_nsec) / 1e9,

            (curr_time.tv_sec - start_time.tv_sec) +
                (curr_time.tv_nsec - start_time.tv_nsec) / 1e9,
            (spi_time.tv_sec - curr_time.tv_sec) +
                (spi_time.tv_nsec - curr_time.tv_nsec) / 1e9,
            (parse_time.tv_sec - spi_time.tv_sec) +
                (parse_time.tv_nsec - spi_time.tv_nsec) / 1e9,
            (log_time.tv_sec - parse_time.tv_sec) +
                (log_time.tv_nsec - parse_time.tv_nsec) / 1e9,
            (stdin_time.tv_sec - log_time.tv_sec) +
                (stdin_time.tv_nsec - log_time.tv_nsec) / 1e9,
            (stdin_time.tv_sec - start_time.tv_sec) +
                (stdin_time.tv_nsec - start_time.tv_nsec) / 1e9,

            (curr_time.tv_sec - prev_time[0].tv_sec) +
                (curr_time.tv_nsec - prev_time[0].tv_nsec) / 1e9);
      prev_time[0] = curr_time;
      prev_time[1] = spi_time;
      prev_time[2] = parse_time;
      prev_time[3] = log_time;
      prev_time[4] = stdin_time;

      // Unset IMU interrupt bool.
      gIntNegEdge = false;
    }

    // Name the file.
    printf("Name your file: ");
    char name[100] = {0};
    fgets(name, 99, stdin);
    name[strcspn(name, "\n")] = '\0';  // Turn the last newline into a \0.
    strcat(name, ".csv\0");
    if (rename(kTempFileName, name) == 0)
      printf("File successfully renamed\n\n");
    else
      perror("rename");
  }
}
