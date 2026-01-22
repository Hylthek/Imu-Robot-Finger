#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "imu.h"

int spi_open(const char* device, int mode);
int spi_transfer(int file_desc, uint8_t* tx_buffer, uint8_t* rx_buffer, size_t len);
void InitSpiDevice();
ImuSample_t SpiImuReadParse();
