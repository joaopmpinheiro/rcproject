#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>   // stat, S_ISREG
#include <unistd.h>    // access, R_OK

#include "utils.h"
#include "../../common/parser.h"
#include "../../common/common.h"

#include "client_data.h"

int verify_file(char* file_name) {
    struct stat st;
    if (stat(file_name, &st) != 0)
        return STATUS_FILE_NOT_FOUND;

    if (access(file_name, R_OK) != 0) 
        return STATUS_FILE_READ_ERROR;

    if (st.st_size > MAX_FILE_SIZE)
        return STATUS_FILE_SIZE_EXCEEDED;
    return VALID;
}

// --------------- UDP ----------------
ReplyStatus login_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
            socklen_t udp_addr_len) {
    if (is_logged_in) return STATUS_ALREADY_LOGGED_IN;
                
    // Verify arguments
    ReplyStatus status;
    char uid[32], password[32];
    status = parse_login(cursor, uid, password);
    if (status != STATUS_UNASSIGNED) return status;

    // PROTOCOL: LIN <uid> <password>
    char request[256], response[256];
    ssize_t response_size = 256;
    
    snprintf(request, sizeof(request), "LIN %s %s\n", uid, password);

    // Send request to server and receive response
    status = udp_send_receive(udp_fd, server_udp_addr, udp_addr_len,
                                request, response, response_size);
   
    if (status != STATUS_UNASSIGNED) return status;
    
    // Protocol: RLI <status>
    char *resp_cursor = response;
    status =  parse_response_header(&resp_cursor, LOGIN);
    if(!is_end_of_message(&resp_cursor)) return STATUS_MALFORMED_RESPONSE;
    // Expected responses: OK / NOK / REG
    if(status != STATUS_OK &&
       status != STATUS_NOK &&
       status != STATUS_REGISTERED &&
       status != STATUS_ERROR &&
       status != STATUS_SEND_FAILED &&
       status != STATUS_RECV_FAILED &&
       status != STATUS_MALFORMED_RESPONSE) {
        return STATUS_UNEXPECTED_STATUS;
    }
    if (status != STATUS_OK && status != STATUS_REGISTERED) return status;

    // Update global state on successful login
    if ((status == STATUS_OK) ||
         status == STATUS_REGISTERED) {
        is_logged_in = 1;
        strcpy(current_password, password);
        strcpy(current_uid, uid);
    }
    return status;
}

ReplyStatus unregister_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
                                socklen_t udp_addr_len) {
    // Verify arguments
    if (!is_padded_end_of_message(cursor)) return STATUS_INVALID_ARGS;
    if (!is_logged_in) return STATUS_NOT_LOGGED_IN_LOCAL;

    // PROTOCOL: UNR <uid> <password>
    char request[256], response[256];
    ssize_t response_size = 256;
    snprintf(request, sizeof(request), "UNR %s %s\n", current_uid, current_password);

    // Send request to server and receive response
    ReplyStatus status = udp_send_receive(udp_fd, server_udp_addr, udp_addr_len,
                                request, response, response_size);
    if (status != STATUS_UNASSIGNED) return status;
    
    // Parse response
    char *resp_cursor = response;
    status = parse_response_header(&resp_cursor, UNREGISTER);
    if(!is_end_of_message(&resp_cursor)) return STATUS_MALFORMED_RESPONSE;    
    // Expected responses: OK / NOK / UNR / WRP
    if(status != STATUS_OK &&
       status != STATUS_NOK &&
       status != STATUS_USER_NOT_REGISTERED &&
       status != STATUS_WRONG_PASSWORD &&
       status != STATUS_ERROR &&
       status != STATUS_SEND_FAILED &&
       status != STATUS_RECV_FAILED &&
       status != STATUS_MALFORMED_RESPONSE) {
        return STATUS_UNEXPECTED_RESPONSE;
    }

    // Clear global state on successful unregister
    if (status == STATUS_OK || 
        status == STATUS_USER_NOT_REGISTERED || 
        status == STATUS_NOK ||
        status == STATUS_WRONG_PASSWORD){
        is_logged_in = 0;
        memset(current_password, 0, sizeof(current_password));
        memset(current_uid, 0, sizeof(current_uid));
    }
    return status;
}

