# filepath: /home/calvinsmith/Documents/Imu-Robot-Finger/RaspPi/imu_recorder_cli/makefile
CC = gcc
CFLAGS = #-DMOCK_GPIO
LDFLAGS = -lpigpio
SRCS = src/*.c
OUT = bin/main.out

all: clean $(OUT)

$(OUT): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) $(LDFLAGS) -o $(OUT)

clean:
	rm -f $(OUT)

run:
	./bin/main.out