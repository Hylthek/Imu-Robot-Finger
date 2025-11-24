#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdlib.h> // Required for exit()
#include <sys/queue.h>
#include <time.h>
#include <pigpio.h>
#include <signal.h>

enum
{
    // IMU registers
    // Note that some are defined only for reference as bulk reads exists.
    kPwrMgmt0 = 0x4E,
    kAccelDataX1 = 0x1F,
    kAccelDataX0 = 0x20,
    kAccelDataY1 = 0x21,
    kAccelDataY0 = 0x22,
    kAccelDataZ1 = 0x23,
    kAccelDataZ0 = 0x24,
    kGyroDataX1 = 0x25,
    kGyroDataX0 = 0x26,
    kGyroDataY1 = 0x27,
    kGyroDataY0 = 0x28,
    kGyroDataZ1 = 0x29,
    kGyroDataZ0 = 0x2A,
    kIntConfig1 = 0x64,
    kIntSource0 = 0x65,
    kIntConfig = 0x14,
    kIntfConfig1 = 0x4d,
    kRegBankSel = 0x76,
    kIntfConfig5 = 0x7b, // Note, this is in bank 1 , not bank 0.
    kAccelConfig0 = 0x50,
    kGyroConfig0 = 0x4f,
};

// Globals.
FILE *imu_data_csv = NULL;

// Handle SIGINT.
void HandleSigInt(int signal)
{
    if (imu_data_csv != NULL)
    {
        fclose(imu_data_csv); // Close the file
        printf("File closed.\n");
    }
    printf("Exiting Program.\n");
    exit(0);
}

// Function to open the SPI device
int spi_open(const char *device, int mode)
{
    int file_desc = open(device, O_RDWR);
    if (file_desc < 0)
    {
        perror("Could not open SPI device");
        return -1;
    }

    // Set SPI mode (e.g., 0, 1, 2, or 3)
    if (ioctl(file_desc, SPI_IOC_WR_MODE, &mode) == -1)
    {
        perror("Can't set SPI mode");
        close(file_desc);
        return -1;
    }
    return file_desc;
}

// Function to perform SPI transfer.
int spi_transfer(int file_desc, uint8_t *tx_buffer, uint8_t *rx_buffer, size_t len)
{
    struct spi_ioc_transfer spi_transfer = {
        .tx_buf = (unsigned long)tx_buffer,
        .rx_buf = (unsigned long)rx_buffer,
        .len = len,
        .speed_hz = 200000,
        .bits_per_word = 8,
    };

    return ioctl(file_desc, SPI_IOC_MESSAGE(1), &spi_transfer);
}

// One-time writes to IMU config-type registers. NOT OPTIONAL.
void ImuInitRegisters(int file_desc)
{
    // Do some one-time SPI writes.
    uint8_t spi_out[6], in_buf[6];

    // Bank 0.
    spi_out[0] = kIntConfig1;
    spi_out[1] = 0b01000000; // Initialize interrupts. Also set interrupt pulse to 100us->8us
    spi_transfer(file_desc, spi_out, in_buf, 2);
    spi_out[0] = kIntSource0;
    spi_out[1] = 0b00001000; // Change interrupt output from "Reset done" to "UI data ready".
    spi_transfer(file_desc, spi_out, in_buf, 2);
    spi_out[0] = kIntConfig;
    spi_out[1] = 0b00000010; // Set dataReady interrupt to push-pull.
    spi_transfer(file_desc, spi_out, in_buf, 2);
    spi_out[0] = kPwrMgmt0;
    spi_out[1] = 0b00001111; // Place gyro and accel in low noise mode.
    spi_transfer(file_desc, spi_out, in_buf, 2);
    spi_out[0] = kIntfConfig1;
    spi_out[1] = 0b10010001; // RTC clock input is NOT required.
    spi_transfer(file_desc, spi_out, in_buf, 2);
    spi_out[0] = kAccelConfig0;
    spi_out[1] = 0b00000100; // Keep FS at +-16g, increase IMU freq from 1kHz to 1kHz.
    spi_transfer(file_desc, spi_out, in_buf, 2);
    spi_out[0] = kGyroConfig0;
    spi_out[1] = 0b01000110; // Change FS from +-2000dps to +-500dps.
    spi_transfer(file_desc, spi_out, in_buf, 2);

    // Bank 1.
    spi_out[0] = kRegBankSel;
    spi_out[1] = 0b00000001; // Change from bank 0 to bank 1.
    spi_out[2] = kIntfConfig5;
    spi_out[3] = 0b00000000; // Sets pin 9 function to Default (INT2).
    spi_out[4] = kRegBankSel;
    spi_out[5] = 0b00000000; // Change from bank 1 to bank 0.
    spi_transfer(file_desc, spi_out, in_buf, 6);
}

// Structs.
typedef struct
{
    double t;
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t gx;
    int16_t gy;
    int16_t gz;
} ImuSample;

int main(void)
{
    // Get the program start monotonic time.
    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // Handle SIGINT.
    struct sigaction sig_action;
    sig_action.sa_handler = HandleSigInt;
    sig_action.sa_flags = 0;
    if (sigaction(SIGINT, &sig_action, NULL) == -1)
    {
        perror("Failed to set SIGINT handler");
        return 1;
    }

    // Init spi device.
    const char *device_name = "/dev/spidev0.0"; // Use /dev/spidev0.1 for the second chip select
    int spi_file_desc;
    int spi_mode = 3;
    spi_file_desc = spi_open(device_name, spi_mode);
    if (spi_file_desc < 0)
        perror("Cant open SPI device.");
    ImuInitRegisters(spi_file_desc);

    // Init IMU interrupt pin.
    if (gpioInitialise() < 0)
    {
        perror("pigpio failed");
        return 1;
    }
    const unsigned PIN = 25;
    gpioSetMode(PIN, PI_INPUT);
    gpioSetPullUpDown(PIN, PI_PUD_UP);

    // Create/open file.
    FILE *imu_data_csv = fopen("imu_data.csv", "w");
    fprintf(imu_data_csv, "Time, ax, ay, az, gx, gy, gz\n");

    // Main loop.
    while (1)
    {
        if (gpioRead(PIN) == 1)
            continue;

        static int count = 999;
        if (++count == 1000)
            count = 0;

        // Get current time.
        struct timespec curr_time;
        clock_gettime(CLOCK_MONOTONIC, &curr_time);

        // Perform the SPI transfer.
        uint8_t spi_out[13] = {0}, spi_in[13] = {0}; // 13 is enough to read all IMU data.
        spi_out[0] = 0x1f | 0x80;
        if (spi_transfer(spi_file_desc, spi_out, spi_in, 13) == -1)
        {
            perror("SPI transfer failed");
            close(spi_file_desc);
            exit(1);
        }

        // Parse IMU bits.

        ImuSample data = {(curr_time.tv_sec - start_time.tv_sec) +
                              (curr_time.tv_nsec - start_time.tv_nsec) / 1e9,
                          (spi_in[1] << 8) + spi_in[2], (spi_in[3] << 8) + spi_in[4], (spi_in[5] << 8) + spi_in[6],
                          (spi_in[7] << 8) + spi_in[8], (spi_in[9] << 8) + spi_in[10], (spi_in[11] << 8) + spi_in[12]};

        // Log received data.
        fprintf(imu_data_csv, "%f, %d, %d, %d, %d, %d, %d\n",
                data.t, data.ax, data.ay, data.az, data.gx, data.gy, data.gz);
    }
}
