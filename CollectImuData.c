#include <pico/stdlib.h>
#include <stdio.h>
#include <hardware/spi.h>
#include <time.h>
#include <inttypes.h>
#include <hardware/watchdog.h>
#include <pico/multicore.h>
#include <pico/util/queue.h>

enum
{
    // Pins.
    kImuRxPin = 16,
    kImuCsPin = 17,
    kImuSckPin = 18,
    kImuTxPin = 19,
    kImuInterruptPin = 15,
    kAccelDataXhRegister = 0x1f,
    kImuClkinPin = 21,
    kLedPin = PICO_DEFAULT_LED_PIN, // 25.
    kDebugPin1 = 0,
    kDebugPin2 = 1,

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

#pragma region Function Definitions

// One-time writes to IMU config-type registers.
void ImuInitRegisters()
{
    // Do some one-time SPI writes.
    uint8_t spi_out[6], in_buf[6];

    // Bank 0.
    spi_out[0] = kIntConfig1;
    spi_out[1] = 0b01000000; // Initialize interrupts. Also set interrupt pulse to 100us->8us
    spi_write_read_blocking(spi0, spi_out, in_buf, 2);
    spi_out[0] = kIntSource0;
    spi_out[1] = 0b00001000; // Change interrupt output from "Reset done" to "UI data ready".
    spi_write_read_blocking(spi0, spi_out, in_buf, 2);
    spi_out[0] = kIntConfig;
    spi_out[1] = 0b00000010; // Set dataReady interrupt to push-pull.
    spi_write_read_blocking(spi0, spi_out, in_buf, 2);
    spi_out[0] = kPwrMgmt0;
    spi_out[1] = 0b00001111; // Place gyro and accel in low noise mode.
    spi_write_read_blocking(spi0, spi_out, in_buf, 2);
    spi_out[0] = kIntfConfig1;
    spi_out[1] = 0b10010001; // RTC clock input is NOT required.
    spi_write_read_blocking(spi0, spi_out, in_buf, 2);
    spi_out[0] = kAccelConfig0;
    spi_out[1] = 0b00000100; // Keep FS at +-16g, increase IMU freq from 1kHz to 4kHz.
    spi_write_read_blocking(spi0, spi_out, in_buf, 2);
    spi_out[0] = kGyroConfig0;
    spi_out[1] = 0b01000110; // Change FS from +-2000dps to +-500dps.
    spi_write_read_blocking(spi0, spi_out, in_buf, 2);

    // Bank 1.
    spi_out[0] = kRegBankSel;
    spi_out[1] = 0b00000001; // Change from bank 0 to bank 1.
    spi_out[2] = kIntfConfig5;
    spi_out[3] = 0b00000000; // Sets pin 9 function to Default (INT2).
    spi_out[4] = kRegBankSel;
    spi_out[5] = 0b00000000; // Change from bank 1 to bank 0.
    spi_write_read_blocking(spi0, spi_out, in_buf, 6);
}

#pragma endregion

// Structs.
typedef struct
{
    absolute_time_t t;
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t gx;
    int16_t gy;
    int16_t gz;
} ImuSample;

// Global vars.
queue_t printf_buffer;
const uint kMaxQueueSize = 10000; // Use 50KB of available 264KB of SRAM.

// Secondary core.
void secondary_core_main()
{
    // Print csv header.
    printf("Time (microseconds), Acceleration X (divide by 2048 to get Gs), Acceleration Y, Acceleration Z, Gyro X (divide by 65.5 to get degrees/sec), Gyro Y, Gyro Z\n");

    ImuSample data = {0};
    while (true)
    {
        // Continue if buffer empty.
        if (queue_try_remove(&printf_buffer, &data) == false)
            continue;

        // Blink task.
        static absolute_time_t last_blink_u = 0;
        int blink_speed = 100 * queue_get_level(&printf_buffer) / kMaxQueueSize + 1; // Value ranges 1-10.
        int blink_delay_u = 3000000 / blink_speed;
        if (get_absolute_time() > last_blink_u + blink_delay_u)
        {
            gpio_put(kLedPin, !gpio_get(kLedPin));
            last_blink_u = get_absolute_time();
        }

        // Print in csv format.
        printf("%" PRId64 ", %d, %d, %d, %d, %d, %d\n", data.t, data.ax, data.ay, data.az, data.gx, data.gy, data.gz);
    }
}

void main()
{
    // LED init.
    {
        gpio_init(kLedPin);
        gpio_set_dir(kLedPin, GPIO_OUT);
    }

    // Init debug printfs and gpios.
    if (true)
    {
        stdio_uart_init_full(uart0, 250000, kDebugPin1, kDebugPin2);
        gpio_init(kDebugPin2);
        gpio_set_dir(kDebugPin2, GPIO_OUT);
        gpio_put(kDebugPin2, 0);
    }
    else
    {
        gpio_init(kDebugPin1);
        gpio_init(kDebugPin2);
        gpio_set_dir(kDebugPin1, GPIO_OUT);
        gpio_set_dir(kDebugPin2, GPIO_OUT);
        gpio_put(kDebugPin1, 0);
        gpio_put(kDebugPin2, 0);
    }

    // Clear console.
    {
        printf("\033[2J\033[HProgram Started.\n\n");
    }

    // Blinking and multicore safety sleep.
    for (int i = 0; i < 10; i++)
    {
        gpio_put(kLedPin, 1);
        sleep_ms(50);
        gpio_put(kLedPin, 0);
        sleep_ms(50);
    }

    // Start second core.
    {
        multicore_launch_core1(secondary_core_main);
    }

    // Init printf queue.
    {
        queue_init(&printf_buffer, sizeof(ImuSample), kMaxQueueSize);
    }

    // Imu initialization.
    {
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
        gpio_disable_pulls(kImuRxPin);
        ImuInitRegisters();
    }

    while (true)
    {
        // Wait until kImuInterruptPin pin is low.
        if (gpio_get(kImuInterruptPin) == true)
            continue;

        // Record time (done right after Imu interrupt).
        absolute_time_t curr_time = get_absolute_time();

        // Read from IMU.
        uint8_t spi_out[13] = {0}, spi_in[13] = {0}; // 13 is enough to read all IMU data.
        spi_out[0] = kAccelDataXhRegister | 0x80;
        spi_write_read_blocking(spi0, spi_out, spi_in, 13);

        // Parse IMU bits.
        ImuSample data = {curr_time,
                          (spi_in[1] << 8) + spi_in[2], (spi_in[3] << 8) + spi_in[4], (spi_in[5] << 8) + spi_in[6],
                          (spi_in[7] << 8) + spi_in[8], (spi_in[9] << 8) + spi_in[10], (spi_in[11] << 8) + spi_in[12]};

        // Record IMU data.
        if (queue_try_add(&printf_buffer, &data) == false)
        {
            while (true)
            {
                gpio_put(kLedPin, true);
            }
        }

        sleep_us(20);
    }
}
