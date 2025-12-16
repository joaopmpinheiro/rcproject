#ifndef VERIFICATIONS_H
#define VERIFICATIONS_H

/**
 * @brief Checks if a string represents a valid number.
 * 
 * @param str 
 * @return int VALID if the string is a number, INVALID otherwise.
 */
int is_number(const char *str);

/**
 * @brief Checks if a string represents a valid port number.
 * 
 * @param port_str 
 * @return int VALID if the string is a valid port, INVALID otherwise.
 */
int is_valid_port(const char *port_str);

/**
 * @brief Checks if a string represents a valid seat count.
 * 
 * @param seat_str 
 * @return int VALID if the string is a valid seat count, INVALID otherwise.
 */
int is_valid_seat_count(const char *seat_str);

/**
 * @brief Verifies if the UID format is correct (6-digit number).
 * 
 * @param uid 
 * @return int VALID if the UID format is correct, INVALID otherwise.
 */
int verify_uid_format(char* uid);

/**
 * @brief Verifies if the event ID format is correct (number between 1 and 999).
 * 
 * @param eid 
 * @return int VALID if the event ID format is correct, INVALID otherwise.
 */
int verify_eid_format(char* eid);

/**
 * @brief Verifies if the password format is correct (8 alphanumeric characters).
 * 
 * @param pass 
 * @return int VALID if the password format is correct, INVALID otherwise.
 */
int verify_password_format(char* pass);

/**
 * @brief Verifies if the argument count is correct.
 * 
 * @param args - char*
 * @param expected_count - int
 * @return int VALID if the argument count is correct, INVALID otherwise.
 */
int verify_argument_count(char* args, int expected_count);

/**
 * @brief Verifies if the event name format is correct (1 to MAX_EVENT_NAME alphanumeric characters).
 * 
 * @param event_name 
 * @return int VALID if the event name format is correct, INVALID otherwise.
 */
int verify_event_name_format(char* event_name);

/**
 * @brief Verifies if the date format is correct (DD-MM-YYYY HH:MM).
 * 
 * @param date_str 
 * @return int VALID if the date format is correct, INVALID otherwise.
 */
int verify_event_date_format(char* date_str);

/**
 * @brief Verifies if the attendance count is within valid range.
 * 
 * @param count_str 
 * @return int VALID if the attendance count is valid, INVALID otherwise.
 */
int verify_seat_count(char* count_str);

/**
 * @brief Verifies if the reserved seats count is valid (a number and less than or equal to total seats).
 * 
 * @param reserved_str 
 * @param total_str 
 * @return int 
 */
int verify_reserved_seats(char* reserved_str, char* total_str);

/**
 * @brief Verifies if the file name format is correct (1 to FILE_NAME_LENGTH alphanumeric, dots, underscores, hyphens).
 * 
 * @param file_name 
 * @return int VALID if the file name format is correct, INVALID otherwise.
 */
int verify_file_name_format(char* file_name);

/**
 * @brief Verifies if the file size is within valid range. Number and less than MAX_FILE_SIZE.
 * @param file_size
 * @return int VALID if the file size is valid, INVALID otherwise.
 */
int verify_file_size(char* file_size);
#endif