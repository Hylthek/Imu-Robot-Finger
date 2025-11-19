#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdlib.h> // Required for exit()

// Function to open the SPI device
int spi_open(const char *device, int mode) {
    int fd = open(device, O_RDWR);
    if (fd < 0) {
        perror("Could not open SPI device");
        return -1;
    }

    // Set SPI mode (e.g., 0, 1, 2, or 3)
    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) == -1) {
        perror("Can't set SPI mode");
        close(fd);
        return -1;
    }
    return fd;
}

// Function to perform SPI transfer
int spi_transfer(int fd, uint8_t *tx_buffer, uint8_t *rx_buffer, size_t len) {
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buffer,
        .rx_buf = (unsigned long)rx_buffer,
        .len = len,
        .speed_hz = 1000000, // Example speed: 1 MHz
        .bits_per_word = 8,
    };

    return ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
}

int main(void) {
    const char *device = "/dev/spidev0.0"; // Use /dev/spidev0.1 for the second chip select
    int fd;
    uint8_t tx_data[2] = {0x75 | 0x80, 0x00}; // Data to send
    uint8_t rx_data[2];
    int mode = 0; // SPI Mode 0

    // Open the SPI device
    fd = spi_open(device, mode);
    if (fd < 0) {
        exit(1);
    }

    while (1) {
        // Perform the SPI transfer (sends 0x33 and 0xFF, receives data into rx_data)
        if (spi_transfer(fd, tx_data, rx_data, sizeof(tx_data)) == -1) {
            perror("SPI transfer failed");
            close(fd);
            exit(1);
        }
        
        // Print received data
        printf("Received: 0x%02X 0x%02X\n", rx_data[0], rx_data[1]);

        sleep(1);
    }
}
