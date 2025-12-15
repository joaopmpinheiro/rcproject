#include "../../include/utils.h"
#include "../../include/client_data.h"
#include <stdio.h>
#include <stdlib.h>

ReplyStatus handle_response_code(char* resp, RequestType command, int parsed, int n, char* status) {
    // Problem parsing response
    if (parsed < 1) return STATUS_MALFORMED_RESPONSE;
    
    // Server did not even recognize the command
    if (parse_status_code(status) == STATUS_ERROR) return STATUS_ERROR;
    
    // Server returned the wrong command response
    const char* command_response = get_command_response_code(command);
    if (strcmp(resp, command_response)) return STATUS_UNEXPECTED_RESPONSE;
    
    // Not enough parts in response
    if (parsed < n) return STATUS_MALFORMED_RESPONSE;   

    return parse_status_code(status);
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


const char* get_command_name(RequestType command) {
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

const char* get_command_code(RequestType command) {
    switch (command) {
        case LOGIN: return "LIN";
        case CHANGEPASS: return "CPS";
        case UNREGISTER: return "UNR";
        case LOGOUT: return "LOU";
        case CREATE: return "CRE";
        case CLOSE: return "CLS";
        case MYEVENTS: return "LME";
        case LIST: return "LST";
        case SHOW: return "SED";
        case RESERVE: return "RID";
        case MYRESERVATIONS: return "LMR";
        default: return "UNK";
    }
}

const char* get_command_response_code(RequestType command) {
    switch (command) {
        case LOGIN: return "RLI";
        case CHANGEPASS: return "RCP";
        case UNREGISTER: return "RUR";
        case LOGOUT: return "RLO";
        case CREATE: return "RCE";
        case MYEVENTS: return "RME";
        case MYRESERVATIONS: return "RMR";
        case CLOSE: return "RCL";
        case LIST: return "RLS";
        case SHOW: return "RSE";
        case RESERVE: return "RRI";
        default: return "UNK";
    }
}

RequestType identify_command_response(char* command) {
    if (strcmp(command, "RLI") == 0) return LOGIN;
    if (strcmp(command, "RCP") == 0) return CHANGEPASS;
    if (strcmp(command, "RUR") == 0) return UNREGISTER;
    if (strcmp(command, "RLO") == 0) return LOGOUT;
    if (strcmp(command, "REX") == 0) return EXIT;
    if (strcmp(command, "RCE") == 0) return CREATE;
    if (strcmp(command, "RCL") == 0) return CLOSE;
    if (strcmp(command, "RME") == 0) return MYEVENTS;
    if (strcmp(command, "RLS") == 0) return LIST;
    if (strcmp(command, "RSE") == 0) return SHOW;
    if (strcmp(command, "RRI") == 0) return RESERVE;
    if (strcmp(command, "RMR") == 0) return MYRESERVATIONS;
    if(strcmp(command, "ERR") == 0) return ERROR_REQUEST;
    return UNKNOWN;
}

ReplyStatus parse_status_code(const char* status) {
    if (strcmp(status, "ERR") == 0) return STATUS_ERROR;
    if (strcmp(status, "OK") == 0) return STATUS_OK;
    if (strcmp(status, "NOK") == 0) return STATUS_NOK;
    if (strcmp(status, "REG") == 0) return STATUS_REGISTERED;
    if (strcmp(status, "NLG") == 0) return STATUS_NOT_LOGGED_IN;
    if (strcmp(status, "WRP") == 0) return STATUS_WRONG_PASSWORD;
    if (strcmp(status, "UNR") == 0) return STATUS_USER_NOT_REGISTERED;
    if (strcmp(status, "NID") == 0) return STATUS_USER_NOT_FOUND;
    if (strcmp(status, "NOE") == 0) return STATUS_NO_EVENT_ID;
    if (strcmp(status, "SLD") == 0) return STATUS_EVENT_SOLD_OUT;
    if (strcmp(status, "PST") == 0) return STATUS_PAST_EVENT;
    if (strcmp(status, "CLO") == 0) return STATUS_EVENT_CLOSED;
    return STATUS_UNEXPECTED_RESPONSE;
}

void command_handler(RequestType command, char** cursor, int udp_fd,
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
            // Handle create event
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