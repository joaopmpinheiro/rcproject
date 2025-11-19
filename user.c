#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define BASE_PORT 58000
#define GROUP_NUMBER 32
#define DEFAULT_PORT 58032
#define PORTMAX 65535
#define PORTMIN 0

char* get_server_ip(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            return argv[i + 1];
        }
    }
    return NULL;
}

char* get_server_port(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            char* port = argv[i + 1];
            int port_num = atoi(port);
            if (port_num < PORTMIN || port_num > PORTMAX) {
                return NULL;
            }
            return port;
        }
    }
    
    static char def_port[6];
    snprintf(def_port, sizeof def_port, "%d", DEFAULT_PORT);
    return def_port;
}

int main(int argc, char* argv[]) {

    char* SERVER_IP = get_server_ip(argc, argv);
    char* SERVER_PORT = get_server_port(argc, argv);
    
    if (SERVER_IP == NULL || SERVER_PORT == NULL) {
        return -1;
    }
    
    struct addrinfo hints, *res;
    int fd, errcode;
    ssize_t n;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) return -1;
    
    struct timeval timeout = {5, 0}; // 5 seconds timeout
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    errcode = getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &res);
    if (errcode != 0) {
        close(fd);
        return -1;
    }

    n = sendto(fd, "Hello!\n", 7, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        freeaddrinfo(res);
        close(fd);
        return -1;
    }

    freeaddrinfo(res);
    return 0;
}