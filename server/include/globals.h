#ifndef __GLOBALS_H__
#define __GLOBALS_H__
#include "data.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>  // For socket functions
#include <sys/socket.h>  // For socket functions
#include <sys/types.h>  // For data types
#include <netinet/in.h>  // For sockaddr_in
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>  // For time functions
#include <signal.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <pthread.h>

#include "../../common/common.h"

#define ERROR -1
#define EMPTY_FILE -2
#define DIR_ALREADY_EXISTS -3
#define MAX_TCP_CLIENTS 10 

#define PAST '0'
#define ACCEPTING '1'
#define SOLD_OUT '2'
#define CLOSED '3'

typedef struct {
    int verbose;
    char* port;
    int udp_socket;
    int tcp_socket;
    fd_set read_fds;
    fd_set temp_fds;
    struct timeval timeout;
} Settings;

typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len;
    int is_tcp;
    char buffer[BUFFER_SIZE];
    char** cursor;
} Request;

extern Settings set;

#endif