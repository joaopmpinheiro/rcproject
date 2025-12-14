#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>   // stat, S_ISREG
#include <unistd.h>    // access, R_OK

#include "utils.h"
#include "../../common/parser.h"

#include "client_data.h"

// TODO: arranjar um sitio para esta funcao
int verify_file(char* file_name) {
    struct stat st;
    if (stat(file_name, &st) != 0) {
        return STATUS_FILE_NOT_FOUND;
    }
    if (access(file_name, R_OK) != 0) {
        return STATUS_FILE_READ_ERROR;
    }
    if (st.st_size > MAX_FILE_SIZE) {
        return STATUS_FILE_SIZE_EXCEEDED;
    }
    return VALID;
}


ReplyStatus login_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
            socklen_t udp_addr_len) {

    if (is_logged_in) return STATUS_ALREADY_LOGGED_IN;
                
    // Verify arguments
    ReplyStatus status;
    char uid[32], password[32];

    status = parse_uid(cursor, uid);
    if (status != STATUS_UNASSIGNED) return status;

    status = parse_password(cursor, password);
    if (status != STATUS_UNASSIGNED) return status;
    if(is_end_of_message(cursor) == ERROR) return STATUS_INVALID_ARGS;

    
    // PROTOCOL: LIN <uid> <password>
    char request[256], response[256];
    ssize_t response_size = 256;
    snprintf(request, sizeof(request), "LIN %s %s\n", uid, password);

    // Send request to server and receive response
    status = udp_send_receive(udp_fd, server_udp_addr, udp_addr_len,
                                request, response, response_size);
    if (status != STATUS_UNASSIGNED) return status;
    
    // Parse response
    char response_code[4], reply_status[4];
    int parsed = sscanf(response, "%3s %3s", response_code, reply_status);
    status = handle_response_code(response_code, LOGIN, parsed, 2, reply_status);

    // Update global state on successful login
    if ((status == STATUS_OK && !is_logged_in) || status == STATUS_REGISTERED) {
        is_logged_in = 1;
        strcpy(current_password, password);
        strcpy(current_uid, uid);
    }
    else if (status == STATUS_OK)  is_logged_in = 1;

    return status;
}

ReplyStatus unregister_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
                                socklen_t udp_addr_len) {
    // Verify arguments
    if(is_end_of_message(cursor) == ERROR) return STATUS_INVALID_ARGS;

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
    char response_code[4], reply_status[4];
    int parsed = sscanf(response, "%3s %3s", response_code, reply_status);
    status = handle_response_code(response_code, UNREGISTER, parsed, 2, reply_status);

    // Clear global state on successful unregister
    if (status == STATUS_OK) {
        is_logged_in = 0;
        memset(current_password, 0, sizeof(current_password));
        memset(current_uid, 0, sizeof(current_uid));
    }

    return status;
}


ReplyStatus logout_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len) {

    // Verify arguments
    if(is_end_of_message(cursor) == ERROR) return STATUS_INVALID_ARGS;
    if (!is_logged_in) return STATUS_NOT_LOGGED_IN_LOCAL;

    char request[256], response[256];
    ssize_t response_size = 256;

    // PROTOCOL: LOU <uid> <password>
    snprintf(request, sizeof(request), "LOU %s %s\n", current_uid, current_password);
    fprintf(stderr, "Logout request: %s", request);

    // Send request to server and receive response
    ReplyStatus status = udp_send_receive(udp_fd, server_udp_addr, udp_addr_len,
                                request, response, response_size);
    if (status != STATUS_UNASSIGNED) return status;

    // Parse response
    char response_code[4], reply_status[4];
    int parsed = sscanf(response, "%3s %3s", response_code, reply_status);
    status = handle_response_code(response_code, LOGOUT, parsed, 2, reply_status);

    // Clear global state on successful logout
    if (status == STATUS_OK) {
        is_logged_in = 0;
        memset(current_password, 0, sizeof(current_password));
        memset(current_uid, 0, sizeof(current_uid));
    }

    return status;
}


