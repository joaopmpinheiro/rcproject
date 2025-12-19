#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "verifications.h"
#include "data.h"

/**
 * @brief Extracts the next whitespace-delimited argument from the cursor position.
 * 
 * Uses sscanf to read a token and advances the cursor past the consumed characters.
 * 
 * @param cursor Pointer to cursor position in the input string (updated on success)
 * @param out Buffer to store the extracted argument
 * @return int SUCCESS if argument extracted, ERROR if no argument found
 */
int get_next_arg(char **cursor, char *out);

/**
 * @brief Checks if the cursor is at the end of message (newline character).
 * 
 * @param cursor Pointer to cursor position in the input string
 * @return int TRUE if at end of message, FALSE otherwise
 */
int is_end_of_message(char **cursor);

/**
 * @brief Parses a 3-letter command from the cursor position.
 * 
 * @param cursor Pointer to cursor position in the input string (updated on success)
 * @param cmd Buffer to store the command (must be at least 4 bytes)
 * @return int SUCCESS if command parsed, ERROR on failure
 */
int parse_cmd(char **cursor, char* cmd);

/**
 * @brief Checks if the cursor is at end of message, skipping leading whitespace.
 * 
 * Advances past spaces and tabs before checking for newline.
 * 
 * @param cursor Pointer to cursor position in the input string (may be updated)
 * @return int TRUE if at end of message after whitespace, FALSE otherwise
 */
int is_padded_end_of_message(char **cursor);

#endif
