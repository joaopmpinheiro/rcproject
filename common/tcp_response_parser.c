#include "tcp_response_parser.h"


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




