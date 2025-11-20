#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define BASE_PORT 58000
#define GROUP_NUMBER 32
#define DEFAULT_PORT "58032"
#define DEFAULT_IP "127.0.0.1"
#define PORTMAX 65535
#define PORTMIN 0

typedef enum CommandType {
    LOGIN,
    CHANGEPASS,
    UNREGISTER,
    LOGOUT,
    EXIT,
    CREATE,
    CLOSE,
    MYEVENTS,
    LIST,
    SHOW,
    RESERVE,
    MYRESERVATIONS,
    UNKNOWN,
} CommandType;

char* get_server_ip(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            return argv[i + 1];
        }
    }
    
    return DEFAULT_IP;
}

char* get_server_port(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            char* port = argv[i + 1];
            int port_num = atoi(port);
            if (port_num < PORTMIN || port_num > PORTMAX) {
                return NULL;
            }
            return port;
        }
    }
    
    return DEFAULT_PORT;
}

CommandType identify_command(char* command) {
    if (strcmp(command, "login") == 0) {
        return LOGIN;
    } else if (strcmp(command, "changePass") == 0) {
        return CHANGEPASS;
    } else if (strcmp(command, "unregister") == 0) {
        return UNREGISTER;
    } else if (strcmp(command, "logout") == 0) {
        return LOGOUT;
    } else if (strcmp(command, "exit") == 0) {
        return EXIT;
    } else if (strcmp(command, "create") == 0) {
        return CREATE;
    } else if (strcmp(command, "close") == 0) {
        return CLOSE;
    } else if (strcmp(command, "myevents") == 0 || strcmp(command, "mye") == 0) {
        return MYEVENTS;
    } else if (strcmp(command, "list") == 0) {
        return LIST;
    } else if (strcmp(command, "show") == 0) {
        return SHOW;
    } else if (strcmp(command, "reserve") == 0) {
        return RESERVE;
    } else if (strcmp(command, "myreservations") == 0 || strcmp(command, "myr") == 0) {
        return MYRESERVATIONS;
    } else {
        return UNKNOWN;
    }
}

int main(int argc, char* argv[]) {

    char* SERVER_IP = get_server_ip(argc, argv);
    char* SERVER_PORT = get_server_port(argc, argv);
    
    if (SERVER_IP == NULL || SERVER_PORT == NULL) {
        return -1;
    }
    
    struct addrinfo hints, *res;
    int fd, errcode;
    ssize_t n;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) return -1;
    
    struct timeval timeout = {5, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    errcode = getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &res);
    if (errcode != 0) {
        close(fd);
        return -1;
    }

    char input_buffer[256] = {0};
    char command_buffer[256] = {0};
    char command_arg_buffer[256] = {0};

    fflush(stdout);
    while (1) {
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
            freeaddrinfo(res);
            close(fd);
            return -1;
        }

        printf("%s", input_buffer);

        scanf("%s %s", command_buffer, command_arg_buffer);

        CommandType command = identify_command(command_buffer);
        
        switch (command) {
            case LOGIN:
                // Handle login
                break;
            case CHANGEPASS:
                // Handle change password
                break;
            case UNREGISTER:
                // Handle unregister
                break;
            case LOGOUT:
                // Handle logout
                break;
            case EXIT:
                // Handle exit
                // FIXME, o chat disse para dar close ao fd e freeaddrinfo ao res
                return 0;
            case CREATE:
                // Handle create event
                break;
            case CLOSE:
                // Handle close event
                break;
            case MYEVENTS:
                // Handle my events
                break;
            case LIST:
                // Handle list events
                break;
            case SHOW:
                // Handle show event details
                break;
            case RESERVE:
                // Handle reserve event
                break;
            case MYRESERVATIONS:
                // Handle my reservations
                break;
            case UNKNOWN:
            default:
                printf("Unknown command\n");
                break;
        }
    }

    freeaddrinfo(res);
    return 0;
}