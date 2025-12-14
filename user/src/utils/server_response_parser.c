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
#include "../include/client_data.h"
#include "../../include/utils.h"
#include "../../common/common.h"
#include "../../common/data.h"
#include "../../common/verifications.h"

ReplyStatus read_command(int tcp_fd, char* command, RequestType expected_command) {
    if(read_tcp_argument(tcp_fd, command, COMMAND_LENGTH) != SUCCESS)\
        return STATUS_RECV_FAILED;
    RequestType req = identify_command_response(command);
    if (req == UNKNOWN) return STATUS_MALFORMED_RESPONSE;
    if (req == ERROR_REQUEST) return STATUS_ERROR;
    if (req != expected_command) return STATUS_UNEXPECTED_RESPONSE;
    return STATUS_OK;
}

ReplyStatus read_status(int tcp_fd, char* status) {
    if(read_tcp_argument(tcp_fd, status, 3) != SUCCESS) return STATUS_RECV_FAILED;
    return parse_status_code(status);
}

ReplyStatus read_uid(int tcp_fd, char* uid) {
    if(read_tcp_argument(tcp_fd, uid, UID_LENGTH) != SUCCESS) return STATUS_RECV_FAILED;
    if (verify_uid_format(uid) == INVALID) return STATUS_INVALID_UID;
    return STATUS_OK;
}

ReplyStatus read_eid(int tcp_fd, char* eid) {
    if(read_tcp_argument(tcp_fd, eid, EID_LENGTH) != SUCCESS) return STATUS_RECV_FAILED;
    if (verify_eid_format(eid) == INVALID) return STATUS_INVALID_EID;
    return STATUS_OK;
}

ReplyStatus read_event_name(int tcp_fd, char* event_name) {
    if(read_tcp_argument(tcp_fd, event_name, MAX_EVENT_NAME) != SUCCESS) return STATUS_RECV_FAILED;
    if (!verify_event_name_format(event_name)) return STATUS_INVALID_EVENT_NAME;
    return STATUS_OK;
}

ReplyStatus read_event_date(int tcp_fd, char* event_date) {
    char day_str[DAY_STR_SIZE + 1];
    char time_str[TIME_STR_SIZE + 1];

    if(read_tcp_argument(tcp_fd, day_str, DAY_STR_SIZE) != SUCCESS) return STATUS_RECV_FAILED;
    if(read_tcp_argument(tcp_fd, time_str, TIME_STR_SIZE) != SUCCESS) return STATUS_RECV_FAILED;

    snprintf(event_date, EVENT_DATE_LENGTH + 1, "%s %s", day_str, time_str);
    if (!verify_event_date_format(event_date)) return STATUS_INVALID_EVENT_DATE;
    return STATUS_OK;
}

ReplyStatus read_seat_count(int tcp_fd, char* seat_count) {
    if(read_tcp_argument(tcp_fd, seat_count, SEAT_COUNT_LENGTH) != SUCCESS) return STATUS_RECV_FAILED;
    if (!verify_seat_count(seat_count)) return STATUS_INVALID_SEAT_COUNT;
    return STATUS_OK;
}

ReplyStatus read_file_name(int tcp_fd, char* file_name) {
    if(read_tcp_argument(tcp_fd, file_name, FILE_NAME_LENGTH) != SUCCESS) return STATUS_RECV_FAILED;
    if (!verify_file_name_format(file_name)) return STATUS_INVALID_FILE;
    return STATUS_OK;
}

ReplyStatus read_file_size(int tcp_fd, char* file_size) {
    if(read_tcp_argument(tcp_fd, file_size, FILE_SIZE_LENGTH) != SUCCESS) return STATUS_RECV_FAILED;
    if (!verify_file_size(file_size)) return STATUS_INVALID_FILE;
    return STATUS_OK;
}

ReplyStatus read_show_response_header(char* response, int tcp_fd,
                                       char* resp_code, char* rep_status, 
                                       char* uid, char* event_name, char* event_date,
                                       char* total_seats, char* reserved_seats,
                                       char* file_name, char* file_size) {
    // RESPONSE CODE
    if(read_tcp_argument(tcp_fd, resp_code, 4) != SUCCESS) {
        close(tcp_fd);
        return STATUS_RECV_FAILED;
    }
    ReplyStatus status;
    status = read_command(tcp_fd, resp_code, SHOW);
    if(status != STATUS_OK) {
        close(tcp_fd);
        return status;
    }
    status = read_status(tcp_fd, rep_status);
    if (status != STATUS_OK) {
        close(tcp_fd);
        return status;
    }

    // EID EVENT_NAME EVENT_DATE UID TOTAL_SEATS RESERVED_SEATS FILE_NAME FILE_SIZE
    if( read_uid(tcp_fd, uid) != STATUS_OK ||
        read_event_name(tcp_fd, event_name) != STATUS_OK ||
        read_event_date(tcp_fd, event_date) != STATUS_OK ||
        read_seat_count(tcp_fd, total_seats) != STATUS_OK ||
        read_seat_count(tcp_fd, reserved_seats) != STATUS_OK ||
        read_file_name(tcp_fd, file_name) != STATUS_OK ||
        read_file_size(tcp_fd, file_size) != STATUS_OK) {
        close(tcp_fd);
        return STATUS_RECV_FAILED;
    }   
    return STATUS_OK;
}