ReplyStatus logout_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len) {

    // Verify arguments
    if(!is_end_of_message(cursor)) return STATUS_INVALID_ARGS;
    if (!is_logged_in) return STATUS_NOT_LOGGED_IN_LOCAL;

    char request[256], response[256];
    ssize_t response_size = 256;

    // PROTOCOL: LOU <uid> <password>
    snprintf(request, sizeof(request), "LOU %s %s\n", current_uid, current_password);

    // Send request to server and receive response
    ReplyStatus status = udp_send_receive(udp_fd, server_udp_addr, udp_addr_len,
                                request, response, response_size);
    if (status != STATUS_UNASSIGNED) return status;

    // Parse response
    char *resp_cursor = response;
    status = parse_response_header(&resp_cursor, LOGOUT);
    if(!is_end_of_message(&resp_cursor)) return STATUS_MALFORMED_RESPONSE;

    // Expected responses: OK / NOK / UNR / WRP
    if(status != STATUS_OK &&
       status != STATUS_NOK &&
       status != STATUS_USER_NOT_REGISTERED &&
       status != STATUS_WRONG_PASSWORD &&
       status != STATUS_ERROR &&
       status != STATUS_SEND_FAILED &&
       status != STATUS_RECV_FAILED &&
       status != STATUS_MALFORMED_RESPONSE) {
        return STATUS_UNEXPECTED_RESPONSE;
    }

    // Clear global state if we know we are logged out or user doesn't exist
    if (status == STATUS_OK ||
        status == STATUS_NOK ||
        status == STATUS_USER_NOT_REGISTERED ||
        status == STATUS_WRONG_PASSWORD) {
        is_logged_in = 0;
        memset(current_password, 0, sizeof(current_password));
        memset(current_uid, 0, sizeof(current_uid));
    }

    return status;
}

ReplyStatus myevent_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
                            socklen_t udp_addr_len) {
    ssize_t n;
    if(!is_end_of_message(cursor)) return STATUS_INVALID_ARGS;
    if (!is_logged_in) return STATUS_NOT_LOGGED_IN_LOCAL;

    // PROTOCOL: LME <uid> <password>
    char request[256];
    snprintf(request, sizeof(request), "LME %s %s\n", current_uid, current_password);
    if (sendto(udp_fd, request, strlen(request), 0, (struct sockaddr*)server_udp_addr,
                udp_addr_len) == ERROR) return STATUS_SEND_FAILED;

    char response[8192];
    n = recvfrom(udp_fd, response, sizeof(response) - 1, 0, NULL, NULL);
    if (n < 0) return STATUS_RECV_FAILED;
    response[n] = '\0';
     
    // Parse response
    char *resp_cursor = response;
    ReplyStatus status = parse_response_header(&resp_cursor, MYEVENTS);
    
    // Expected responses: OK / NOK / NLG / WRP
    if(status != STATUS_OK &&
       status != STATUS_NOK &&
       status != STATUS_NOT_LOGGED_IN &&
       status != STATUS_WRONG_PASSWORD &&
       status != STATUS_ERROR &&
       status != STATUS_SEND_FAILED &&
       status != STATUS_RECV_FAILED &&
       status != STATUS_MALFORMED_RESPONSE) {
        return STATUS_UNEXPECTED_RESPONSE;
    }

    // IF wrong password or not logged in, clear global state
    if (is_end_of_message(&resp_cursor) && 
        (status == STATUS_WRONG_PASSWORD ||
         status == STATUS_NOT_LOGGED_IN)){
            is_logged_in = 0;
            memset(current_password, 0, sizeof(current_password));
            memset(current_uid, 0, sizeof(current_uid));
    }
    if (status != STATUS_OK && is_end_of_message(&resp_cursor)) return status;   
    else if (is_end_of_message(&resp_cursor)) return STATUS_MALFORMED_RESPONSE; 

    char* event_list = response + 7;

    printf("\n%-5s %-10s\n", "EID", "State");
    printf("===================\n");

    char eid[4];
    int state;
    int offset = 0;
    int chars_read;
    while (sscanf(event_list + offset, " %3s %d%n", eid, &state, &chars_read) == 2) {
        // Validate EID format (3 digits)
        if (strlen(eid) != 3) {
            printf("Warning: Invalid EID format in response\n");
            break;
        }

        const char* state_str;
        switch (state) {
            case 0: state_str = "Past"; break;
            case 1: state_str = "Active"; break;
            case 2: state_str = "Sold out"; break;
            case 3: state_str = "Closed"; break;
            default: state_str = "Unknown"; break;
        }
        printf("%-5s %-10s\n", eid, state_str);
        offset += chars_read;
    }
    return STATUS_CUSTOM_OUTPUT;
}


