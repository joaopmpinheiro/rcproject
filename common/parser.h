#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "verifications.h"
#include "data.h"

int get_next_arg(char **cursor, char *out);
int is_end_of_message(char **cursor);
int parse_cmd(char **cursor, char* cmd);
ReplyStatus parse_uid(char **cursor, char* uid);
ReplyStatus parse_password(char **cursor, char* password);
ReplyStatus parse_eid(char **cursor, char* eid);
ReplyStatus parse_datetime(char **cursor, char *date);
ReplyStatus parse_event_name(char **cursor, char *event_name);
ReplyStatus parse_total_seat(char **cursor, char *num_seats);
ReplyStatus parse_file_name(char **cursor, char *file_name);


#endif