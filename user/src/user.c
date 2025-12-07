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
#include "../include/client_data.h"
#include "../common/verifications.h"
#include "../common/common.h"

char current_uid[UID_LENGTH + 1] = "";
char current_password[PASSWORD_LENGTH + 1] = "";
int is_logged_in = LOGGED_OUT;

char IP[INET_ADDRSTRLEN] = DEFAULT_IP;
char port[6] = DEFAULT_PORT;


void usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [-n server_ip] [-p server_port]\n", prog_name);
    fprintf(stderr, "  -n server_ip    Specify the server IP address\n");
    fprintf(stderr, "  -p server_port  Specify the server port number\n");
}

void parse_arguments(int argc, char *argv[]) {   
    int opt;
    while ((opt = getopt(argc, argv, "-p:-n:")) != -1) {
        switch (opt) {
            case 'p':
                if(!is_valid_port(optarg)) {
                    fprintf(stderr, "Error: Invalid port number\n");
                    exit(EXIT_FAILURE);
                }
               strcpy(port, optarg);
                break;
            case 'n':   
                strcpy(IP, optarg);
                break;
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
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

int main(int argc, char* argv[]) {

    parse_arguments(argc, argv);

    struct sockaddr_in server_udp_addr;
    int udp_fd = setup_udp(IP, port, &server_udp_addr);
    if (udp_fd == ERROR) {
        fprintf(stderr, "UDP setup failed\n");
        return ERROR;
    }

    char input_buffer[256] = {0};
    char command_buffer[256] = {0};
    char command_arg_buffer[256] = {0};

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