CC = gcc
CFLAGS = -DMOCK_GPIO
SRCS = main.c gpio.c spi.c imu.c cli.c csv.c priority_manager.c imu_time.c
OUT = main.out

# .PHONY: all

all:$(OUT)
	clear
	@echo "Made"

$(OUT): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(OUT)

clean:
	@clear
	@rm -f $(OUT)
	@mv data/* archive/ 2>/dev/null || true
	@echo "Cleant"

run:
	clear
	@./$(OUT)