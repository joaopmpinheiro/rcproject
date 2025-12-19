#include "../include/globals.h"
#include "../../include/utils.h"
#include "../../common/verifications.h"

/**
 * @brief Parses command line arguments and updates server settings
 * @param argc Argument count
 * @param argv Argument values (argv[0] is program name)
 * @note -p specifies port, -v enables verbose mode
 */
void parse_arguments(int argc, char *argv[]) {   
    int opt;
    set.port = DEFAULT_PORT;
    set.verbose = 0;

    while ((opt = getopt(argc, argv, "-p:-v")) != -1) {
        switch (opt) {
            case 'p':
                if(!is_valid_port(optarg)) {
                    fprintf(stderr, "Error: Invalid port number\n");
                    exit(EXIT_FAILURE);
                }
                set.port = optarg;
                break;
            case 'v':
                set.verbose = 1;
                printf("Verbose mode enabled\n");
                break;
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief Creates and configures socket for UDP or TCP
 * @param flag SOCK_DGRAM for UDP, SOCK_STREAM for TCP
 * @return File descriptor for new socket, ERROR on failure
 */
int socket_setup(int flag){
    struct addrinfo hints, *res;

    int sck = socket(AF_INET, flag, 0);
    if (sck < 0) {
        perror("Socket creation failed"); 
        return ERROR;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = flag; // type of socket
    hints.ai_flags = AI_PASSIVE; // server mode
    
    int errcode = getaddrinfo(NULL, set.port, &hints, &res);
    if (errcode != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errcode));
        close(sck);
        return ERROR;
    }

    if (bind(sck, res->ai_addr, res->ai_addrlen) < 0) {
        perror("Bind failed");
        freeaddrinfo(res);
        close(sck);
        return ERROR;  
    }
    freeaddrinfo(res);
    return sck;
}


/**
 * @brief Sets up UDP socket for receiving datagrams
 * @return SUCCESS if setup complete, ERROR on failure
 */
int udp_setup(){
    set.udp_socket = socket_setup(SOCK_DGRAM);
    if(set.udp_socket == ERROR){
        fprintf(stderr, "Failed to set up UDP socket\n");
        exit(EXIT_FAILURE);
    }
    printf("Servidor UDP à espera na porta %s...\n", set.port);
    return 0;
}

/**
 * @brief Sets up TCP socket for accepting connections
 * @return SUCCESS if setup complete, ERROR on failure
 */
int tcp_setup(){
    set.tcp_socket = socket_setup(SOCK_STREAM);
    if(set.tcp_socket == ERROR){
        fprintf(stderr, "Failed to set up TCP socket\n");
        exit(EXIT_FAILURE);
    }

    if (listen(set.tcp_socket, MAX_TCP_CLIENTS) != 0) {
        perror("Listen failed");
        close(set.tcp_socket);
        return ERROR;
    }
    printf("TCP server listening on port %s...\n", set.port);
    return SUCCESS;
}


/**
 * @brief Initializes complete server: sockets, file structure, fd_sets
 * Calls udp_setup(), tcp_setup() and initializes select() file descriptor sets
 */
void server_setup(){
    if (udp_setup() == ERROR) exit(EXIT_FAILURE);
    if (tcp_setup() == ERROR) {
        close(set.udp_socket);
        exit(EXIT_FAILURE);
    }

    set.timeout = (struct timeval){.tv_sec = 10, .tv_usec = 0};
    FD_ZERO(&set.read_fds);
    FD_SET(set.udp_socket, &set.read_fds);
    FD_SET(set.tcp_socket, &set.read_fds);
}
