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
#include "../include/client_data.h"
#include "../../common/common.h"


int connect_tcp(char* ip, char* port) {
    struct addrinfo hints, *res;
    int fd, errcode;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    errcode = getaddrinfo(ip, port, &hints, &res);
    if (errcode != 0) return ERROR;

    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) {
        freeaddrinfo(res);
        perror("TCP Socket creation failed");
        return ERROR;
    }

    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1) {
        close(fd);
        freeaddrinfo(res);
        perror("TCP Connection failed");
        return ERROR;
    }

    freeaddrinfo(res);
    return fd;
}

int setup_udp(char* ip, char* port, struct sockaddr_in* server_addr) {
    struct addrinfo hints, *res;
    int fd, errcode;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    errcode = getaddrinfo(ip, port, &hints, &res);
    if (errcode != 0) return ERROR;

    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) {
        freeaddrinfo(res);
        return ERROR;
    }

    struct timeval timeout = {5, 0};

    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    memset(server_addr, 0, sizeof(*server_addr));
    memcpy(server_addr, res->ai_addr, res->ai_addrlen);

    freeaddrinfo(res);
    return fd;
}