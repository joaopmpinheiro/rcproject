#include "../include/globals.h"

void server_log(const char* message) {
    if (set.verbose) {
        printf("%s\n", message);
    }
}

void usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [-n server_ip] [-p server_port]\n", prog_name);
    fprintf(stderr, "  -p server_port  Specify the server port number\n");
    fprintf(stderr, "  -v              Enable verbose mode\n");
}