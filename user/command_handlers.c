#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "command_handlers.h"
#include "verifications.h"
#include "client_data.h"

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

int login_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len) {
    // FIXME as verificações dão return a -1 e já dão print, 
    // pode dar problema depois com erros na implementação do envio do pedido
    char uid[7];
    char password[9];
    ssize_t n;

    if (!verify_argument_count(args, 2)) {
        printf("Invalid login argument count, expected 2 arguments: <uid> <password>\n");
        return -1;
    }

    sscanf(args, "%s %s", uid, password);

    if (!verify_uid_format(uid)) {
        printf("Login failed: Invalid UID format, UID must be exactly 6 digits\n");
        return -1;
    }
    if (!verify_password_format(password)) {
        printf("Login failed: Invalid password format, password must be exactly 8 alphanumeric characters\n");
        return -1;
    }

    char request[256];

    // PROTOCOL: LIN <uid> <password>
    snprintf(request, sizeof(request), "LIN %s %s\n", uid, password);

    n = sendto(udp_fd, request, strlen(request), 0,
         (struct sockaddr*)server_udp_addr, udp_addr_len);

    if (n == -1) {
        perror("Failed to send login request");
        return -1;
    }

    char response[256];
    n = recvfrom(udp_fd, response, sizeof(response) - 1, 0,
         NULL, NULL);
    if (n == -1) {
        perror("Failed to receive login response");
        return -1;
    }

    response[n] = '\0';

    char response_code[4];
    char reply_status[4];

    int parsed = sscanf(response, "%3s %3s", response_code, reply_status);

    if (parsed < 2) {
        printf("Error: Malformed server response\n");
        return -1;
    }
    
    int status = 0;

    if (strcmp(response_code, "RLI") == 0) {
        if (strcmp(reply_status, "OK") == 0) {
            printf("Login successfull\n");
            status = 1;
        } else if (strcmp(reply_status, "NOK") == 0) {
            printf("Login failed: Wrong password\n");
        } else if (strcmp(reply_status, "REG") == 0) {
            printf("Login successfull: New user registered\n");
            status = 1;
        } else {
            printf("Login failed: Unknown reply status\n");
        }
    } else {
        printf("Login failed: Unexpected response code\n");
    }

    if (status == 1) {
        is_logged_in = 1;
        strcpy(current_password, password);
        strcpy(current_uid, uid);
    }

    return 0;
}

int unregister_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len) {
    
    ssize_t n;

    if (!verify_argument_count(args, 0)) {
        printf("Invalid unregister argument count, no arguments expected\n");
        return -1;
    }

    if (!is_logged_in) {
        printf("Unregister failed: User not logged in\n");
        return -1;
    }

    char request[256];

    // PROTOCOL: UNR <uid> <password>
    snprintf(request, sizeof(request), "UNR %s %s\n", current_uid, current_password);

    n = sendto(udp_fd, request, strlen(request), 0,
         (struct sockaddr*)server_udp_addr, udp_addr_len);

    if (n == -1) {
        perror("Failed to send unregister request");
        return -1;
    }

    char response[256];
    n = recvfrom(udp_fd, response, sizeof(response) - 1, 0,
         NULL, NULL);
    if (n == -1) {
        perror("Failed to receive unregister response");
        return -1;
    }

    response[n] = '\0';

    char response_code[4];
    char reply_status[4];

    int parsed = sscanf(response, "%3s %3s", response_code, reply_status);

    if (parsed < 2) {
        printf("Error: Malformed server response\n");
        return -1;
    }
    
    int status = 0;

    if (strcmp(response_code, "RUR") == 0) {
        if (strcmp(reply_status, "OK") == 0) {
            printf("Unregister successfull: User unregistered\n");
                status = 1;
        } else if (strcmp(reply_status, "NOK") == 0) {
            printf("Unregister failed: User not logged in\n");
        } else if (strcmp(reply_status, "WRP") == 0) {
            printf("Unregister failed: Wrong password\n");
        } else if (strcmp(reply_status, "UNR") == 0) {
            printf("Unregister failed: User not registered\n");
        } 
        else {
            printf("Unregister failed: Unknown reply status\n");
        }
    } else {
        printf("Unregister failed: Unexpected response code\n");
    }

    if (status == 1) {
        is_logged_in = 0;
        memset(current_password, 0, sizeof(current_password));
        memset(current_uid, 0, sizeof(current_uid));
    }

    return 0;
}

