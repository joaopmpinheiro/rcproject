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
    // Success statuses
    STATUS_OK,              // Operation successful
    STATUS_REGISTERED,      // REG - new user registered (login)
    
    // Server error statuses (from protocol)
    STATUS_NOK,             // Generic failure
    STATUS_NOT_LOGGED_IN,   // NLG - user not logged in
    STATUS_WRONG_PASSWORD,  // WRP - incorrect password
    STATUS_USER_NOT_REGISTERED, // UNR - user not registered
    STATUS_USER_NOT_FOUND,  // NID - user does not exist
    
    // Client-side errors (before/during communication)
    STATUS_INVALID_ARGS,    // Invalid argument count or format
    STATUS_INVALID_UID,     // Invalid UID format
    STATUS_INVALID_PASSWORD,// Invalid password format
    STATUS_SEND_FAILED,     // Failed to send request
    STATUS_RECV_FAILED,     // Failed to receive response
    STATUS_MALFORMED_RESPONSE, // Could not parse server response
    STATUS_UNEXPECTED_RESPONSE, // Unexpected response code
    
    // Special status
    STATUS_ALREADY_LOGGED_IN,   // User already logged in (for login)
    STATUS_NOT_LOGGED_IN_LOCAL, // User not logged in (client-side check)
    STATUS_CUSTOM_OUTPUT,   // Handler printed its own output
} ReplyStatus;

CommandType identify_command(char* command);

void command_handler(CommandType command, char* args, int udp_fd,
     struct sockaddr_in* server_udp_addr);

// Print result message based on command and status
void print_result(CommandType command, ReplyStatus status);

// Individual command handlers - return ReplyStatus
ReplyStatus login_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len);
ReplyStatus unregister_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len);
ReplyStatus logout_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len);
ReplyStatus myevent_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len);


#endif