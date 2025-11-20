CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
TARGET = server.o
SRC = server.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)