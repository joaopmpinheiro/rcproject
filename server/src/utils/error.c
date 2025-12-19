#include "../include/globals.h"
#include <arpa/inet.h>

void server_log(const char* message, struct sockaddr_in* client_addr) {
    if (set.verbose) {
        if (client_addr != NULL) {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr->sin_addr, client_ip, INET_ADDRSTRLEN);
            int client_port = ntohs(client_addr->sin_port);
            printf("%s [from %s:%d]\n", message, client_ip, client_port);
        } else {
            printf("%s\n", message);
        }
    }
}

void usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [-n server_ip] [-p server_port]\n", prog_name);
    fprintf(stderr, "  -p server_port  Specify the server port number\n");
    fprintf(stderr, "  -v              Enable verbose mode\n");
}