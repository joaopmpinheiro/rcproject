#include "../../common/common.h"
#include "../../common/data.h"
#include "parser.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

ReplyStatus parse_eid(char **cursor, char* eid) {
    if(get_next_arg(cursor, eid) == ERROR ||
       !is_end_of_message(cursor))
        return STATUS_INVALID_ARGS;

    if(!verify_eid_format(eid)) return STATUS_INVALID_EID;
    return STATUS_UNASSIGNED;    
}

ReplyStatus parse_login(char **cursor, char* uid, char* password) {
    if(get_next_arg(cursor, uid) == ERROR ||
       get_next_arg(cursor, password) == ERROR ||
       !is_end_of_message(cursor))
        return STATUS_INVALID_ARGS;
    if(!verify_uid_format(uid)) return STATUS_INVALID_UID;
    if(!verify_password_format(password)) return STATUS_INVALID_PASSWORD;
    return STATUS_UNASSIGNED;    
}

ReplyStatus parse_create_event(char **cursor, char* event_name, char* file_name,
                            char* date, char* num_seats) {
    char day_str[DAY_STR_SIZE + 1] = {0};
	char time_str[TIME_STR_SIZE + 1] = {0};
	
    if(get_next_arg(cursor, event_name) == ERROR ||
    get_next_arg(cursor, file_name) == ERROR ||
    get_next_arg(cursor, day_str) == ERROR||
    get_next_arg(cursor, time_str) == ERROR ||
    get_next_arg(cursor, num_seats) == ERROR ||
    !is_end_of_message(cursor))
        return STATUS_INVALID_ARGS;

    snprintf(date, EVENT_DATE_LENGTH + 1, "%s %s", day_str, time_str);
    if(!verify_event_name_format(event_name)) return STATUS_INVALID_EVENT_NAME;
    if(!verify_file_name_format(file_name)) return STATUS_INVALID_FILE;
    if(!verify_event_date_format(date)) return STATUS_INVALID_EVENT_DATE;
    if(!verify_seat_count(num_seats)) return STATUS_INVALID_SEAT_COUNT;
    return STATUS_UNASSIGNED;
}


ReplyStatus parse_change_password(char **cursor, char* old_password,
                                  char* new_password, char* current_password) {
    if(get_next_arg(cursor, old_password) == ERROR ||
       get_next_arg(cursor, new_password) == ERROR ||
       !is_end_of_message(cursor))
        return STATUS_INVALID_ARGS;

    if (!verify_password_format(old_password) ||
        !verify_password_format(new_password))
        return STATUS_INVALID_PASSWORD;

    if(strcmp(old_password, current_password) != 0) return STATUS_WRONG_PASSWORD;
    return STATUS_UNASSIGNED;
}

ReplyStatus parse_reserve(char **cursor, char* eid, char* num_seats) {
    if(get_next_arg(cursor, eid) == ERROR ||
       get_next_arg(cursor, num_seats) == ERROR ||
       !is_end_of_message(cursor))
        return STATUS_INVALID_ARGS;
    if(!verify_eid_format(eid)) return STATUS_INVALID_EID;
    if(!verify_reserved_seats(num_seats, "999")) return STATUS_INVALID_SEAT_COUNT;
    return STATUS_UNASSIGNED;    
}

ReplyStatus parse_reservations(char **cursor, char* eid, char* event_date,
                              char* seats_reserved) {
    char day_str[DAY_STR_SIZE + 1], time_str[TIME_STR_SIZE + 1 + 3];
    
    if(get_next_arg(cursor, eid) == ERROR ||
       get_next_arg(cursor, day_str) == ERROR ||
       get_next_arg(cursor, time_str) == ERROR ||
       get_next_arg(cursor, seats_reserved) == ERROR)
        return STATUS_MALFORMED_RESPONSE;

    snprintf(event_date, EVENT_DATE_LENGTH + 1 + 3, "%s %s", day_str, time_str);

    if(!verify_eid_format(eid) ||
       !verify_reserved_seats(seats_reserved, "999"))
          return STATUS_MALFORMED_RESPONSE;

    return STATUS_UNASSIGNED;    
}