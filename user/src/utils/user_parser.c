#include "common.h"
#include "verifications.h"
#include "parser.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

ReplyStatus parse_create_event(char **cursor, char* event_name, char* file_name,
                            char* date, char* num_seats) {
    ReplyStatus status;
    status = parse_event_name(cursor, event_name);
    if (status != STATUS_UNASSIGNED) return status;
    status = parse_file_name(cursor, file_name);
    if (status != STATUS_UNASSIGNED) return status;
    status = parse_datetime(cursor, date);
    if (status != STATUS_UNASSIGNED) return status;
    status = parse_total_seat(cursor, num_seats);
    if (status != STATUS_UNASSIGNED) return status;
    if (is_end_of_message(cursor) == ERROR) return STATUS_INVALID_ARGS;
    return STATUS_UNASSIGNED;
}


ReplyStatus parse_change_password(char **cursor, char* old_password,
                                  char* new_password, char* current_password) {
    ReplyStatus status;
    status = parse_password(cursor, old_password);
    if (status != STATUS_UNASSIGNED) return status;
    status = parse_password(cursor, new_password);
    if (status != STATUS_UNASSIGNED) return status;
    if (is_end_of_message(cursor) == ERROR) return STATUS_INVALID_ARGS;
    if(strcmp(old_password, current_password) != 0) return STATUS_WRONG_PASSWORD;
    return STATUS_UNASSIGNED;

}

ReplyStatus parse_close(char **cursor, char* eid) {
    ReplyStatus status;
    status = parse_eid(cursor, eid);
    if (status != STATUS_UNASSIGNED) return status;
    if (is_end_of_message(cursor) == ERROR) return STATUS_INVALID_ARGS;
    return STATUS_UNASSIGNED;    
}