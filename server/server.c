#define _POSIX_C_SOURCE 200112L // TODO: confirmar se isto esta bem/é preciso

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

#define BASE_PORT 58000
#define GROUP_NUMBER 32
#define DEFAULT_PORT "58032"
#define PORTMAX 65535
#define PORTMIN 0
#define MAX_TCP_CLIENTS 10 // TODO: confirmar enunciado
#define BUFFER_SIZE 128 // TODO: confirmar enunciado


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
    int udp_fd, tcp_fd, errcode;
    ssize_t n;
    socklen_t addrlen;
    struct addrinfo hints, *res;
    struct sockaddr_in addr;
    char buffer[128];
    fd_set readfds, masterfds;
    int max_fd, new_fd;

    int tcp_clients_fd[MAX_TCP_CLIENTS] = {0};
    
    
    // ---- UDP socket setup ----
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        perror("UDP Socket creation failed"); 
        return 1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket
    hints.ai_flags = AI_PASSIVE; // server mode
    
    errcode = getaddrinfo(NULL, DEFAULT_PORT, &hints, &res);
    if (errcode) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errcode));
        close(udp_fd);
        return 1;
    }

    n = bind(udp_fd, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    if (n == -1) {
        perror("UDP bind failed");
        close(udp_fd);
        return 1;  
    }

    printf("Servidor UDP à espera na porta %s...\n", DEFAULT_PORT);


    // ---- TCP socket setup ----
    //aka communication endpoint (TODO REMOVE)
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd == -1) {
        perror("TCP Socket creation failed");
        close(udp_fd);
        return 1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    hints.ai_flags = AI_PASSIVE; // server mode

    errcode = getaddrinfo(NULL, DEFAULT_PORT, &hints, &res);

    // assign an IP address and port number to the socket
    // (needed so clients know where to connect/send data) (TODO REMOVE)
    n = bind(tcp_fd, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    if (n != 0) {
        perror("TCP bind failed");
        close(udp_fd);
        close(tcp_fd);
        return 1;
    }

    if (listen(tcp_fd, MAX_TCP_CLIENTS) != 0) { // TODO: quantas connections em espera? - neste momento aceita 5
        perror("Listen failed");
        close(udp_fd);
        close(tcp_fd);
        return 1;
    }

    printf("TCP server listening on port %s...\n", DEFAULT_PORT);

    // --- Setup select ---
    FD_ZERO(&masterfds);
    FD_SET(udp_fd, &masterfds);
    FD_SET(tcp_fd, &masterfds);
    max_fd = (udp_fd > tcp_fd) ? udp_fd : tcp_fd;

    // 5 seconds timeout for select
    struct timeval timeout = {5, 0}; 
    int tcp_clients_fd[MAX_TCP_CLIENTS] = {0};


    while (1) {
        readfds = masterfds;

        if (select(max_fd + 1, &readfds, NULL, NULL, &timeout) < 0) {
            perror("select error");
            break;
        }

        // --- Check UDP socket ---
        if (FD_ISSET(udp_fd, &readfds)) {
            addrlen = sizeof(addr);
            n = recvfrom(udp_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&addr, &addrlen);
            if (n < 0) {
                perror("UDP recvfrom failed");
            } else {
                write(1, "UDP received: ", 14);
                write(1, buffer, n);
                // echo back
                sendto(udp_fd, buffer, n, 0, (struct sockaddr *)&addr, addrlen);
            }
        }

        // --- Check TCP socket (new connections) ---
        if (FD_ISSET(tcp_fd, &readfds)) {
            addrlen = sizeof(addr);
            new_fd = accept(tcp_fd, (struct sockaddr *)&addr, &addrlen);
            if (new_fd < 0) {
                perror("TCP accept failed");
            } else {
                printf("New TCP client connected\n");
                // add new client to list
                for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
                    if (tcp_clients[i] == 0) {
                        tcp_clients[i] = new_fd;
                        FD_SET(new_fd, &masterfds);
                        if (new_fd > max_fd) max_fd = new_fd;
                        break;
                    }
                }
            }
            close(new_fd); // TODO: remover isto depois de adicionar aos clients
        }

        // --- Check existing TCP clients ---
        for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
            int fd = tcp_clients[i];
            if (fd > 0 && FD_ISSET(fd, &readfds)) {
                n = recv(fd, buffer, BUFFER_SIZE, 0);
                if (n <= 0) {
                    if (n == 0) {
                        printf("TCP client disconnected\n");
                    } else {
                        perror("TCP recv failed");
                    }
                    close(fd);
                    FD_CLR(fd, &masterfds);
                    tcp_clients[i] = 0;
                } else {
                    write(1, "TCP received: ", 14);
                    write(1, buffer, n);
                    // echo back
                    send(fd, buffer, n, 0);
                }
            }
        }
    }

    close(udp_fd);
    close(tcp_fd);
    for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
        if (tcp_clients[i] > 0) close(tcp_clients[i]);
    }
    return 0;
}
