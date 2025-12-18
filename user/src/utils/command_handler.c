#include "../../include/utils.h"
#include "../../include/client_data.h"
#include <stdio.h>
#include <stdlib.h>

ReplyStatus handle_response_code(char* resp, RequestType command, int parsed, int n, char* status) {
    // Problem parsing response
    if (parsed < 1) return STATUS_MALFORMED_RESPONSE;
    
    // Server did not even recognize the command
    if (identify_status_code(status) == STATUS_ERROR) return STATUS_ERROR;
    
    // Server returned the wrong command response
    const char* command_response = get_command_response_code(command);
    if (strcmp(resp, command_response)) return STATUS_UNEXPECTED_RESPONSE;
    
    // Not enough parts in response
    if (parsed < n) return STATUS_MALFORMED_RESPONSE;   

    return identify_status_code(status);
}

RequestType identify_command(char* command) {
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


ReplyStatus command_handler(RequestType command, char** cursor, int udp_fd,
     struct sockaddr_in* server_udp_addr) {
    
    socklen_t udp_addr_len = sizeof(*server_udp_addr);
    ReplyStatus status = STATUS_UNASSIGNED;
    char* extra_info = NULL;
    
    switch (command) {
        case LOGIN:
            status = login_handler(cursor, udp_fd, server_udp_addr, udp_addr_len);
            break;
        case CHANGEPASS:
            status = changepass_handler(cursor);
            break;
        case UNREGISTER:
            status = unregister_handler(cursor, udp_fd, server_udp_addr, udp_addr_len);
            break;
        case LOGOUT:
            status = logout_handler(cursor, udp_fd, server_udp_addr, udp_addr_len);
            break;
        case EXIT:
            if (is_logged_in) {
                status = logout_handler(cursor, udp_fd, server_udp_addr, udp_addr_len);
                print_result(LOGOUT, status, NULL);
            }
            printf("Exiting application.\n");
            close(udp_fd);
            exit(0);
            break;
        case CREATE:
            status = create_event_handler(cursor, &extra_info);
            break;
        case CLOSE:
            status = close_event_handler(cursor);
            break;
        case MYEVENTS:
            status = myevent_handler(cursor, udp_fd, server_udp_addr, udp_addr_len);
            break; 
        case LIST:
            status = list_handler(cursor);
            break;
        case SHOW:
            status = show_handler(cursor);
            break;
        case RESERVE:
            status = reserve_handler(cursor);
            break;
        case MYRESERVATIONS:
            status = myreservations_handler(cursor, udp_fd, server_udp_addr, udp_addr_len);
            break; 
        default:
            printf("Unknown command\n");
            break;
    }
    if(status != STATUS_UNASSIGNED) print_result(command, status, extra_info);
    return status;
}