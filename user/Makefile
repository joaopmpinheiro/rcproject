CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -D_POSIX_C_SOURCE=200809L
TARGET = user
OBJS = user.o command_handlers.o verifications.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

user.o: user.c command_handlers.h verifications.h
	$(CC) $(CFLAGS) -c user.c

command_handlers.o: command_handlers.c command_handlers.h
	$(CC) $(CFLAGS) -c command_handlers.c

verifications.o: verifications.c verifications.h
	$(CC) $(CFLAGS) -c verifications.c

clean:
	rm -f $(TARGET) $(OBJS)