ReplyStatus myreservations_handler(char** cursor, int udp_fd,
                                struct sockaddr_in* server_udp_addr,
                                socklen_t udp_addr_len) {
    ssize_t n;
    if(!is_end_of_message(cursor)) return STATUS_INVALID_ARGS;
    if (!is_logged_in) return STATUS_NOT_LOGGED_IN_LOCAL;

    // PROTOCOL: LMR <uid> <password>
    char request[256];
    snprintf(request, sizeof(request), "LMR %s %s\n", current_uid, current_password);

    if (sendto(udp_fd, request, strlen(request), 0, (struct sockaddr*)server_udp_addr,
                udp_addr_len) == ERROR) return STATUS_SEND_FAILED;

    char response[8192];
    n = recvfrom(udp_fd, response, sizeof(response) - 1, 0, NULL, NULL);
    if (n < 0) return STATUS_RECV_FAILED;
    response[n] = '\0';

    // PROTOCOL: RMR <status> [<event1ID date value> ...]
    char *resp_cursor = response;
    ReplyStatus status = parse_response_header(&resp_cursor, MYRESERVATIONS);
    
    // Expected responses: OK / NOK / NLG / WRP
    if(status != STATUS_OK &&
       status != STATUS_NOK &&
       status != STATUS_NOT_LOGGED_IN &&
       status != STATUS_WRONG_PASSWORD &&
       status != STATUS_ERROR &&
       status != STATUS_SEND_FAILED &&
       status != STATUS_RECV_FAILED &&
       status != STATUS_MALFORMED_RESPONSE) {
        return STATUS_UNEXPECTED_RESPONSE;
    }
    if (is_end_of_message(&resp_cursor) && 
        (status == STATUS_WRONG_PASSWORD ||
         status == STATUS_NOT_LOGGED_IN)){
            is_logged_in = 0;
            memset(current_password, 0, sizeof(current_password));
            memset(current_uid, 0, sizeof(current_uid));
    }
    if (!is_end_of_message(&resp_cursor)) return STATUS_MALFORMED_RESPONSE;
    if (status != STATUS_OK) return status;
    return show_myreservations(resp_cursor);
}


// ------------- TCP -------------
ReplyStatus changepass_handler(char** cursor) {
    // Verify arguments
    char new_password[PASSWORD_LENGTH + 1], old_password[PASSWORD_LENGTH + 1];
    ReplyStatus status = parse_change_password(cursor, old_password, new_password);
    if (status != STATUS_UNASSIGNED) return status;
    if (!is_logged_in) return STATUS_NOT_LOGGED_IN_LOCAL;

    // PROTOCOL: CHP <uid> <old_password> <new_password>
    char request[256], response[256];
    snprintf(request, sizeof(request), "CPS %s %s %s\n", current_uid, current_password, new_password);

    // Send request to server and receive response
    status = tcp_send_receive(request, response, 256);
    if (status != STATUS_UNASSIGNED) return status;

    char *resp_cursor = response;
    status = parse_response_header(&resp_cursor, CHANGEPASS);
    
    // Expected responses: OK / NOK / NLG / NID
    if (status != STATUS_OK &&
        status != STATUS_NOK &&
        status != STATUS_NOT_LOGGED_IN &&
        status != STATUS_USER_NOT_FOUND &&
        status != STATUS_ERROR &&
        status != STATUS_MALFORMED_RESPONSE) {
            return STATUS_UNEXPECTED_RESPONSE;
        }
        
    if(!is_end_of_message(&resp_cursor)) return STATUS_MALFORMED_RESPONSE;
     // If wrong password or not logged in, clear global state
    if (status == STATUS_NOT_LOGGED_IN || status == STATUS_USER_NOT_FOUND){
        is_logged_in = 0;
        memset(current_password, 0, sizeof(current_password));
        memset(current_uid, 0, sizeof(current_uid));
    }
    // Update global state on successful password change
    if (status == STATUS_OK) strcpy(current_password, new_password);
    return status;
}

