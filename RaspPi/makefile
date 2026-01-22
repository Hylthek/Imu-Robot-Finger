NO_PRINT = --no-print-directory

.PHONY: all clean imu_recorder_cli imu_kernel_isr

all: imu_recorder_cli imu_kernel_isr

imu_recorder_cli:
	@$(MAKE) -C imu_recorder_cli $(NO_PRINT)

imu_kernel_isr:
	@$(MAKE) -C imu_kernel_isr $(NO_PRINT)

clean:
	@$(MAKE) -C imu_recorder_cli clean $(NO_PRINT)
	@$(MAKE) -C imu_kernel_isr clean $(NO_PRINT)