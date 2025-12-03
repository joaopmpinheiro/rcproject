#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

#include "common.h"

int is_number(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return 0;
        }
    }
    return 1;
}

// TODO: necessário um is valid ip?

int is_valid_port(const char *port_str) {
    if (!is_number(port_str)) {
        return INVALID;
    }

    int port = atoi(port_str);
    if (port < PORTMIN || port > PORTMAX) {
        return INVALID;
    }

    return VALID;
}



static char* get_server_port(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 >= argc) {
                return INVALID;
            }
            if (!is_valid_port(argv[i + 1])) {
                return INVALID;
            }
            return argv[i + 1];
        }
    }
    return DEFAULT_PORT;
}