#include "../include/globals.h"
#include <arpa/inet.h>

/**
 * @brief Logs message to stdout in verbose mode with optional client IP:PORT
 * @param message Message to log
 * @param client_addr Client socket address (NULL for pre-connection errors)
 * @note Output format: "message [from IP:PORT]" or just "message"
 */
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

/**
 * @brief Prints usage information to stderr
 * @param prog_name Program name (typically argv[0])
 */
void usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [-n server_ip] [-p server_port]\n", prog_name);
    fprintf(stderr, "  -p server_port  Specify the server port number\n");
    fprintf(stderr, "  -v              Enable verbose mode\n");
}