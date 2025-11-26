#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "command_handlers.h"
#include "verifications.h"

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

int login_handler(char* request, char* args) {
    // FIXME as verificações dão return a -1 e já dão print, 
    // pode dar problema depois com erros na implementação do envio do pedido
    char REQUEST_TYPE[3] = "LIN";
    char uid[6];
    char password[8];

    if (!verify_argument_count(args, 2)) {
        printf("Invalid login arguments count, expected 2 arguments: <uid> <password>\n");
        return -1;
    }
    sscanf(args, "%s %s", uid, password);
    if (!verify_uid_format(uid)) {
        printf("Invalid UID format, UID must be exactly 6 digits\n");
        return -1;
    }
    if (!verify_password_format(password)) {
        printf("Invalid password format, password must be exactly 8 alphanumeric characters\n");
        return -1;
    }

    char REQUEST[256];
    snprintf(REQUEST, sizeof(REQUEST), "%s %s %s", REQUEST_TYPE, uid, password);
    // TODO send request to server and then process response
    // DOUBT if the request protocol isn't right what should be done? Assume it's always right from the server?
    return 0;
}

void command_handler(CommandType command, char* args) {
    char request[256];
    switch (command) {
        case LOGIN:
            // Handle login
            login_handler(request, args);
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
            // FIXME, APAGUEI O RETURN -1;
            break;
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