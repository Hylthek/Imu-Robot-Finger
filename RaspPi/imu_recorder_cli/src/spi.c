#include "spi.h"

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "imu.h"

int spi_file_desc = -1;  // -1 is null file descriptor value I think.

// Function to open the SPI device
int spi_open(const char* device, int mode) {
  int file_desc = open(device, O_RDWR);
  if (file_desc < 0) {
    perror("Could not open SPI device");
    return -1;
  }

  // Set SPI mode (e.g., 0, 1, 2, or 3)
  if (ioctl(file_desc, SPI_IOC_WR_MODE, &mode) == -1) {
    perror("Can't set SPI mode");
    close(file_desc);
    return -1;
  }
  return file_desc;
}

// Function to perform SPI transfer.
int spi_transfer(int file_desc, uint8_t* tx_buffer, uint8_t* rx_buffer, size_t len) {
  struct spi_ioc_transfer spi_transfer = {
      .tx_buf = (unsigned long)tx_buffer,
      .rx_buf = (unsigned long)rx_buffer,
      .len = len,
      .speed_hz = 1000000,
      .bits_per_word = 8,
  };

  return ioctl(file_desc, SPI_IOC_MESSAGE(1), &spi_transfer);
}

void InitSpiDevice() {
  const char* device_name = "/dev/spidev0.0";  // Use /dev/spidev0.1 for the second chip select
  int spi_mode = 3;
  spi_file_desc = spi_open(device_name, spi_mode);
  if (spi_file_desc < 0) {
    perror("Cant open SPI device.");
  }
  ImuInitRegisters(spi_file_desc);
}

ImuSample_t SpiImuReadParse() {
  uint8_t spi_out[13] = {0},
          spi_in[13] = {0};  // 13 is enough to read all IMU data.
  spi_out[0] = 0x1f | 0x80;  // AccelX1 register address with reading bit (0x80) set.
  if (spi_transfer(spi_file_desc, spi_out, spi_in, 13) == -1) {
    perror("SPI transfer failed");
    close(spi_file_desc);
    exit(1);
  }
  
  ImuSample_t imu_data_notime = {0,
                      (spi_in[1] << 8) + spi_in[2],
                      (spi_in[3] << 8) + spi_in[4],
                      (spi_in[5] << 8) + spi_in[6],
                      (spi_in[7] << 8) + spi_in[8],
                      (spi_in[9] << 8) + spi_in[10],
                      (spi_in[11] << 8) + spi_in[12]};

  return imu_data_notime;
}