CC = gcc
CFLAGS = -DMOCK_GPIO
SRCS = src/*.c
OUT = bin/main.out

all: clean $(OUT)

$(OUT): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(OUT)

clean:
	rm -f $(OUT)

run:
	./bin/main.out