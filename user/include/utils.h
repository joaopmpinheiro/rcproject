#ifndef COMMAND_HANDLERS_H
#define COMMAND_HANDLERS_H

#include "../common/common.h"
#include <netdb.h>


// =============== command_handler.c ===============

/**
 * @brief Handles the response code from the server after receiving a reply.
 * 
 * @param resp Response buffer containing server reply
 * @param command The command type that was sent
 * @param parsed Number of successfully parsed fields
 * @param n Expected number of fields
 * @param status Status string extracted from response
 * @return ReplyStatus Parsed status or error code
 */
ReplyStatus handle_response_code(char* resp, RequestType command, int parsed, int n, char* status);

/**
 * @brief Identifies the command type from user input string.
 * 
 * @param command User input command string
 * @return RequestType The identified command type or UNKNOWN
 */
RequestType identify_command(char* command);

/**
 * @brief Converts a status code string to ReplyStatus enum.
 * 
 * @param status 3-letter status code from server
 * @return ReplyStatus The corresponding enum value
 */
ReplyStatus identify_status_code(const char* status);

/**
 * @brief Main command dispatcher that routes user commands to appropriate handlers.
 * 
 * @param command The type of command to execute
 * @param cursor Pointer to remaining command arguments
 * @param udp_fd UDP socket file descriptor
 * @param server_udp_addr Server UDP address structure
 * @return ReplyStatus Result of command execution
 */
ReplyStatus command_handler(RequestType command, char** cursor, int udp_fd,
                           struct sockaddr_in* server_udp_addr);

/**
 * @brief Converts a RequestType to its 3-letter response code.
 * 
 * @param command The command type
 * @return const char* 3-letter response code (e.g., "RLI" for LOGIN)
 */
const char* get_command_response_code(RequestType command);

/**
 * @brief Converts a 3-letter response code to RequestType.
 * 
 * @param command 3-letter response code from server
 * @return RequestType The corresponding command type
 */
RequestType identify_command_response(char* command);


// =============== commands.c ===============

/**
 * @brief Verifies that a file exists and is readable.
 * 
 * @param file_name Path to the file to verify
 * @return int TRUE if file is valid, FALSE otherwise
 */
int verify_file(char* file_name);

/**
 * @brief Does the login of a user or creates a new user if not registered.
 * 
 * USER INPUT: login UID password 
 * 
 * USER PROTOCOL: UID password 
 * 
 * SERVER PROTOCOL: RLI status
 *  
 * @param cursor 
 * @param udp_fd 
 * @param server_udp_addr 
 * @param udp_addr_len 
 * @return ReplyStatus
 */
ReplyStatus login_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
            socklen_t udp_addr_len);

/**
 * @brief Unregisters the logged-in user.
 * 
 * USER INPUT: unregister
 * 
 * USER PROTOCOL: UNR <uid> <password>
 * 
 * SERVER PROTOCOL: RUR <status>
 * 
 * @param cursor 
 * @param udp_fd 
 * @param server_udp_addr 
 * @param udp_addr_len 
 * @return ReplyStatus 
 */
ReplyStatus unregister_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
                                socklen_t udp_addr_len);

/**
 * @brief Logs out the logged-in user.
 * 
 * USER INPUT: logout
 * 
 * USER PROTOCOL: LOU <uid> <password>
 * 
 * SERVER PROTOCOL: RLO <status>
 * 
 * @param cursor 
 * @param udp_fd 
 * @param server_udp_addr 
 * @param udp_addr_len 
 * @return ReplyStatus 
 */
ReplyStatus logout_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len);

/**
 * @brief Asks the server for the events created by the logged-in user.
 * USER INPUT: myevents or mye
 * USER PROTOCOL: LME <uid> <password>
 * SERVER PROTOCOL: RME <status> [<event1ID state>...]*
 * 
 * @param cursor 
 * @param udp_fd 
 * @param server_udp_addr 
 * @param udp_addr_len 
 * @return ReplyStatus 
 */
ReplyStatus myevent_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
                            socklen_t udp_addr_len);

/**
 * @brief Lists the events reserved by the logged-in user (by up to 50 events).
 * USER INPUT: myreservations or myres
 * USER PROTOCOL: LMR <uid> <password>
 * SERVER PROTOCOL: RMR <status> [<event1ID name event_date seats
 * reserved> <event2ID name event_date seats_reserved> ...]
 * @param 
 */
ReplyStatus myreservations_handler(char** cursor, int udp_fd,
                                struct sockaddr_in* server_udp_addr,
                                socklen_t udp_addr_len);                      


                             
/**
 * @brief Changes the password of the logged-in user.
 * 
 * USER INPUT: changePass old_password new_password
 * 
 * USER PROTOCOL: CPS UID oldPassword newPassword
 * 
 * SERVER PROTOCOL: RCP <status>
 * 
 * @param cursor 
 * @return ReplyStatus 
 */
ReplyStatus changepass_handler(char** cursor);

