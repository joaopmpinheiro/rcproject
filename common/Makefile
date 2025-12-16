CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -D_POSIX_C_SOURCE=200809L -O2 -I.

TARGET = libcommon.a
OBJS = common.o\
		verifications.o\
		parser.o

all: $(TARGET)

$(TARGET): $(OBJS)
	ar rcs $@ $^

%.o: %.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