ReplyStatus create_event_handler(char** cursor, char** extra_info) {
    char event_name[MAX_EVENT_NAME + 1], file_name[FILE_NAME_LENGTH + 1];
    char date[EVENT_DATE_LENGTH + 1], num_seats[4];
    *extra_info = NULL;
    
    if(!is_logged_in) return STATUS_NOT_LOGGED_IN_LOCAL;
    ReplyStatus status = parse_create_event(cursor, event_name, file_name, date, num_seats);
    if (status != STATUS_UNASSIGNED) return status;
    
    int tcp_fd = connect_tcp(IP, PORT);
    if (tcp_fd == -1) return STATUS_SEND_FAILED;

    // PROTOCOL: CRE <uid> <password> <name> <event_date> <attendance_size> <Fname> <Fsize> <Fdata>
    // Get file size
    struct stat st;
    stat(file_name, &st);
    long file_size = st.st_size;

    // Prepare request header
    char request_header[512];

    snprintf(request_header, sizeof(request_header), "CRE %s %s %s %s %s %s %ld ",
             current_uid, current_password, event_name, date, num_seats, file_name, file_size);
    
    // Send request header to server
    if (tcp_send_message(tcp_fd, request_header) == ERROR) {
        close(tcp_fd);
        return STATUS_SEND_FAILED;
    }

    // Send file to server
    if (tcp_send_file(tcp_fd, file_name) == ERROR) {
        close(tcp_fd);
        return STATUS_SEND_FAILED;
    }   

    // Read server response
    tcp_read(tcp_fd, request_header, sizeof(request_header));
    close(tcp_fd);
    char response_code[4], reply_status[4], eid[4];
    char *cursor_resp = request_header;
    
    if(get_next_arg(&cursor_resp, response_code) == ERROR)
    return STATUS_MALFORMED_RESPONSE;
    
    // Identify response type (check if it is ERR)
    RequestType resp_type = identify_command_response(response_code);
    if(is_end_of_message(&cursor_resp)){
        if(resp_type == ERROR_REQUEST) return STATUS_ERROR;
        else return STATUS_MALFORMED_RESPONSE;
    }

    // Extract reply status
    if(get_next_arg(&cursor_resp, reply_status) == ERROR)
        return STATUS_EVENT_SOLD_OUT;

    if(resp_type != CREATE) return STATUS_EVENT_RESERVATION_REJECTION;

    status = identify_status_code(reply_status);
    // Expected responses: OK / NOK / NLG / WRP
    if(status != STATUS_OK &&
       status != STATUS_NOK &&
       status != STATUS_NOT_LOGGED_IN &&
       status != STATUS_WRONG_PASSWORD &&
       status != STATUS_ERROR &&
       status != STATUS_MALFORMED_RESPONSE) {
        return STATUS_UNEXPECTED_RESPONSE;
    }
    // If wrong password or not logged in, clear global state
    if(status == STATUS_WRONG_PASSWORD || status == STATUS_NOT_LOGGED_IN){
        is_logged_in = 0;
        memset(current_password, 0, sizeof(current_password));
        memset(current_uid, 0, sizeof(current_uid));
    }
    if (status == STATUS_OK){
        if(get_next_arg(&cursor_resp, eid) == ERROR ||
           !is_end_of_message(&cursor_resp))
            return STATUS_MALFORMED_RESPONSE;
        event_message(eid);
        return STATUS_CUSTOM_OUTPUT;
    }

    return status;
}