/**
 * @brief Create a new event by the logged in user.
 * 
 * USER INPUT: create <event_name> <file_name> <event_date> <num_seats> 
 * 
 * USER PROTOCOL: CRE <uid> <password> <name> <event_date> <attendance_size>
 * <Fname> <Fsize> <Fdata>
 * 
 * SERVER PROTOCOL: RCE <status> [<eid>]
 * 
 * @param cursor 
 * @param extra_info 
 * @return ReplyStatus 
 */
ReplyStatus create_event_handler(char** cursor, char** extra_info);

/**
 * @brief Closes an event created by the logged-in user.
 * 
 * USER INPUT: close <eid>
 * 
 * USER PROTOCOL: CLS <uid> <password> <eid>
 * 
 * SERVER PROTOCOL: RCL <status>
 * 
 * @param cursor 
 * @return ReplyStatus 
 */
ReplyStatus close_event_handler(char** cursor);

/**
 * @brief Lists all events in the server.
 * 
 * USER INPUT: list
 * 
 * USER PROTOCOL: LST
 * 
 * SERVER PROTOCOL: RST <status> [<event1ID> <name> <state> <date>] * 
 * 
 * @param cursor 
 * @return ReplyStatus 
 */
ReplyStatus list_handler(char** cursor);

/**
 * @brief Shows the details of a specific event.
 * 
 * USER INPUT: show <eid>
 * 
 * USER PROTOCOL: SED <eid>
 * 
 * SERVER PROTOCOL: RSE status [UID name event_date attendance_size Seats_reserved Fname
 * Fsize Fdata]
 * 
 * @param cursor 
 * @return ReplyStatus 
 */
ReplyStatus show_handler(char** cursor);

/**
 * @brief Reserve seats for an event.
 * 
 * USER INPUT: reserve <eid> <num_seats>
 * 
 * USER PROTOCOL: RID UID password EID people.
 * 
 * SEVER PROTOCOL: RRI status [ n_seats ]*.
 * 
 * @param cursor 
 * @return ReplyStatus 
 */
ReplyStatus reserve_handler(char** cursor);


// =============== messages.c ===============

/**
 * @brief Prints usage information for the user client.
 * 
 * @param prog_name Name of the executable
 */
void usage(const char *prog_name);

/**
 * @brief Prints the result of a command execution to the user.
 * 
 * @param command The command that was executed
 * @param status The result status from the server
 * @param extra_info Additional information (e.g., EID for create)
 */
void print_result(RequestType command, ReplyStatus status, char* extra_info);

/**
 * @brief Prints a message about an event.
 * 
 * @param eid Event ID
 */
void event_message(char* eid);

/**
 * @brief Displays detailed event information to the user.
 * 
 * @param eid Event ID
 * @param uid Creator's user ID
 * @param event_name Event name
 * @param event_date Event date and time
 * @param total_seats Total seats available
 * @param reserved_seats Number of reserved seats
 * @param file_name Description filename
 * @param file_size Size of description file
 */
void show_event_details(char* eid, char* uid, char* event_name, char* event_date,
                        char* total_seats, char* reserved_seats,
                        char* file_name, char* file_size);

/**
 * @brief Displays the list of all events from the server.
 * 
 * @param tcp_fd TCP socket file descriptor
 */
void show_events_list(int tcp_fd);

/**
 * @brief Displays reservation result information.
 * 
 * @param seats_left Number of seats remaining (for rejection case)
 * @param eid Event ID
 */
void show_event_reservations(char* seats_left, char* eid);

/**
 * @brief Displays the user's reservations list.
 * 
 * @param cursor_lst Cursor to the reservations list data
 * @return ReplyStatus STATUS_OK on success
 */
ReplyStatus show_myreservations(char* cursor_lst);


// =============== socket_manager.c ===============

/**
 * @brief Sets up a UDP socket and configures the server address.
 * 
 * @param ip Server IP address
 * @param port Server port number
 * @param server_addr Pointer to sockaddr_in to be configured
 * @return int Socket file descriptor on success, ERROR on failure
 */
int setup_udp(const char* ip, const char* port, struct sockaddr_in* server_addr);

/**
 * @brief Establishes a TCP connection to the server.
 * 
 * @param ip Server IP address
 * @param port Server port number
 * @return int Socket file descriptor on success, ERROR on failure
 */
int connect_tcp(const char* ip, const char* port);

/**
 * @brief Sends a UDP request and receives the response.
 * 
 * Handles timeout and retransmission.
 * 
 * @param udp_fd UDP socket file descriptor
 * @param server_udp_addr Server address structure
 * @param udp_addr_len Length of address structure
 * @param request Request message to send
 * @param response Buffer to store response
 * @param response_size Size of response buffer
 * @return ReplyStatus STATUS_OK on success, error status on failure
 */
ReplyStatus udp_send_receive(int udp_fd, struct sockaddr_in* server_udp_addr,
                            socklen_t udp_addr_len, char* request, char* response, 
                            size_t response_size);

