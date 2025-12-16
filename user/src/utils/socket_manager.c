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
#include "../../include/utils.h"
#include "../../common/common.h"
#include "../../common/data.h"



int connect_tcp(const char* ip, const char* port) {
    struct addrinfo hints, *res;
    int fd, errcode;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    errcode = getaddrinfo(ip, port, &hints, &res);
    if (errcode != 0) return ERROR;

    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        freeaddrinfo(res);
        perror("TCP Socket creation failed");
        return ERROR;
    }

    if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
        close(fd);
        freeaddrinfo(res);
        perror("TCP Connection failed");
        return ERROR;
    }

    freeaddrinfo(res);
    return fd;
}

int setup_udp(const char* ip, const char* port, struct sockaddr_in* server_addr) {
    struct addrinfo hints, *res;
    int fd, errcode;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    errcode = getaddrinfo(ip, port, &hints, &res);
    if (errcode != 0) return ERROR;

    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
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

ReplyStatus udp_send_receive(int udp_fd, struct sockaddr_in* server_udp_addr,
                            socklen_t udp_addr_len, char* request, char* response, 
                            size_t response_size) {
    ssize_t n;
    // Send request to server
    if (sendto(udp_fd, request, strlen(request), 0, (struct sockaddr*)server_udp_addr,\
        udp_addr_len) == ERROR) return STATUS_SEND_FAILED;
    
    // Read server response
    n = recvfrom(udp_fd, response, response_size - 1, 0, NULL, NULL);
    if (n == ERROR) return STATUS_RECV_FAILED;
    response[n] = '\0';
    return STATUS_UNASSIGNED;
}

ReplyStatus tcp_send_receive(char* message, char* response, 
                            size_t response_size) {
    int tcp_fd = connect_tcp(IP, PORT);
    if (tcp_fd == -1) return STATUS_SEND_FAILED;
    
    // Send request header to server
    if (tcp_send_message(tcp_fd, message) == ERROR) {
        close(tcp_fd);
        return STATUS_SEND_FAILED;
    }

    // Read server response
    if (tcp_read(tcp_fd, response, response_size) == ERROR) {
        close(tcp_fd);
        return STATUS_RECV_FAILED;
    }
    close(tcp_fd);
    return STATUS_UNASSIGNED;
}

