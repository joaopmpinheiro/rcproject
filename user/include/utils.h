#ifndef COMMAND_HANDLERS_H
#define COMMAND_HANDLERS_H

#include <netdb.h>

typedef enum CommandType {
    LOGIN,
    CHANGEPASS,
    UNREGISTER,
    LOGOUT,
    EXIT,
    CREATE,
    CLOSE,
    MYEVENTS,
    LIST,
    SHOW,
    RESERVE,
    MYRESERVATIONS,
    UNKNOWN,
} CommandType;

// Reply status codes returned by handlers
typedef enum ReplyStatus {
    STATUS_ERROR,          // ERR - generic error, server wasn't able to process request

    // Success statuses
    STATUS_OK,              // Operation successful
    STATUS_REGISTERED,      // REG - new user registered (login)
    
    // Server error statuses (from protocol)
    STATUS_NOK,             // NOK - Generic failure
    STATUS_NOT_LOGGED_IN,   // NLG - user not logged in
    STATUS_WRONG_PASSWORD,  // WRP - incorrect password
    STATUS_USER_NOT_REGISTERED, // UNR - user not registered
    STATUS_USER_NOT_FOUND,  // NID - user does not exist

    
    // Client-side errors (before/during communication)
    STATUS_INVALID_ARGS,    // Invalid argument count or format
    STATUS_INVALID_UID,     // Invalid UID format
    STATUS_INVALID_PASSWORD,// Invalid password format
    STATUS_INVALID_EVENT_NAME, // Invalid event name format
    STATUS_INVALID_EVENT_DATE, // Invalid event date format
    STATUS_INVALID_SEAT_COUNT, // Invalid seat count
    STATUS_INVALID_FILE,    // Invalid file format or size
    STATUS_SEND_FAILED,     // Failed to send request
    STATUS_RECV_FAILED,     // Failed to receive response
    STATUS_MALFORMED_RESPONSE, // Could not parse server response
    STATUS_UNEXPECTED_RESPONSE, // Unexpected response code
    
    // Special status
    STATUS_ALREADY_LOGGED_IN,   // User already logged in (for login)
    STATUS_NOT_LOGGED_IN_LOCAL, // User not logged in (client-side check)
    STATUS_CUSTOM_OUTPUT,   // Handler printed its own output

    STATUS_UNASSIGNED,
} ReplyStatus;


// ------------ command_handler.c -------------
ReplyStatus handle_response_code(char* resp, char* command, int parsed, int n, char* status);
CommandType identify_command(char* command);
const char* get_command_name(CommandType command);
ReplyStatus parse_status_code(const char* status);
void command_handler(CommandType command, char* args, int udp_fd,
     struct sockaddr_in* server_udp_addr);


// ------------ commands.c -------------
int verify_file(char* file_name);
ReplyStatus login_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
            socklen_t udp_addr_len);
ReplyStatus unregister_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
                                socklen_t udp_addr_len);
ReplyStatus logout_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len);
ReplyStatus myevent_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
                            socklen_t udp_addr_len);
/**
 * @brief Sends TCP request to create a new event and handles the response.
 * Receives:
 * NOK - event could not be created
 * NGL - user not logged in
 * OK EID- event created successfully 
 * PROTOCOL: CRE <uid> <password> <name> <event_date> <attendance_size> <Fname> <Fsize> <Fdata>
 * 
 * @param args [event_name event_file_name event_date num_seats]
 * @return ReplyStatus 
 */
ReplyStatus create_event_handler(char* args, char** extra_info);


// ---------- messages.c ----------
void usage(const char *prog_name);
void print_result(CommandType command, ReplyStatus status, char* extra_info);


// ---------- socket_manager.c ----------
int setup_udp(const char* ip, const char* port, struct sockaddr_in* server_addr);
int connect_tcp(const char* ip, const char* port);
ReplyStatus udp_send_receive(int udp_fd, struct sockaddr_in* server_udp_addr,
                            socklen_t udp_addr_len, char* request, char* response);



#endif