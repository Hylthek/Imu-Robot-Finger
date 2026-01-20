#include "spi.h"

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "imu.h"

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
int spi_transfer(int file_desc, uint8_t* tx_buffer, uint8_t* rx_buffer,
                 size_t len) {
  struct spi_ioc_transfer spi_transfer = {
      .tx_buf = (unsigned long)tx_buffer,
      .rx_buf = (unsigned long)rx_buffer,
      .len = len,
      .speed_hz = 1000000,
      .bits_per_word = 8,
  };

  return ioctl(file_desc, SPI_IOC_MESSAGE(1), &spi_transfer);
}

int InitSpiDevice() {
  const char* device_name =
      "/dev/spidev0.0";  // Use /dev/spidev0.1 for the second chip select
  int spi_file_desc;
  int spi_mode = 3;
  spi_file_desc = spi_open(device_name, spi_mode);
  if (spi_file_desc < 0) {
    perror("Cant open SPI device.");
    return -1;
  }
  ImuInitRegisters(spi_file_desc);
  return spi_file_desc;
}