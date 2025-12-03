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

int socket_setup(int flag){
    struct addrinfo hints, *res;

    int sck = socket(AF_INET, flag, 0);
    if (sck < 0) {
        perror("Socket creation failed"); 
        return -1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = flag; // type of socket
    hints.ai_flags = AI_PASSIVE; // server mode
    
    int errcode = getaddrinfo(NULL, settings.port, &hints, &res);
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


int udp_setup(){
    settings.udp_socket = socket_setup(SOCK_DGRAM);
    if(settings.udp_socket == ERROR){
        fprintf(stderr, "Failed to set up UDP socket\n");
        exit(EXIT_FAILURE);
    }
    printf("Servidor UDP à espera na porta %s...\n", settings.port);
    return 0;
}

int tcp_setup(){
    settings.tcp_socket = socket_setup(SOCK_STREAM);
    if(settings.tcp_socket == ERROR){
        fprintf(stderr, "Failed to set up TCP socket\n");
        exit(EXIT_FAILURE);
    }

    if (listen(settings.tcp_socket, MAX_TCP_CLIENTS) != 0) { // TODO: quantas connections em espera? - neste momento aceita 5
        perror("Listen failed");
        close(settings.tcp_socket);
        return ERROR;
    }
    printf("TCP server listening on port %s...\n", settings.port);
    return 0;
}



void server_setup(){

    udp_setup();
    tcp_setup();

    settings.timeout = (struct timeval){.tv_sec = 10, .tv_usec = 0};
    FD_ZERO(&settings.read_fds);
    FD_SET(settings.udp_socket, &settings.read_fds);
    FD_SET(settings.tcp_socket, &settings.read_fds);

}
