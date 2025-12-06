CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 \
	-Iinclude -I../common -D_POSIX_C_SOURCE=200809L

SRCDIR = src
UTILS = $(SRCDIR)/utils

OBJS = \
	$(UTILS)/command_handlers.o \
	$(SRCDIR)/user.o

TARGET = user

all: $(TARGET)

$(TARGET): $(OBJS) ../common/libcommon.a
	$(CC) $(CFLAGS) -o $@ $(OBJS) ../common/libcommon.a

$(UTILS)/%.o: $(UTILS)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)