#include "../include/constants.h"
#include "../include/globals.h"
#include "../../include/utils.h"

void parse_arguments(int argc, char *argv[]) {   
    int opt;
    settings.port = DEFAULT_PORT;
    settings.verbose = 0;

    while ((opt = getopt(argc, argv, "p:v")) != -1) {
        switch (opt) {
            case 'p':
                if(!is_valid_port(optarg)) {
                    fprintf(stderr, "Error: Invalid port number\n");
                    exit(EXIT_FAILURE);
                }
                settings.port = optarg;
                break;
            case 'v':
                settings.verbose = 1;
                printf("Verbose mode enabled\n");
                break;
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}