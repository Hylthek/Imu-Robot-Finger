# filepath: /home/calvinsmith/Documents/Imu-Robot-Finger/RaspPi/imu_data_sender/makefile
CC = gcc
CFLAGS = 
LDFLAGS = #-lgpiod
SRCS = src/*.c
OUT = bin/main.out

all: clean $(OUT)

$(OUT): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) $(LDFLAGS) -o $(OUT)

clean:
	rm -f $(OUT)

run:
	sudo ./bin/main.out