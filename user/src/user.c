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

#include "../include/command_handlers.h"
#include "../include/verifications.h"
#include "../include/client_data.h"
#include "../common/common.h"

char current_uid[7] = "";
char current_password[9] = "";
int is_logged_in = 0;

char* get_server_ip(int argc, char* argv[]) {
    // TODO: validar IP
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            return argv[i + 1];
        }
    }
    
    return DEFAULT_IP;
}

char* get_server_port(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 >= argc) {
                return NULL;
            }
            if (!is_valid_port(argv[i + 1])) {
                return NULL;
            }
            return argv[i + 1];
        }
    }
    return DEFAULT_PORT;
}

int connect_tcp(char* ip, char* port) {
    struct addrinfo hints, *res;
    int fd, errcode;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    errcode = getaddrinfo(ip, port, &hints, &res);
    if (errcode != 0) return -1;

    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) {
        freeaddrinfo(res);
        perror("TCP Socket creation failed");
        return -1;
    }

    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1) {
        close(fd);
        freeaddrinfo(res);
        perror("TCP Connection failed");
        return -1;
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
    if (errcode != 0) return -1;

    fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) {
        freeaddrinfo(res);
        return -1;
    }

    struct timeval timeout = {5, 0};

    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    memset(server_addr, 0, sizeof(*server_addr));
    memcpy(server_addr, res->ai_addr, res->ai_addrlen);

    freeaddrinfo(res);
    return fd;
}

int main(int argc, char* argv[]) {

    char* SERVER_IP = get_server_ip(argc, argv);
    char* SERVER_PORT = get_server_port(argc, argv);
    
    if (SERVER_IP == NULL || SERVER_PORT == NULL) {
        return -1;
    }

    struct sockaddr_in server_udp_addr;
    int udp_fd = setup_udp(SERVER_IP, SERVER_PORT, &server_udp_addr);
    if (udp_fd == -1) {
        fprintf(stderr, "UDP setup failed\n");
        return -1;
    }

    char input_buffer[256] = {0};
    char command_buffer[256] = {0};
    char command_arg_buffer[256] = {0};

    fflush(stdout);
    while (1) {
        printf("> ");
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) break;

        memset(command_buffer, 0, sizeof(command_buffer));
        memset(command_arg_buffer, 0, sizeof(command_arg_buffer));

        sscanf(input_buffer, "%s %[^\n]", command_buffer, command_arg_buffer);

        CommandType command = identify_command(command_buffer);
        command_handler(command, command_arg_buffer, udp_fd, &server_udp_addr);
    }

    return 0;
}