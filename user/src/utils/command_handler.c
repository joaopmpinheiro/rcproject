#include "../../include/command_handlers.h"
#include "../../include/client_data.h"
#include <stdio.h>
#include <stdlib.h>

ReplyStatus handle_response_code(char* resp, char* command, int parsed, int n, char* status) {
    if (parsed < 1) return STATUS_MALFORMED_RESPONSE;
    if (strcmp(resp, "ERR"))  return STATUS_ERROR;
    if (strcmp(resp, command))  return STATUS_UNEXPECTED_RESPONSE;
    if (parsed < n) return STATUS_MALFORMED_RESPONSE;

    ReplyStatus reply_status = parse_status_code(status);
    return reply_status;
}


CommandType identify_command(char* command) {
    if (strcmp(command, "login") == 0) return LOGIN;
    if (strcmp(command, "changePass") == 0) return CHANGEPASS;
    if (strcmp(command, "unregister") == 0) return UNREGISTER;
    if (strcmp(command, "logout") == 0) return LOGOUT;
    if (strcmp(command, "exit") == 0) return EXIT;
    if (strcmp(command, "create") == 0) return CREATE;
    if (strcmp(command, "close") == 0) return CLOSE;
    if (strcmp(command, "myevents") == 0 || strcmp(command, "mye") == 0) return MYEVENTS;
    if (strcmp(command, "list") == 0) return LIST;
    if (strcmp(command, "show") == 0) return SHOW;
    if (strcmp(command, "reserve") == 0) return RESERVE;
    if (strcmp(command, "myreservations") == 0 || strcmp(command, "myr") == 0) return MYRESERVATIONS;
    return UNKNOWN;
}

const char* get_command_name(CommandType command) {
    switch (command) {
        case LOGIN: return "Login";
        case CHANGEPASS: return "Change password";
        case UNREGISTER: return "Unregister";
        case LOGOUT: return "Logout";
        case EXIT: return "Exit";
        case CREATE: return "Create";
        case CLOSE: return "Close";
        case MYEVENTS: return "My events";
        case LIST: return "List";
        case SHOW: return "Show";
        case RESERVE: return "Reserve";
        case MYRESERVATIONS: return "My reservations";
        default: return "Unknown";
    }
}

ReplyStatus parse_status_code(const char* status) {
    if (strcmp(status, "OK") == 0) return STATUS_OK;
    if (strcmp(status, "NOK") == 0) return STATUS_NOK;
    if (strcmp(status, "REG") == 0) return STATUS_REGISTERED;
    if (strcmp(status, "NLG") == 0) return STATUS_NOT_LOGGED_IN;
    if (strcmp(status, "WRP") == 0) return STATUS_WRONG_PASSWORD;
    if (strcmp(status, "UNR") == 0) return STATUS_USER_NOT_REGISTERED;
    if (strcmp(status, "NID") == 0) return STATUS_USER_NOT_FOUND;

    if (strcmp(status, "ERR") == 0) return STATUS_ERROR;
    return STATUS_UNEXPECTED_RESPONSE;
}

void command_handler(CommandType command, char* args, int udp_fd,
     struct sockaddr_in* server_udp_addr) {
    
    socklen_t udp_addr_len = sizeof(*server_udp_addr);
    ReplyStatus status = STATUS_UNASSIGNED;
    char* extra_info = NULL;
    
    switch (command) {
        case LOGIN:
            status = login_handler(args, udp_fd, server_udp_addr, udp_addr_len);
            break;
        case CHANGEPASS:
            // Handle change password
            break;
        case UNREGISTER:
            status = unregister_handler(args, udp_fd, server_udp_addr, udp_addr_len);
            break;
        case LOGOUT:
            status = logout_handler(args, udp_fd, server_udp_addr, udp_addr_len);
            break;
        case EXIT:
            if (is_logged_in) {
                status = logout_handler(args, udp_fd, server_udp_addr, udp_addr_len);
                print_result(LOGOUT, status, NULL);
            }
            printf("Exiting application.\n");
            close(udp_fd);
            exit(0);
            break;
        case CREATE:
            // Handle create event
            status = create_event_handler(args, &extra_info);
            break;
        case CLOSE:
            // Handle close event
            break;
        case MYEVENTS:
            status = myevent_handler(args, udp_fd, server_udp_addr, udp_addr_len);
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
        default:
            printf("Unknown command\n");
            break;
    }
    if(status != STATUS_UNASSIGNED) print_result(command, status, extra_info);
}