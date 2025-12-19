#ifndef __COMMON_H__
#define __COMMON_H__

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "data.h"

#define BASE_PORT 58000
#define GROUP_NUMBER 32
#define DEFAULT_PORT "58032"
#define DEFAULT_IP "127.0.0.1" 
#define MAX_HOSTNAME_LENGTH 256
#define PORTMAX 65535
#define PORTMIN 0

#define TRUE 1
#define FALSE 0

#define TIMEOUT_SECONDS 5
#define PASSWORD_LENGTH 8
#define COMMAND_LENGTH 3
#define UID_LENGTH 6
#define EID_LENGTH 3
#define SEAT_COUNT_LENGTH 3
#define DAY_STR_SIZE 10
#define TIME_STR_SIZE 5
#define EVENT_DATE_LENGTH 16 // DD-MM-YYYY HH:MM
#define EVENT_DATE_LENGHT_W_SECONDS 19 // DD-MM-YYYY HH:MM:SS
#define FILE_NAME_LENGTH 24
#define FILE_SIZE_LENGTH 8
#define MAX_EVENTS 999
#define MAX_EVENT_NAME 10
#define MAX_AVAIL_SEATS 999
#define MIN_AVAIL_SEATS 10
#define MAX_EVENT_NAME 10 
#define DATE_LENGTH 11
#define TIME_LENGTH 5
#define SHOW_BUFFER_SIZE (EID_LENGTH + MAX_EVENT_NAME + EVENT_DATE_LENGTH + SEAT_COUNT_LENGTH * 2 + FILE_NAME_LENGTH + 36)

#define MAX_FILE_SIZE (1024 * 1024 * 10) // 10 MB
#define TCP_BUFFER_SIZE 1024
#define BUFFER_SIZE 256

#define LOGGED_IN 1
#define LOGGED_OUT 0

#define SUCCESS 1
#define FAILURE 0

#define ERROR -1

#define VALID 1
#define INVALID 0

#define EOM '\n'

/**
 * @brief Sends a message over a TCP connection, handling partial writes.
 * 
 * @param fd File descriptor of the TCP socket
 * @param message Null-terminated message string to send
 * @return int SUCCESS if message sent completely, ERROR on failure
 */
int tcp_send_message(int fd, char *message);

/**
 * @brief Sends a file's contents over a TCP connection.
 * 
 * Reads the file in chunks and sends each chunk, handling partial writes.
 * Appends a newline character to indicate end of file transfer.
 * 
 * @param fd File descriptor of the TCP socket
 * @param file_name Path to the file to send
 * @return int SUCCESS if file sent completely, ERROR on failure
 */
int tcp_send_file(int fd, char *file_name);

/**
 * @brief Reads data from a TCP socket until newline or buffer is full.
 * 
 * @param fd File descriptor of the TCP socket
 * @param buf Buffer to store the read data
 * @param len Maximum number of bytes to read
 * @return int SUCCESS on completion
 */
int tcp_read(int fd, void *buf, size_t len);

/**
 * @brief Writes data to a TCP socket, handling partial writes.
 * 
 * @param fd File descriptor of the TCP socket
 * @param buffer Data buffer to write
 * @param length Number of bytes to write
 * @return int SUCCESS if all bytes written, ERROR on failure
 */
int tcp_write(int fd, const char* buffer, size_t length);

/**
 * @brief Reads a single space-delimited field from a TCP socket.
 * 
 * Skips leading spaces and reads until the next space or newline.
 * 
 * @param fd File descriptor of the TCP socket
 * @param buffer Buffer to store the field
 * @param max_len Maximum length of the field
 * @return int SUCCESS if terminated by space, EOM if terminated by newline, ERROR on failure
 */
int tcp_read_field(int fd, char* buffer, size_t max_len);

/**
 * @brief Receives a file over TCP and writes it to disk.
 * 
 * @param fd File descriptor of the TCP socket
 * @param file_name Path where the file will be saved
 * @param file_size Expected size of the file in bytes
 * @return int SUCCESS if file received completely, ERROR on failure
 */
int tcp_read_file(int fd, char *file_name, long file_size);

/**
 * @brief From command RequestType, get human-readable command name.
 * 
 * @param command RequestType
 * @return const char* 
 */
const char* command_to_str(RequestType command);

/**
 * @brief Convert command RequestType to its 3-letter protocol code.
 * 
 * User -> Server
 * 
 * RequestType -> "XXX"
 * 
 * @param command RequestType
 * @return const char* 
 */
const char* get_command_request(RequestType command);

/**
 * @brief Convert 3-letter command request code to RequestType.
 * 
 * User -> Server
 * 
 * "XXX" -> RequestType
 * 
 * @param command_buff 
 * @return RequestType 
 */
RequestType identify_command_request(char* command_buff);

/**
 * @brief Convert 3-letter command response code to RequestType.
 * 
 * Server -> User
 * 
 * "XXX" -> RequestType
 * 
 * @param command 
 * @return RequestType 
 */
RequestType identify_command_response(char* command);

/**
 * @brief Convert command RequestType to its 3-letter protocol response code.
 * 
 * Server -> User
 * 
 * RequestType -> "XXX"
 * 
 * @param command 
 * @return const char* 
 */
const char* get_command_response_code(RequestType command);


/**
 * @brief Convert 3-letter status code to ReplyStatus enum.
 * 
 * Server -> User
 * 
 * "XXX" -> ReplyStatus
 * 
 * @param status 
 * @return ReplyStatus 
 */
ReplyStatus identify_status_code(const char* status);

/**
 * @brief Convert ReplyStatus enum to its 3-letter status code.
 * 
 * Server -> User
 * 
 * ReplyStatus -> "XXX"
 * 
 * @param status 
 * @return const char* 
 */
const char* get_status_code(ReplyStatus status);


#endif