ReplyStatus close_event_handler(char** cursor) {
    char raw_eid[4];
    ReplyStatus status = parse_eid(cursor, raw_eid);
    if (status != STATUS_UNASSIGNED) return status;
    if (!is_logged_in) return STATUS_NOT_LOGGED_IN_LOCAL;

    char eid[4];
    if (convert_to_3_digit(raw_eid, eid) == ERROR) return STATUS_INVALID_EID;

    // PROTOCOL: CLO <uid> <password> <eid>
    char request[256], response[256];
    snprintf(request, sizeof(request), "CLS %s %s %s\n", current_uid, current_password, eid);

    status = tcp_send_receive(request, response, 256);
    if(status != STATUS_UNASSIGNED) return status;
    
    char *response_cursor = response;
    status = parse_response_header(&response_cursor, CLOSE);
    if(!is_end_of_message(&response_cursor)) return STATUS_MALFORMED_RESPONSE;
    
    // Expected responses: OK / NOK / NLG / NOE / EOW / SLD / PST / CLO / WRP
    if(status != STATUS_OK &&
       status != STATUS_NOK &&
       status != STATUS_NOT_LOGGED_IN &&
       status != STATUS_NO_EVENT_ID &&
       status != STATUS_EVENT_WRONG_USER &&
       status != STATUS_EVENT_SOLD_OUT &&
       status != STATUS_PAST_EVENT &&
       status != STATUS_EVENT_CLOSE_CLOSED &&
       status != STATUS_WRONG_PASSWORD &&
       status != STATUS_ERROR &&
       status != STATUS_MALFORMED_RESPONSE) 
        return STATUS_UNEXPECTED_RESPONSE;
    

    // If wrong password or not logged in, clear global state
    if(status == STATUS_WRONG_PASSWORD || status == STATUS_NOT_LOGGED_IN){
        is_logged_in = 0;
        memset(current_password, 0, sizeof(current_password));
        memset(current_uid, 0, sizeof(current_uid));
    }

    return status;
}

ReplyStatus list_handler(char** cursor) {
    if(!is_end_of_message(cursor)) return STATUS_INVALID_ARGS;

    // PROTOCOL: LST <uid> <password>
    char request[256];
    snprintf(request, sizeof(request), "LST\n");

    // Send request to server and receive response
    int tcp_fd = connect_tcp(IP, PORT);
    if (tcp_fd == -1) return STATUS_SEND_FAILED;
    
    // Send request header to server
    if (tcp_send_message(tcp_fd, request) == ERROR) {
        close(tcp_fd);
        return STATUS_SEND_FAILED;
    }

    // Read server response command and status
    ReplyStatus status = read_cmd_status(tcp_fd, LIST);
    
    // Expected responses: OK / NOK
    if(status != STATUS_OK &&
       status != STATUS_NOK &&
       status != STATUS_ERROR &&
       status != STATUS_MALFORMED_RESPONSE) {
        close(tcp_fd);
        return STATUS_UNEXPECTED_RESPONSE;
    }
    if (status != STATUS_OK){
        close(tcp_fd);
        return status;  
    }

    show_events_list(tcp_fd);
    close(tcp_fd);
    return STATUS_CUSTOM_OUTPUT;
}

