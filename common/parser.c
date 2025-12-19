#include "common.h"
#include "verifications.h"
#include "parser.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int get_next_arg(char **cursor, char *out) {
	int n = 0;

    // Read the next token
	if (sscanf(*cursor, "%s%n", out, &n) != 1) return ERROR; 

	// Move the cursor forward by n bytes
	*cursor += n;
	return SUCCESS;
}

int is_end_of_message(char **cursor) {
	return (**cursor == EOM) ? TRUE : FALSE;
}

int skip_whitespaces(char **cursor) {
	while(**cursor == ' ' || **cursor == '\t') {
		(*cursor)++;
	}
	return SUCCESS;
}
int parse_cmd(char **cursor, char* cmd) {
	int err = get_next_arg(cursor, cmd);
	if (err == ERROR) return ERROR;
	return SUCCESS;
}

