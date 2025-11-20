#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BASE_PORT 58000
#define GROUP_NUMBER 32
#define DEFAULT_PORT "58032"
#define PORTMAX 65535
#define PORTMIN 0

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

    return DEFAULT_PORT;
}

int main() {
    int fd, n;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    socklen_t addrlen;
    char buffer[128];

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        return -1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    int err = getaddrinfo(NULL, DEFAULT_PORT, &hints, &res);
    if (err != 0) {
        return -1;
    }

    n = bind(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        return -1;  
    }

    printf("Servidor UDP à espera na porta %s...\n", DEFAULT_PORT);

    while (1) {
        addrlen = sizeof(addr);

        n = recvfrom(fd, buffer, sizeof(buffer), 0,
                     (struct sockaddr *)&addr, &addrlen);

        if (n == -1) {
            return -1;
        }

        write(1, "received: ", 10);
        write(1, buffer, n);

        // Enviar de volta ao cliente
        n = sendto(fd, buffer, n, 0,
                   (struct sockaddr *)&addr, addrlen);

        if (n == -1) {
            return -1;
        }
    }

    freeaddrinfo(res);
    close(fd);
    return 0;
}
