#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>   // stat, S_ISREG
#include <unistd.h>    // access, R_OK

#include "command_handlers.h"
#include "../../common/verifications.h"
#include "../../common/common.h"

#include "client_data.h"

int verify_file(char* file_name) {
    struct stat st;
    return (stat(file_name, &st) == 0 && //Accessible file
            S_ISREG(st.st_mode) && // Regular file
            access(file_name, R_OK) == 0 && // Readable file 
            st.st_size <= MAX_FILE_SIZE) ? // Size limit
            VALID : INVALID;
}


ReplyStatus login_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
            socklen_t udp_addr_len) {
    char uid[32], password[32];
    ssize_t n;

    if (is_logged_in) return STATUS_ALREADY_LOGGED_IN;

    if (!verify_argument_count(args, 2)) return STATUS_INVALID_ARGS;
    sscanf(args, "%31s %31s", uid, password);

    if (!verify_uid_format(uid)) return STATUS_INVALID_UID;
    if (!verify_password_format(password)) return STATUS_INVALID_PASSWORD;

    char request[256];

    // PROTOCOL: LIN <uid> <password>
    snprintf(request, sizeof(request), "LIN %s %s\n", uid, password);

    if (sendto(udp_fd, request, strlen(request), 0, (struct sockaddr*)server_udp_addr,
                udp_addr_len) == ERROR) return STATUS_SEND_FAILED;


    char response[256];
    n = recvfrom(udp_fd, response, sizeof(response) - 1, 0, NULL, NULL);
    if (n == ERROR) return STATUS_RECV_FAILED;

    response[n] = '\0';
    char response_code[4];
    char reply_status[4];

    int parsed = sscanf(response, "%3s %3s", response_code, reply_status);
    ReplyStatus status = handle_response_code(response_code, "RLI", parsed, 2, reply_status);

    // Update global state on successful login
    if (status == STATUS_OK || status == STATUS_REGISTERED) {
        is_logged_in = 1;
        strcpy(current_password, password);
        strcpy(current_uid, uid);
    }

    return status;
}

ReplyStatus unregister_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
                                socklen_t udp_addr_len) {
    ssize_t n;
    if (!verify_argument_count(args, 0)) return STATUS_INVALID_ARGS;
    if (!is_logged_in) return STATUS_NOT_LOGGED_IN_LOCAL;

    char request[256];

    // PROTOCOL: UNR <uid> <password>
    snprintf(request, sizeof(request), "UNR %s %s\n", current_uid, current_password);

    if (sendto(udp_fd, request, strlen(request), 0, (struct sockaddr*)server_udp_addr,\
        udp_addr_len) == ERROR) return STATUS_SEND_FAILED;

    char response[256];
    n = recvfrom(udp_fd, response, sizeof(response) - 1, 0, NULL, NULL);
    if (n == ERROR) return STATUS_RECV_FAILED;

    response[n] = '\0';

    char response_code[4];
    char reply_status[4];

    int parsed = sscanf(response, "%3s %3s", response_code, reply_status);

    if (parsed < 2) return STATUS_MALFORMED_RESPONSE;

    if (strcmp(response_code, "RUR") != 0) return STATUS_UNEXPECTED_RESPONSE;

    ReplyStatus status = parse_status_code(reply_status);

    // Clear global state on successful unregister
    if (status == STATUS_OK) {
        is_logged_in = 0;
        memset(current_password, 0, sizeof(current_password));
        memset(current_uid, 0, sizeof(current_uid));
    }

    return status;
}

ReplyStatus logout_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len) {

    ssize_t n;

    if (!verify_argument_count(args, 0)) return STATUS_INVALID_ARGS;

    if (!is_logged_in) return STATUS_NOT_LOGGED_IN_LOCAL;

    char request[256];

    // PROTOCOL: LOU <uid> <password>
    snprintf(request, sizeof(request), "LOU %s %s\n", current_uid, current_password);

    if (sendto(udp_fd, request, strlen(request), 0, (struct sockaddr*)server_udp_addr,
                udp_addr_len) == ERROR) return STATUS_SEND_FAILED;

    char response[256];
    n = recvfrom(udp_fd, response, sizeof(response) - 1, 0, NULL, NULL);
    if (n == ERROR) return STATUS_RECV_FAILED;

    response[n] = '\0';

    char response_code[4];
    char reply_status[4];

    int parsed = sscanf(response, "%3s %3s", response_code, reply_status);

    if (parsed < 2) return STATUS_MALFORMED_RESPONSE;

    if (strcmp(response_code, "RLO") != 0) return STATUS_UNEXPECTED_RESPONSE;

    ReplyStatus status = parse_status_code(reply_status);

    // Clear global state on successful logout
    if (status == STATUS_OK) {
        is_logged_in = 0;
        memset(current_password, 0, sizeof(current_password));
        memset(current_uid, 0, sizeof(current_uid));
    }

    return status;
}

ReplyStatus myevent_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
                            socklen_t udp_addr_len) {
    ssize_t n;
    if (!verify_argument_count(args, 0)) return STATUS_INVALID_ARGS;
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

ReplyStatus create_event_handler(char* args, char** extra_info) {
    char event_name[MAX_EVENT_NAME + 1];
    char file_name[24 + 1]; // TODO: maximum file name length?
    char date[10 + 1];
    char time[6 + 1];
    char real_date[30 + 1];
    char num_seats[4];
    *extra_info = NULL;

    if (!verify_argument_count(args, 5)) return STATUS_INVALID_ARGS;

    // name event_fname event_date, num_attendees
    // TODO: corrigir tamanho do scanf
    sscanf(args,"%10s %24s %10s %6s %s", event_name, file_name, date, time, num_seats);
    snprintf(real_date, sizeof(real_date), "%s %s", date, time);
    printf("Creating event: %s on %s with %s seats, file: %s\n", 
            event_name, real_date, num_seats, file_name);
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
    printf("File size: %ld bytes\n", file_size);

    // Prepare request header
    char request_header[512];
    snprintf(request_header, sizeof(request_header), "CRE %s %s %s %s %s %s %ld ",
             current_uid, current_password, event_name, real_date, num_seats, file_name, file_size);
    
    printf("Sending create event request: %s\n", request_header);
    // Send request header
    if (send_tcp_message(tcp_fd, request_header) == ERROR) {
        close(tcp_fd);
        return STATUS_SEND_FAILED;
    }


    // Send file
    if (send_tcp_file(tcp_fd, file_name) == ERROR) {
        close(tcp_fd);
        return STATUS_SEND_FAILED;
    }   

    read_tcp(tcp_fd, request_header, sizeof(request_header));
    close(tcp_fd);
    char response_code[4];
    char status[4];
    char eid[4];
    int parsed = sscanf(request_header, "%3s %3s %3s", response_code, status, eid);
    if (parsed < 1) return STATUS_MALFORMED_RESPONSE;
    printf("Create event response: %s\n", request_header);
    if (!strcmp(response_code, "ERR")) return STATUS_UNEXPECTED_RESPONSE;
    if (strcmp(response_code, "RCE")) return STATUS_UNEXPECTED_RESPONSE;
    if(!strcmp(status, "NGL")) return STATUS_NOT_LOGGED_IN;
    if (!strcmp(status, "OK")){
        *extra_info = malloc(4 * sizeof(char));
        sscanf(request_header, "%*s %*s %3s", *extra_info);
        return STATUS_OK;
    }
    if (!strcmp(response_code, "NOK")) return STATUS_NOK;
    
    return STATUS_UNASSIGNED; // TODO: handle response
}

