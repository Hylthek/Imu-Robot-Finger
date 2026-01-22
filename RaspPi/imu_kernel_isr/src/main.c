#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>

// GPIO and SPI configuration
#define GPIO_IRQ_PIN 17  // Replace with the GPIO pin number for the interrupt
#define SPI_BUS 0
#define SPI_BUS_CS1 0
#define SPI_BUS_SPEED 1000000

static int irq_number;
static struct spi_device *spi_device;

// Function to read IMU data via SPI
static int read_imu_data(uint8_t *rx_buffer, size_t len) {
    struct spi_transfer transfer = {
        .tx_buf = NULL,  // No data to send
        .rx_buf = rx_buffer,
        .len = len,
        .speed_hz = SPI_BUS_SPEED,
    };
    struct spi_message message;
    spi_message_init(&message);
    spi_message_add_tail(&transfer, &message);

    return spi_sync(spi_device, &message);
}

// ISR for GPIO interrupt
static irqreturn_t gpio_isr_handler(int irq, void *dev_id) {
    uint8_t imu_data[6];  // Adjust size based on IMU data length
    int ret;

    // Read IMU data via SPI
    ret = read_imu_data(imu_data, sizeof(imu_data));
    if (ret < 0) {
        printk(KERN_ERR "IMU ISR: Failed to read IMU data via SPI\n");
        return IRQ_HANDLED;
    }

    // Log IMU data
    printk(KERN_INFO "IMU ISR: IMU Data: %02x %02x %02x %02x %02x %02x\n",
           imu_data[0], imu_data[1], imu_data[2],
           imu_data[3], imu_data[4], imu_data[5]);

    return IRQ_HANDLED;
}

// Initialize SPI device
static int spi_device_init(void) {
    struct spi_master *master;

    master = spi_busnum_to_master(SPI_BUS);
    if (!master) {
        printk(KERN_ERR "IMU ISR: Failed to acquire SPI master\n");
        return -ENODEV;
    }

    spi_device = spi_alloc_device(master);
    if (!spi_device) {
        printk(KERN_ERR "IMU ISR: Failed to allocate SPI device\n");
        return -ENOMEM;
    }

    spi_device->chip_select = SPI_BUS_CS1;
    spi_device->max_speed_hz = SPI_BUS_SPEED;
    spi_device->mode = SPI_MODE_0;
    spi_device->bits_per_word = 8;

    if (spi_add_device(spi_device)) {
        printk(KERN_ERR "IMU ISR: Failed to add SPI device\n");
        spi_dev_put(spi_device);
        return -ENODEV;
    }

    return 0;
}

// Module initialization
static int __init imu_isr_module_init(void) {
    int ret;

    // Initialize SPI device
    ret = spi_device_init();
    if (ret) {
        printk(KERN_ERR "IMU ISR: SPI initialization failed\n");
        return ret;
    }

    // Request GPIO and IRQ
    if (!gpio_is_valid(GPIO_IRQ_PIN)) {
        printk(KERN_ERR "IMU ISR: Invalid GPIO pin\n");
        return -ENODEV;
    }

    ret = gpio_request(GPIO_IRQ_PIN, "imu_gpio_irq");
    if (ret) {
        printk(KERN_ERR "IMU ISR: Failed to request GPIO\n");
        return ret;
    }

    gpio_direction_input(GPIO_IRQ_PIN);
    irq_number = gpio_to_irq(GPIO_IRQ_PIN);

    ret = request_irq(irq_number, gpio_isr_handler, IRQF_TRIGGER_RISING, "imu_gpio_irq", NULL);
    if (ret) {
        printk(KERN_ERR "IMU ISR: Failed to request IRQ\n");
        gpio_free(GPIO_IRQ_PIN);
        return ret;
    }

    printk(KERN_INFO "IMU ISR: Module loaded successfully\n");
    return 0;
}

// Module cleanup
static void __exit imu_isr_module_exit(void) {
    free_irq(irq_number, NULL);
    gpio_free(GPIO_IRQ_PIN);
    spi_unregister_device(spi_device);

    printk(KERN_INFO "IMU ISR: Module unloaded\n");
}

module_init(imu_isr_module_init);
module_exit(imu_isr_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Kernel module for IMU data logging via GPIO ISR and SPI");