/**
 * @brief Sends a TCP request and receives the response.
 * 
 * Opens connection, sends request, receives response, and closes connection.
 * 
 * @param request Request message to send
 * @param response Buffer to store response
 * @param response_size Size of response buffer
 * @return ReplyStatus STATUS_OK on success, error status on failure
 */
ReplyStatus tcp_send_receive(char* request, char* response, size_t response_size);


// =============== user_parser.c ===============

/**
 * @brief Parses an event ID from the cursor position.
 * 
 * @param cursor Pointer to cursor in input string
 * @param eid Buffer to store the parsed EID
 * @return ReplyStatus STATUS_OK on success, error status on failure
 */
ReplyStatus parse_eid(char **cursor, char* eid);

/**
 * @brief Parses login credentials from the cursor position.
 * 
 * @param cursor Pointer to cursor in input string
 * @param uid Buffer to store the parsed UID
 * @param password Buffer to store the parsed password
 * @return ReplyStatus STATUS_OK on success, error status on failure
 */
ReplyStatus parse_login(char **cursor, char* uid, char* password);

/**
 * @brief Parses reservation parameters from the cursor position.
 * 
 * @param cursor Pointer to cursor in input string
 * @param eid Buffer to store the event ID
 * @param num_seats Buffer to store the number of seats
 * @return ReplyStatus STATUS_OK on success, error status on failure
 */
ReplyStatus parse_reserve(char **cursor, char* eid, char* num_seats);

/**
 * @brief Parses create event parameters from the cursor position.
 * 
 * @param cursor Pointer to cursor in input string
 * @param event_name Buffer to store event name
 * @param file_name Buffer to store description filename
 * @param date Buffer to store event date
 * @param num_seats Buffer to store seat count
 * @return ReplyStatus STATUS_OK on success, error status on failure
 */
ReplyStatus parse_create_event(char **cursor, char* event_name, char* file_name,
                             char* date, char* num_seats);

/**
 * @brief Parses change password parameters from the cursor position.
 * 
 * @param cursor Pointer to cursor in input string
 * @param old_password Buffer to store old password
 * @param new_password Buffer to store new password
 * @param current_password Current stored password for validation
 * @return ReplyStatus STATUS_OK on success, error status on failure
 */
ReplyStatus parse_change_password(char** cursor, char* old_password,
                                 char* new_password, char* current_password);

/**
 * @brief Parses reservation details from the cursor position.
 * 
 * @param cursor Pointer to cursor in input string
 * @param eid Buffer to store event ID
 * @param event_date Buffer to store reservation date
 * @param seats_reserved Buffer to store number of seats
 * @return ReplyStatus STATUS_OK on success, error status on failure
 */
ReplyStatus parse_reservations(char **cursor, char* eid, char* event_date,
                              char* seats_reserved);

/**
 * @brief Parses and validates the response header from server.
 * 
 * @param cursor Pointer to cursor in response string
 * @param request_type Expected request type for validation
 * @return ReplyStatus Parsed status or error
 */
ReplyStatus parse_response_header(char** cursor, RequestType request_type);


// =============== read_from_server.c ===============

/**
 * @brief Reads the command code from TCP response.
 * 
 * @param tcp_fd TCP socket file descriptor
 * @param command Buffer to store the command code
 * @param expected_command Expected command type for validation
 * @return ReplyStatus STATUS_OK on success, error on mismatch
 */
ReplyStatus read_command(int tcp_fd, char* command, RequestType expected_command);

/**
 * @brief Reads the command and status from the server response.
 * 
 * Verifies they match the expected command and parses the status.
 * 
 * @param tcp_fd TCP socket file descriptor
 * @param expected_command Expected command type
 * @return ReplyStatus Parsed status or STATUS_UNEXPECTED_RESPONSE
 */
ReplyStatus read_cmd_status(int tcp_fd, RequestType expected_command);

/**
 * @brief Reads the show event response header fields.
 * 
 * @param tcp_fd TCP socket file descriptor
 * @param uid Buffer to store creator UID
 * @param event_name Buffer to store event name
 * @param event_date Buffer to store event date
 * @param attendance_size Buffer to store total seats
 * @param reserved_seats Buffer to store reserved count
 * @param file_name Buffer to store description filename
 * @param file_size Buffer to store file size
 * @return ReplyStatus STATUS_OK on success, error on failure
 */
ReplyStatus read_show_response_header(int tcp_fd,
                                       char* uid, char* event_name,
                                       char* event_date, char* attendance_size,
                                       char* reserved_seats, char* file_name,
                                       char* file_size);

/**
 * @brief Reads a single event entry from the events list.
 * 
 * @param fd_tcp TCP socket file descriptor
 * @param eid Buffer to store event ID
 * @param name Buffer to store event name
 * @param state Buffer to store event state
 * @param event_day Buffer to store event day
 * @param event_time Buffer to store event time
 * @return ReplyStatus STATUS_OK if more events, EOM at end of list
 */
ReplyStatus read_events_list(int fd_tcp, char* eid, char* name, char* state,
                              char* event_day, char* event_time);

#endif