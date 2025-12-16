#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "verifications.h"
#include "data.h"

int get_next_arg(char **cursor, char *out);
int is_end_of_message(char **cursor);
int parse_cmd(char **cursor, char* cmd);


#endif