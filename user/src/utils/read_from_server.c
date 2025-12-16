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
    if(tcp_read_field(tcp_fd, command, COMMAND_LENGTH) == ERROR)\
        return STATUS_RECV_FAILED;
    RequestType req = identify_command_response(command);
    if (req == UNKNOWN) return STATUS_MALFORMED_RESPONSE;
    if (req == ERROR_REQUEST) return STATUS_ERROR;
    if (req != expected_command) return STATUS_UNEXPECTED_RESPONSE;
    return STATUS_OK;
}

ReplyStatus read_cmd_status(int tcp_fd, RequestType expected_command) {
    char command[COMMAND_LENGTH + 1];
    char rep_status[4];
    // Response command
    if(tcp_read_field(tcp_fd, command, COMMAND_LENGTH + 1) == ERROR)
        return STATUS_RECV_FAILED;

    // Confirm command
    RequestType req = identify_command_response(command);
    if (req == ERROR_REQUEST) return CMD_ERROR;
    if (req != expected_command) return STATUS_UNEXPECTED_RESPONSE;

    // Response status
    if(tcp_read_field(tcp_fd, rep_status, 3) == ERROR) return STATUS_RECV_FAILED;
    return parse_status_code(rep_status);
}

ReplyStatus read_show_response_header(int tcp_fd,
                                       char* uid, char* event_name,
                                       char* event_date, char* attendance_size,
                                       char* reserved_seats, char* file_name,
                                       char* file_size) {
    ReplyStatus status = read_cmd_status(tcp_fd, SHOW);
    if (status != STATUS_OK) return status;
    
    char str_day[DAY_STR_SIZE + 1], str_time[TIME_STR_SIZE + 1];
    // Read remaining fields
    if(tcp_read_field(tcp_fd, uid, UID_LENGTH) != SUCCESS ||
       tcp_read_field(tcp_fd, event_name, MAX_EVENT_NAME) != SUCCESS ||
       tcp_read_field(tcp_fd, str_day, DAY_STR_SIZE) != SUCCESS ||
       tcp_read_field(tcp_fd, str_time, TIME_STR_SIZE) != SUCCESS ||
       tcp_read_field(tcp_fd, attendance_size, SEAT_COUNT_LENGTH) != SUCCESS ||
       tcp_read_field(tcp_fd, reserved_seats, SEAT_COUNT_LENGTH) != SUCCESS ||
       tcp_read_field(tcp_fd, file_name, FILE_NAME_LENGTH) != SUCCESS ||
       tcp_read_field(tcp_fd, file_size, FILE_SIZE_LENGTH) != SUCCESS){
        return STATUS_MALFORMED_RESPONSE;
    }
    snprintf(event_date, EVENT_DATE_LENGTH + 1, "%s %s", str_day, str_time);
    if(!verify_uid_format(uid) ||
       !verify_event_name_format(event_name) ||
       !verify_event_date_format(event_date) ||
       !verify_seat_count(attendance_size) ||
       !verify_reserved_seats(reserved_seats, attendance_size) ||
       !verify_file_name_format(file_name) ||
       !verify_file_size(file_size)) return STATUS_MALFORMED_RESPONSE;
    return STATUS_OK;
}    

//TODO: reconhecer diferencas entre EOM e ERROR
ReplyStatus read_events_list(int fd_tcp, char* eid, char* name, char* state,
                              char* event_day, char* event_time) {
                                
    if(tcp_read_field(fd_tcp, eid, EID_LENGTH) != SUCCESS)
        return STATUS_MALFORMED_RESPONSE;
    if(tcp_read_field(fd_tcp, name, MAX_EVENT_NAME) != SUCCESS)
        return STATUS_MALFORMED_RESPONSE;
    if(tcp_read_field(fd_tcp, state, 2) != SUCCESS)
        return STATUS_MALFORMED_RESPONSE;
    if(tcp_read_field(fd_tcp, event_day, DAY_STR_SIZE) != SUCCESS)
        return STATUS_MALFORMED_RESPONSE;   
    if(tcp_read_field(fd_tcp, event_time, TIME_STR_SIZE) != SUCCESS)
        return STATUS_MALFORMED_RESPONSE;
    return STATUS_UNASSIGNED;
}

ReplyStatus read_reserve_seats(int tcp_fd, char* seats) {
    if (tcp_read_field(tcp_fd, NULL, 0) != SUCCESS)
        return STATUS_MALFORMED_RESPONSE;
    if(!verify_reserved_seats(seats, "999"))
        return STATUS_MALFORMED_RESPONSE;
    return STATUS_UNASSIGNED;
}


