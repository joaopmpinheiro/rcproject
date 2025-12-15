#ifndef __TCP_RESPONSE_PARSER_H__
#define __TCP_RESPONSE_PARSER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "common.h"
#include "data.h"
#include "verifications.h"

ReplyStatus read_file_size(int tcp_fd, char* file_size);
ReplyStatus read_file_name(int tcp_fd, char* file_name);
ReplyStatus read_seat_count(int tcp_fd, char* seat_count);
ReplyStatus read_event_date(int tcp_fd, char* event_date);
ReplyStatus read_event_name(int tcp_fd, char* event_name);
ReplyStatus read_eid(int tcp_fd, char* eid);
ReplyStatus read_uid(int tcp_fd, char* uid);
#endif