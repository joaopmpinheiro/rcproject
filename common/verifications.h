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
 * @brief Verifies if the UID format is correct (6-digit number).
 * 
 * @param uid 
 * @return int VALID if the UID format is correct, INVALID otherwise.
 */
int verify_uid_format(char* uid);

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

#endif