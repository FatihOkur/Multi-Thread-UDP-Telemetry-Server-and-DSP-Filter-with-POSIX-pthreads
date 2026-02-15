CC = gcc
CFLAGS = -Wall -Wextra -pthread -Iinclude -std=c99 -O2
TARGET = telemetry_server

SRCS = src/main.c src/udp_server.c src/dsp_filter.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
