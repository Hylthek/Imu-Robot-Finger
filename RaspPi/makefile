CC = gcc
CFLAGS = -DMOCK_GPIO
SRCS = main.c gpio.c spi.c imu.c cli.c csv.c priority_manager.c
OUT = main.out

all: $(OUT)

$(OUT): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(OUT)

clean:
	rm -f $(OUT)
	mv data/* archive/ 2>/dev/null || true