int logout_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len) {

    ssize_t n;

    if (!verify_argument_count(args, 0)) {
        printf("Invalid logout argument count, no arguments expected\n");
        return -1;
    }

    if (!is_logged_in) {
        printf("Logout failed: User not logged in\n");
        return -1;
    }
    char request[256];

    // PROTOCOL: LOU <uid> <password>
    snprintf(request, sizeof(request), "LOU %s %s\n", current_uid, current_password);

    n = sendto(udp_fd, request, strlen(request), 0,
         (struct sockaddr*)server_udp_addr, udp_addr_len);

    if (n == -1) {
        perror("Failed to send logout request");
        return -1;
    }

    char response[256];
    n = recvfrom(udp_fd, response, sizeof(response) - 1, 0,
         NULL, NULL);
    if (n == -1) {
        perror("Failed to receive logout response");
        return -1;
    }

    response[n] = '\0';

    char response_code[4];
    char reply_status[4];

    int parsed = sscanf(response, "%3s %3s", response_code, reply_status);

    if (parsed < 2) {
        printf("Error: Malformed server response\n");
        return -1;
    }

    int status = 0;

    if (strcmp(response_code, "RLO") == 0) {
        if (strcmp(reply_status, "OK") == 0) {
            printf("Logout successfull\n");
                status = 1;
        } else if (strcmp(reply_status, "NOK") == 0) {
            printf("Logout failed: User not logged in\n");
        } else if (strcmp(reply_status, "WRP") == 0) {
            printf("Logout failed: Wrong password\n");
        } else if (strcmp(reply_status, "UNR") == 0) {
            printf("Logout failed: User not registered\n");
        } 
        else {
            printf("Logout failed: Unknown reply status\n");
        }
    } else {
        printf("Logout failed: Unexpected response code\n");
    }

    if (status == 1) {
        is_logged_in = 0;
        memset(current_password, 0, sizeof(current_password));
        memset(current_uid, 0, sizeof(current_uid));
    }

    return 0;
}

int myevent_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len) {

    ssize_t n;

    if (!verify_argument_count(args, 0)) {
        printf("Invalid myevents argument count, no arguments expected\n");
        return -1;
    }

    if (!is_logged_in) {
        printf("Myevents failed: User not logged in\n");
        return -1;
    }

    char request[256];

    // PROTOCOL: LME <uid> <password>
    snprintf(request, sizeof(request), "LME %s %s\n", current_uid, current_password);

    n = sendto(udp_fd, request, strlen(request), 0,
         (struct sockaddr*)server_udp_addr, udp_addr_len);

    if (n == -1) {
        perror("Failed to send myevents request");
        return -1;
    }

    char response[256];
    n = recvfrom(udp_fd, response, sizeof(response) - 1, 0,
         NULL, NULL);
    if (n == -1) {
        perror("Failed to receive myevents response");
        return -1;
    }

    response[n] = '\0';

    char response_code[4];
    char reply_status[4];

    int parsed = sscanf(response, "%3s %3s", response_code, reply_status);

    if (parsed < 2) {
        printf("Error: Malformed server response\n");
        return -1;
    }

    int status = 0;

    if (strcmp(response_code, "RME") == 0) {
        if (strcmp(reply_status, "OK") == 0) {
            status = 1;
        } else if (strcmp(reply_status, "NOK") == 0) {
            printf("Myevents failed: User has no events\n");
        } else if (strcmp(reply_status, "NLG") == 0) {
            printf("Myevents failed: User not logged in\n");
        } else if (strcmp(reply_status, "WRP") == 0) {
            printf("Myevents failed: Wrong password\n");
        } else {
            printf("Myevents failed: Unknown reply status\n");
        }
    } else {
        printf("Myevents failed: Unexpected response code\n");
    }

    if (status == 1) {
        
    }

    return 0;
}

void command_handler(CommandType command, char* args, int udp_fd,
     struct sockaddr_in* server_udp_addr) {
    
    socklen_t udp_addr_len = sizeof(*server_udp_addr);
    switch (command) {
        case LOGIN:
            // Handle login
            login_handler(args, udp_fd, server_udp_addr, udp_addr_len);
            break;
        case CHANGEPASS:
            // Handle change password
            break;
        case UNREGISTER:
            unregister_handler(args, udp_fd, server_udp_addr, udp_addr_len);
            break;
        case LOGOUT:
            logout_handler(args, udp_fd, server_udp_addr, udp_addr_len);
            break;
        case EXIT:
            // Handle exit
            // FIXME, o chat disse para dar close ao fd e freeaddrinfo ao res
            // FIXME, APAGUEI O RETURN -1;
            break;
        case CREATE:
            // Handle create event
            break;
        case CLOSE:
            // Handle close event
            break;
        case MYEVENTS:
            myevent_handler(args, udp_fd, server_udp_addr, udp_addr_len);
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