#ifndef __COMMON_H__
#define __COMMON_H__

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "data.h"

#define BASE_PORT 58000
#define GROUP_NUMBER 32
#define DEFAULT_PORT "58032"
#define DEFAULT_IP "127.0.0.1" // TODO CONFIRM  eu acho q é o tejo aqui
#define MAX_HOSTNAME_LENGTH 256
#define PORTMAX 65535
#define PORTMIN 0

// TODO por todos os defines no mesmo sitio
#define TIMEOUT_SECONDS 5
#define PASSWORD_LENGTH 8
#define COMMAND_LENGTH 3
#define UID_LENGTH 6
#define EID_LENGTH 3
#define SEAT_COUNT_LENGTH 3
#define DAY_STR_SIZE 10
#define TIME_STR_SIZE 5
#define EVENT_DATE_LENGTH 16 // DD-MM-YYYY HH:MM
#define FILE_NAME_LENGTH 24
#define FILE_SIZE_LENGTH 8
#define MAX_EVENTS 999
#define MAX_EVENT_NAME 10
#define MAX_AVAIL_SEATS 999
#define MIN_AVAIL_SEATS 10
#define MAX_EVENT_NAME 10 // TODO: já ta no common.h (?) mas é MAX_EVENT_NAME_LENGTH
#define DATE_LENGTH 11
#define TIME_LENGTH 5
#define SHOW_BUFFER_SIZE (EID_LENGTH + MAX_EVENT_NAME + EVENT_DATE_LENGTH + SEAT_COUNT_LENGTH * 2 + FILE_NAME_LENGTH + 36)

#define MAX_FILE_SIZE (1024 * 1024 * 10) // 10 MB
#define TCP_BUFFER_SIZE 1024
#define BUFFER_SIZE 256

#define LOGGED_IN 1
#define LOGGED_OUT 0

#define SUCCESS 1
#define FAILURE 0

#define ERROR -1

#define VALID 1
#define INVALID 0

#define EOM '\n'

int tcp_send_message(int fd, char *message);
int tcp_send_file(int fd, char *file_name);
int tcp_read(int fd, void *buf, size_t len);
int tcp_write(int fd, const char* buffer, size_t length);
int tcp_read_field(int fd, char* buffer, size_t max_len);
int tcp_read_file(int fd, char *file_name, long file_size);

#endif
