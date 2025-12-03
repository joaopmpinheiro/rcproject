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
#include <signal.h>

#define BASE_PORT 58000
#define GROUP_NUMBER 32
#define PORTMAX 65535
#define PORTMIN 0
#define MAX_TCP_CLIENTS 10 // TODO: confirmar enunciado
#define BUFFER_SIZE 128 // TODO: confirmar enunciado
#define TIMEOUT_SEC 5

int verbose = 0;

char* get_server_port(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            char* port = argv[i + 1];
            int port_num = atoi(port);
            if (port_num < PORTMIN || port_num > PORTMAX) {
                return -1;
            }
            return port;
        }
    }
    return DEFAULT_PORT;
}

int setup_UDP(int port){
    struct addrinfo hints, *res;

    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        perror("UDP Socket creation failed"); 
        return -1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket
    hints.ai_flags = AI_PASSIVE; // server mode
    
    int errcode = getaddrinfo(NULL, port, &hints, &res);
    if (errcode != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errcode));
        close(udp_fd);
        return -1;
    }

    ssize_t n = bind(udp_fd, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    if (n == -1) {
        perror("UDP bind failed");
        close(udp_fd);
        return -1;  
    }

    printf("Servidor UDP à espera na porta %s...\n", port);
    return udp_fd;
}

int setup_TCP(char* port){
    struct addrinfo hints, *res;

    int tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd == -1) {
        perror("TCP Socket creation failed");
        return -1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    hints.ai_flags = AI_PASSIVE; // server mode

    int errcode = getaddrinfo(NULL, port, &hints, &res);
    if (errcode) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errcode));
        close(tcp_fd);
        return -1;
    }

    // assign an IP address and port number to the socket
    // (needed so clients know where to connect/send data) (TODO REMOVE)
    int n = bind(tcp_fd, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    if (n != 0) {
        perror("TCP bind failed");
        close(tcp_fd);  
        return -1;
    }

    if (listen(tcp_fd, MAX_TCP_CLIENTS) != 0) { // TODO: quantas connections em espera? - neste momento aceita 5
        perror("Listen failed");
        close(tcp_fd);
        return -1;
    }

    printf("TCP server listening on port %s...\n", port);
    return tcp_fd;
}

int handle_tcp_client(void *arg) {
    int client_fd = (intptr_t)arg;
    char buffer[BUFFER_SIZE];
    ssize_t n;

    while (1) {
        n = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (n <= 0) {
            if (n == 0) {
                printf("TCP client disconnected\n");
            } else {
                perror("TCP recv failed");
            }
            close(client_fd);
            break;
        } else {
            write(1, "TCP received: ", 14);
            write(1, buffer, n);
            // echo back
            send(client_fd, buffer, n, 0);
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    ssize_t n;
    socklen_t addrlen;
    struct sockaddr_in addr;
    char buffer[128];
    int max_fd, new_fd, opt;
    char* port = DEFAULT_PORT;

    while ((opt = getopt(argc, argv, "p:v")) != -1) {
            switch (opt) {
                case 'p':   // -p <port>x\
                    port = optarg;
                    break;

                case 'v':   // -v
                    verbose = 1;
                    break;

                default:
                    fprintf(stderr, "Usage: %s [-p port] [-v]\n", argv[0]);
                    exit(EXIT_FAILURE);
            }
        }



    const char* SERVER_PORT = get_server_port(argc, argv);


    signal(SIGPIPE, SIG_IGN);
    
    int udp_fd = setup_UDP(port);
    if (udp_fd == -1) {
        return 1;
    }
    
    int tcp_fd = setup_TCP(port);
    if (tcp_fd == -1) {
        close(udp_fd);
        return 1;
    }
    
    
    // --- Setup select ---
    fd_set readfds, masterfds;
    FD_ZERO(&masterfds);
    FD_SET(udp_fd, &masterfds);
    FD_SET(tcp_fd, &masterfds);
    max_fd = (udp_fd > tcp_fd) ? udp_fd : tcp_fd;



    while (1) {
        readfds = masterfds;
        struct timeval timeout = {TIMEOUT_SEC, 0}; 


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
            } 
            printf("UDP received from %s:%d: ", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
            if(write(1, buffer, n) < 0) {
                perror("Write to stdout failed");
            }
        }

        // --- Check TCP socket (new connections) ---
        if (FD_ISSET(tcp_fd, &readfds)) {
            addrlen = sizeof(addr);
            new_fd = accept(tcp_fd, (struct sockaddr *)&addr, &addrlen);
            if (new_fd < 0) {
                perror("TCP accept failed");
            }
            
            if (new_fd > max_fd) max_fd = new_fd;
            FD_SET(new_fd, &masterfds);
            printf("New TCP connection from %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        }

        

    close(udp_fd);
    close(tcp_fd);
    return 0;
    }
}
