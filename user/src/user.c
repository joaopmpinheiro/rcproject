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

#include "../include/utils.h"
#include "../include/client_data.h"
#include "../common/parser.h"

char current_uid[UID_LENGTH + 1] = "";
char current_password[PASSWORD_LENGTH + 1] = "";
int is_logged_in = LOGGED_OUT;

char IP[MAX_HOSTNAME_LENGTH] = DEFAULT_IP;
char PORT[6] = DEFAULT_PORT;

    
void parse_arguments(int argc, char *argv[]) {   
    int opt;
    while ((opt = getopt(argc, argv, "p:n:")) != -1) {
        switch (opt) {
            case 'p':
                if (optarg[0] == '-') {
                    fprintf(stderr, "Error: -p requires a port number\n");
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                if(!is_valid_port(optarg)) {
                    fprintf(stderr, "Error: Invalid port number\n");
                    exit(EXIT_FAILURE);
                }
                strcpy(PORT, optarg);
                break;
            case 'n':
                if (optarg[0] == '-') {
                    fprintf(stderr, "Error: -n requires a hostname or IP\n");
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                if (strlen(optarg) >= sizeof(IP)) {
                    fprintf(stderr, "Error: Hostname too long\n");
                    exit(EXIT_FAILURE);
                }
                strcpy(IP, optarg);
                break;
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char* argv[]) {

    parse_arguments(argc, argv);
    
    struct sockaddr_in server_udp_addr;
    int udp_fd = setup_udp(IP, PORT, &server_udp_addr);
    if (udp_fd == ERROR) {
        fprintf(stderr, "UDP setup failed\n");
        return ERROR;
    }

    char input_buffer[1024] = {0};
    char cmd[36] = {0};

    while (1) {
        printf("> ");
        // Read user input
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) break;

        memset(cmd, 0, sizeof(cmd));

        // Parse command and arguments
        //sscanf(input_buffer, "%s %[^\n]", command_buffer, command_arg_buffer);

        char* cursor = input_buffer;
        if (parse_cmd(&cursor, cmd) == ERROR) continue;
        fprintf(stdout, "Command received: %s\n", cmd);
        CommandType command = identify_command(cmd);
        command_handler(command, &cursor, udp_fd, &server_udp_addr);
    }
    return 0;
}