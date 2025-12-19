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

#include "../include/utils.h"
#include "../include/client_data.h"
#include "../common/parser.h"

char current_uid[UID_LENGTH + 1] = "";
char current_password[PASSWORD_LENGTH + 1] = "";
int is_logged_in = LOGGED_OUT;

char IP[MAX_HOSTNAME_LENGTH] = DEFAULT_IP;
char PORT[6] = DEFAULT_PORT;
volatile sig_atomic_t stop = 0;


void sig_detected(int signum) {
    (void)signum;
    stop = 1;
}
    
void parse_arguments(int argc, char *argv[]) {   
    int opt;
    while ((opt = getopt(argc, argv, "p:n:")) != -1) {
        switch (opt) {
            case 'p':
                if (optarg[0] == '-') {
                    fprintf(stdout, "Error: -p requires a port number\n");
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                if(!is_valid_port(optarg)) {
                    fprintf(stdout, "Error: Invalid port number\n");
                    exit(EXIT_FAILURE);
                }
                strcpy(PORT, optarg);
                break;
            case 'n':
                if (optarg[0] == '-') {
                    fprintf(stdout, "Error: -n requires a hostname or IP\n");
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                if (strlen(optarg) >= sizeof(IP)) {
                    fprintf(stdout, "Error: Hostname too long\n");
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
    signal(SIGINT, sig_detected);
    signal(SIGPIPE, SIG_IGN); 
    parse_arguments(argc, argv);
    
    struct sockaddr_in server_udp_addr;
    int udp_fd = setup_udp(IP, PORT, &server_udp_addr);
    if (udp_fd == ERROR) {
        fprintf(stderr, "UDP setup failed\n");
        return ERROR;
    }

    char input_buffer[1024] = {0}; //FIXME: tamanho do buffer
    char cmd[64] = {0};
    RequestType command;
    ReplyStatus status;

    while (!stop) {
        printf("> ");
        fflush(stdout);

        // Read user input
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) break;

        memset(cmd, 0, sizeof(cmd));

        // Parse command and arguments
        char* cursor = input_buffer;
        if (parse_cmd(&cursor, cmd) == ERROR) continue;
        command = identify_command(cmd);
        status = command_handler(command, &cursor, udp_fd, &server_udp_addr);
        
        // TODO: mais algum devia dar paragem?
        if (status == STATUS_MALFORMED_RESPONSE ||
            status == STATUS_MALFORMED_COMMAND ||
            status == STATUS_SEND_FAILED ||
            status == STATUS_RECV_FAILED ||
            status == STATUS_UNEXPECTED_RESPONSE ||
            status == STATUS_UNEXPECTED_STATUS) {
            close(udp_fd);
            exit(1);
        }
    }
    if(stop == 1) {
        printf("\nExiting...\n");
        fflush(stdout);
    }
    close(udp_fd);
    return 0;
}