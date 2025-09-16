#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/spi.h"
#include "time.h"
#include <inttypes.h>

enum
{
    kImuRxPin = 16,
    kImuCsPin = 17,
    kImuSckPin = 18,
    kImuTxPin = 19,
    kImuInterruptPin = 15,
    kAccelDataXhRegister = 0x1f,
    kImuClkinPin = 21,
};

enum
{
    // IMU registers
    // Note that some are defined only for reference as bulk reads exists).
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

#pragma region Function Definitions

void ImuInitRegisters() {
    // Do some one-time SPI writes.
    uint8_t out_buf[2], in_buf[2];

    // Bank 0.
    out_buf[0] = kIntConfig1; // Initialize interrupts.
    out_buf[1] = 0b00000000;
    spi_write_read_blocking(spi0, out_buf, in_buf, 2);
    out_buf[0] = kIntSource0;
    out_buf[1] = 0b00001000; // Change interrupt output from "Reset done" to "UI data ready".
    spi_write_read_blocking(spi0, out_buf, in_buf, 2);
    out_buf[0] = kIntConfig;
    out_buf[1] = 0b00000010; // Set dataReady interrupt to push-pull.
    spi_write_read_blocking(spi0, out_buf, in_buf, 2);
    out_buf[0] = kPwrMgmt0;
    out_buf[1] = 0b00001111; // Place gyro and accel in low noise mode.
    spi_write_read_blocking(spi0, out_buf, in_buf, 2);
    out_buf[0] = kIntfConfig1;
    out_buf[1] = 0b10010001; // RTC clock input is NOT required.
    spi_write_read_blocking(spi0, out_buf, in_buf, 2);
    out_buf[0] = kAccelConfig0;
    out_buf[1] = 0b01000110; // Change FS from +-16g to +-4g.
    spi_write_read_blocking(spi0, out_buf, in_buf, 2);
    out_buf[0] = kGyroConfig0;
    out_buf[1] = 0b01000110; // Change FS from +-2000dps to +-500dps.
    spi_write_read_blocking(spi0, out_buf, in_buf, 2);

    // Bank 1.
    out_buf[0] = kRegBankSel;
    out_buf[1] = 0b00000001; // Change from bank 0 to bank 1.
    out_buf[2] = kIntfConfig5;
    out_buf[3] = 0b00000000; // Sets pin 9 function to Default (INT2).
    out_buf[4] = kRegBankSel;
    out_buf[5] = 0b00000000; // Change from bank 1 to bank 0.
    spi_write_read_blocking(spi0, out_buf, in_buf, 6);
}

#pragma endregion

void main()
{
    // LED init.
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // UART printf init.
    stdio_init_all();

    // Imu interrupt init.
    gpio_init(kImuInterruptPin);
    gpio_set_dir(kImuInterruptPin, GPIO_IN);
    gpio_disable_pulls(kImuInterruptPin);
    gpio_pull_up(kImuInterruptPin);

    // Imu Spibus init.
    spi_init(spi0, 1000 * 1000);
    spi_set_format(spi0, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
    gpio_set_function(kImuRxPin, GPIO_FUNC_SPI);
    gpio_set_function(kImuCsPin, GPIO_FUNC_SPI);
    gpio_set_function(kImuSckPin, GPIO_FUNC_SPI);
    gpio_set_function(kImuTxPin, GPIO_FUNC_SPI);
    gpio_disable_pulls(kImuRxPin); // Since when was this line needed???

    ImuInitRegisters();

    // Create the IMU instruction buffer, out_buf, and the register buffer, in_buf
    uint8_t out_buf[20], in_buf[20]; // 20 is an arbitrary size.

    // Do some one-time SPI writes.
    out_buf[0] = kIntConfig1; // Initialize interrupts.
    out_buf[1] = 0b00000000;
    spi_write_read_blocking(spi0, out_buf, in_buf, 2);
    out_buf[0] = kIntSource0;
    out_buf[1] = 0b00001000; // Change interrupt output from "Reset done" to "UI data ready".
    spi_write_read_blocking(spi0, out_buf, in_buf, 2);
    out_buf[0] = kIntConfig;
    out_buf[1] = 0b00000010; // Set dataReady interrupt to push-pull.
    spi_write_read_blocking(spi0, out_buf, in_buf, 2);
    out_buf[0] = kPwrMgmt0;
    out_buf[1] = 0b00001111; // Place gyro and accel in low noise mode.
    spi_write_read_blocking(spi0, out_buf, in_buf, 2);
    out_buf[0] = kIntfConfig1;
    out_buf[1] = 0b10010001; // RTC clock input is NOT required.
    spi_write_read_blocking(spi0, out_buf, in_buf, 2);

    out_buf[0] = kRegBankSel;
    out_buf[1] = 0b00000001; // Change from bank 0 to bank 1.
    out_buf[2] = kIntfConfig5;
    out_buf[3] = 0b00000000; // Sets pin 9 function to Default (INT2).
    out_buf[4] = kRegBankSel;
    out_buf[5] = 0b00000000; // Change from bank 1 to bank 0.
    spi_write_read_blocking(spi0, out_buf, in_buf, 6);

    out_buf[0] = kAccelConfig0;
    out_buf[1] = 0b01000110; // Change FS from +-16g to +-4g.
    spi_write_read_blocking(spi0, out_buf, in_buf, 2);
    out_buf[0] = kGyroConfig0;
    out_buf[1] = 0b01000110; // Change FS from +-2000dps to +-500dps.
    spi_write_read_blocking(spi0, out_buf, in_buf, 2);

    // Recording buffers.
    const int kSamplesToRecord = 5000;
    absolute_time_t timestamp_record_buffer[kSamplesToRecord];
    int16_t imu_record_buffer[kSamplesToRecord * 6];

    // Blink for a second.
    for (int i = 0; i < 10; i++)
    {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(50);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        sleep_ms(50);
    }
    printf("Program start.\n");

    while (true)
    {
        // Wait until kImuInterruptPin pin is low.
        while (gpio_get(kImuInterruptPin) == true)
        {
            tight_loop_contents();
        }

        // Counter for current sample.
        static uint curr_sample = 0;
        curr_sample++;

        // Record time.
        absolute_time_t curr_time = get_absolute_time();

        // Read from reg and parse data.
        out_buf[0] = kAccelDataXhRegister | 0x80;
        spi_write_read_blocking(spi0, out_buf, in_buf, 13);
        int16_t accel_reg[3] = {(in_buf[1] << 8) + in_buf[2], (in_buf[3] << 8) + in_buf[4], (in_buf[5] << 8) + in_buf[6]};
        int16_t gyro_reg[3] = {(in_buf[7] << 8) + in_buf[8], (in_buf[9] << 8) + in_buf[10], (in_buf[11] << 8) + in_buf[12]};

        if (curr_sample > kSamplesToRecord)
            break;
        // Add data to recording buffers.
        imu_record_buffer[(curr_sample - 1) * 6 + 0] = accel_reg[0];
        imu_record_buffer[(curr_sample - 1) * 6 + 1] = accel_reg[1];
        imu_record_buffer[(curr_sample - 1) * 6 + 2] = accel_reg[2];
        imu_record_buffer[(curr_sample - 1) * 6 + 3] = gyro_reg[0];
        imu_record_buffer[(curr_sample - 1) * 6 + 4] = gyro_reg[1];
        imu_record_buffer[(curr_sample - 1) * 6 + 5] = gyro_reg[2];
        timestamp_record_buffer[curr_sample - 1] = curr_time;

        // Sleep for safety.
        sleep_us(200);
    }

    // Print all data in csv format.
    printf("Time (microseconds), Acceleration X (divide by 8192 to get Gs), Acceleration Y, Acceleration Z, Gyro X (divide by 65.5 to get degrees/sec), Gyro Y, Gyro Z\n");
    for (int i = 0; i < kSamplesToRecord; i++)
    {
        printf("%" PRId64 ", %d, %d, %d, %d, %d, %d\n", timestamp_record_buffer[i], imu_record_buffer[i * 6 + 0], imu_record_buffer[i * 6 + 1], imu_record_buffer[i * 6 + 2], imu_record_buffer[i * 6 + 3], imu_record_buffer[i * 6 + 4], imu_record_buffer[i * 6 + 5]);
    }
}