ReplyStatus show_handler(char** cursor){
    char raw_eid[4];
    ReplyStatus status = parse_eid(cursor, raw_eid);
    if (status != STATUS_UNASSIGNED) return status;

    char eid[4];
    if (convert_to_3_digit(raw_eid, eid) == ERROR)
        return STATUS_INVALID_EID;

    // PROTOCOL: SED <eid>
    char request[256];
    snprintf(request, sizeof(request), "SED %s\n", eid);
    
    // Send request to server and receive response
    int tcp_fd = connect_tcp(IP, PORT);
    if (tcp_fd == -1) return STATUS_SEND_FAILED;
    
    // Send request header to server
    if (tcp_send_message(tcp_fd, request) == ERROR) {
        close(tcp_fd);
        return STATUS_SEND_FAILED;
    }

    // PROTOCOl: RSE status [UID name event_date attendance_size Seats_reserved Fname Fsize Fdata]
    char uid[UID_LENGTH + 1];
    char event_name[MAX_EVENT_NAME + 1];
    char event_date[EVENT_DATE_LENGTH + 1];
    char attendance_size[SEAT_COUNT_LENGTH + 1];
    char reserved_seats[SEAT_COUNT_LENGTH + 1];
    char file_name[FILE_NAME_LENGTH + 1];
    char file_size[FILE_SIZE_LENGTH + 1];
    status = read_show_response_header(tcp_fd,
                                       uid, event_name,
                                       event_date, attendance_size,
                                       reserved_seats, file_name,
                                       file_size);
    // Expected responses: OK / NOK
    if(status != STATUS_OK &&
       status != STATUS_NOK &&
       status != STATUS_ERROR &&
       status != STATUS_MALFORMED_RESPONSE) {
        close(tcp_fd);
        return STATUS_UNEXPECTED_RESPONSE;
    }
    if (status != STATUS_OK){
        close(tcp_fd);       
        return status;
    }

    // Expected responses: OK / NOK
    long file_size_long = atol(file_size);
    if(tcp_read_file(tcp_fd, file_name, file_size_long) == ERROR) {
        close(tcp_fd);
        return STATUS_RECV_FAILED;
    }
    close(tcp_fd);
    // Display event details
    show_event_details(eid, uid, event_name, event_date,
                      attendance_size, reserved_seats,
                      file_name, file_size);
    return STATUS_CUSTOM_OUTPUT;
} 

ReplyStatus reserve_handler(char** cursor) {
    char eid[4], num_seats[4];
    ReplyStatus status = parse_reserve(cursor, eid, num_seats);
    if (status != STATUS_UNASSIGNED) return status;
    if (!is_logged_in) return STATUS_NOT_LOGGED_IN_LOCAL;

    char padded_eid[4];
    if (convert_to_3_digit(eid, padded_eid) == ERROR)
        return STATUS_INVALID_EID;

    // PROTOCOL: RID <uid> <password> <EID> <people>.
    char request[256];
    snprintf(request, sizeof(request), "RID %s %s %s %s\n",
             current_uid, current_password, padded_eid, num_seats);

    int tcp_fd = connect_tcp(IP, PORT);
    if (tcp_fd == -1) return STATUS_SEND_FAILED;
    
    // Send request header to server
    if (tcp_send_message(tcp_fd, request) == ERROR) {
        close(tcp_fd);
        return STATUS_SEND_FAILED;
    }
    status = read_cmd_status(tcp_fd, RESERVE);
    
    // Expected responses: ACC / REJ / CLS / SLD / PST / NOK / NLG / WRP
    if(status != STATUS_EVENT_RESERVATION_REJECTION &&
       status != STATUS_EVENT_RESERVED &&
       status != STATUS_EVENT_CLOSED &&
       status != STATUS_EVENT_SOLD_OUT &&
       status != STATUS_PAST_EVENT &&
       status != STATUS_NOK &&
       status != STATUS_NOT_LOGGED_IN &&
       status != STATUS_WRONG_PASSWORD &&
       status != STATUS_ERROR &&
       status != STATUS_MALFORMED_RESPONSE) {
        close(tcp_fd);
        return STATUS_UNEXPECTED_RESPONSE;
    }

    // If wrong password or not logged in, clear global state
    if(status == STATUS_WRONG_PASSWORD || status == STATUS_NOT_LOGGED_IN){
        is_logged_in = 0;
        memset(current_password, 0, sizeof(current_password));
        memset(current_uid, 0, sizeof(current_uid));
    }

    if(status != STATUS_EVENT_RESERVATION_REJECTION){
        close(tcp_fd);
        return status;  
    }

    char seats_left[4];
    if(tcp_read_field(tcp_fd, seats_left, sizeof(seats_left)) == ERROR ||
       !verify_reserved_seats(seats_left, "999")) {
        close(tcp_fd);
        return STATUS_RECV_FAILED;
    }
    show_event_reservations(seats_left, eid);
    close(tcp_fd);
    return STATUS_CUSTOM_OUTPUT;
}
