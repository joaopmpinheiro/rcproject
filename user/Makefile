CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 \
	-I. -I../common -D_POSIX_C_SOURCE=200809L

OBJS = \
	command_handlers.o \
	user.o \
	verifications.o

TARGET = user

all: $(TARGET)

$(TARGET): $(OBJS) ../common/libcommon.a
	$(CC) $(CFLAGS) -o $@ $(OBJS) ../common/libcommon.a

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
