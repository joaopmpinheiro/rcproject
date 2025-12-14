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
	while(**cursor == ' ' || **cursor == '\t') {
		(*cursor)++;
	}
	return (**cursor == EOM) ? SUCCESS : ERROR;
}

int parse_cmd(char **cursor, char* cmd) {
	int err = get_next_arg(cursor, cmd);
	if (err == ERROR) return ERROR;
	return SUCCESS;
}

ReplyStatus parse_uid(char **cursor, char* uid) {
	if (get_next_arg(cursor, uid) == ERROR) return STATUS_INVALID_ARGS;
	if (verify_uid_format(uid) == INVALID) return STATUS_INVALID_UID;
	return STATUS_UNASSIGNED;
}

ReplyStatus parse_password(char **cursor, char* password) {
	if (get_next_arg(cursor, password) == ERROR) return STATUS_INVALID_ARGS;
    if (verify_password_format(password) == INVALID) return STATUS_INVALID_PASSWORD;
	return STATUS_UNASSIGNED;
}

/* ReplyStatus parse_eid(char **cursor, EID *eid) {
	char eid_str[EID_STR_SIZE] = {0};

	bool err = get_next_arg(cursor, eid_str, EID_FORMAT "%n", 1);

	if (err) {
		return true;
	}

	// Validate UID format
	if (!is_valid_uid(eid_str)) {
		errno = EINVAL;
		return true;
	}

	*eid = atoi(eid_str);
	return false;
}
 */

ReplyStatus parse_datetime(char **cursor, char *date) {
	char day_str[DAY_STR_SIZE + 1] = {0};
	char time_str[TIME_STR_SIZE + 1] = {0};
	if (get_next_arg(cursor, day_str) == ERROR) return STATUS_INVALID_ARGS;
	if (get_next_arg(cursor, time_str) == ERROR) return STATUS_INVALID_ARGS;
	
	snprintf(date, EVENT_DATE_LENGTH + 1, "%s %s", day_str, time_str);
	if (!verify_event_date_format(date)) return STATUS_INVALID_EVENT_DATE;
	return STATUS_UNASSIGNED;
}
 
ReplyStatus parse_event_name(char **cursor, char* event_name) {
	int err = get_next_arg(cursor, event_name);
	if (err == ERROR) return STATUS_INVALID_ARGS;
	if (!verify_event_name_format(event_name)) return STATUS_INVALID_EVENT_NAME;
	return STATUS_UNASSIGNED;
}

ReplyStatus parse_total_seat(char **cursor, char *num_seats) {
	if (get_next_arg(cursor, num_seats) == ERROR) return STATUS_INVALID_ARGS;
	if (!verify_seat_count(num_seats)) return STATUS_INVALID_SEAT_COUNT;
	return STATUS_UNASSIGNED;
}

ReplyStatus parse_file_name(char **cursor, char* filename) {
	int err = get_next_arg(cursor, filename);
	if (err == ERROR) return STATUS_INVALID_ARGS;
	if (!verify_file_name_format(filename)) return STATUS_INVALID_FILE;
	return STATUS_UNASSIGNED;
}

/* ReplyStatus parse_file_size(char **cursor, size_t *file_size) {
	size_t size;
	int err = get_next_arg(cursor, &size, "%zu%n", 1);
	if (err) {
		return true;
	}

	if (!is_valid_file_size(size)) {
		errno = EINVAL;
		return true;
	}

	*file_size = size;
	return false;
}
 */

/*  void parse_file_data(char **cursor, FileData file_data, size_t file_size) {
	// Ignore white space before file data
	while (**cursor == ' ') {
		(*cursor)++;
	}

	// Copy file data
	memcpy(file_data, *cursor, file_size);
	file_data[file_size] = '\0';

	*cursor += file_size;
}
 */

/* bool parse_seats_to_reserve(char **cursor, int *seats_to_reserve) {
	bool err = get_next_arg(cursor, seats_to_reserve, "%d%n", 1);
	if (err) {
		return true;
	}

	return false;
}

bool parse_login(char **cursor, UID *uid, Password password) {
	// Parse UID
	bool err = parse_uid(cursor, uid);
	if (err) return true;

	// Parse password
	err = parse_password(cursor, password);
	if (err) return true;

	return false;
}

bool parse_login_request(char **cursor, UID *uid, Password password) {
	bool err = parse_login(cursor, uid, password);
    if (err) return true;

	if (!is_end_of_message(cursor)) return true;
	return false;
}

bool parse_show_event_details_request(char **cursor, EID *event_id) {
	bool err = parse_eid(cursor, event_id);
	if (err) return true;

	if (!is_end_of_message(cursor)) return true;
	return false;
}
 */