ReplyStatus myevent_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
                            socklen_t udp_addr_len) {
    ssize_t n;
    if(is_end_of_message(cursor) == ERROR) return STATUS_INVALID_ARGS;
    if (!is_logged_in) return STATUS_NOT_LOGGED_IN_LOCAL;

    char request[256];

    // PROTOCOL: LME <uid> <password>
    snprintf(request, sizeof(request), "LME %s %s\n", current_uid, current_password);

    if (sendto(udp_fd, request, strlen(request), 0, (struct sockaddr*)server_udp_addr,
                udp_addr_len) == ERROR) return STATUS_SEND_FAILED;

    char response[8192];
    n = recvfrom(udp_fd, response, sizeof(response) - 1, 0, NULL, NULL);
    if (n == ERROR) return STATUS_RECV_FAILED;

    response[n] = '\0';

    char response_code[4];
    char reply_status[4];

    int parsed = sscanf(response, "%3s %3s", response_code, reply_status);

    if (parsed < 2) return STATUS_MALFORMED_RESPONSE;

    if (strcmp(response_code, "RME") != 0) return STATUS_UNEXPECTED_RESPONSE;

    ReplyStatus status = parse_status_code(reply_status);

    // PROTOCOL: RME <status>[ <event1ID state> <event2ID state> ...]
    if (status == STATUS_OK) {
        char* event_list = response + 7;

        printf("Your events:\n");
        printf("%-5s %-10s\n", "EID", "State");
        printf("-------------------\n");

        if (strlen(event_list) == 0) {
            printf("(no events)\n");
            return STATUS_CUSTOM_OUTPUT;
        }

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

    return status;
}

/*ReplyStatus create_event_handler(char** cursor, char** extra_info) {
    char event_name[MAX_EVENT_NAME + 1];
    char file_name[FILE_NAME_LENGTH + 1]; // TODO: maximum file name length?
    char real_date[EVENT_DATE_LENGTH + 1]; //TODO: change this 
    char date[10 + 1];
    char time[5 + 1];
    char num_seats[4];
    *extra_info = NULL;

    char* args;
    if (!verify_argument_count(*cursor, 5)) return STATUS_INVALID_ARGS;

    // Verify arguments
    sscanf(args, "%10s %24s %10s %5s %3s", event_name, file_name, date, time, num_seats);
    
    // TODO: delete mas para isso é preciso trocar a funcao de verificar data
    snprintf(real_date, sizeof(real_date), "%s %s", date, time);
   
    if (!verify_event_name_format(event_name)) return STATUS_INVALID_EVENT_NAME;
    if (!verify_file(file_name)) return STATUS_INVALID_FILE;
    if (!verify_event_date_format(real_date)) return STATUS_INVALID_EVENT_DATE;
    if (!verify_seat_count(num_seats)) return STATUS_INVALID_SEAT_COUNT;
    
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
             current_uid, current_password, event_name, real_date, num_seats, file_name, file_size);
    
    // Send request header to server
    if (send_tcp_message(tcp_fd, request_header) == ERROR) {
        close(tcp_fd);
        return STATUS_SEND_FAILED;
    }

    // Send file to server
    if (send_tcp_file(tcp_fd, file_name) == ERROR) {
        close(tcp_fd);
        return STATUS_SEND_FAILED;
    }   

    // Read server response
    read_tcp(tcp_fd, request_header, sizeof(request_header));
    close(tcp_fd);
    char response_code[4], reply_status[4], eid[4];
    fprintf(stderr, "Create event response: %s\n", request_header);
    int parsed = sscanf(request_header, "%3s %3s %3s", response_code, reply_status, eid);
    ReplyStatus status = handle_response_code(response_code, CREATE, parsed, 3, reply_status);

    if (status == STATUS_OK){
        *extra_info = malloc(4 * sizeof(char));
        sscanf(request_header, "%*s %*s %3s", *extra_info);
    }    
    return status; // TODO: handle response
}

  */