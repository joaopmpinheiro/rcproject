#include "../include/constants.h"
#include "../include/globals.h"

void usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [-n server_ip] [-p server_port]\n", prog_name);
    fprintf(stderr, "  -p server_port  Specify the server port number\n");
    fprintf(stderr, "  -v              Enable verbose mode\n");
}