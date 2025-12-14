#ifndef COMMON_DATA_H
#define COMMON_DATA_H

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

    STATUS_MALFORMED_COMMAND, // Command not recognized
    
    // Client-side errors (before/during communication)
    STATUS_INVALID_ARGS,    // Invalid argument count or format
    STATUS_INVALID_UID,     // Invalid UID format
    STATUS_INVALID_PASSWORD,// Invalid password format
    STATUS_INVALID_EVENT_NAME, // Invalid event name format
    STATUS_INVALID_EVENT_DATE, // Invalid event date format
    STATUS_INVALID_SEAT_COUNT, // Invalid seat count
    STATUS_INVALID_FILE,    // Invalid file format or size
    STATUS_FILE_NOT_FOUND,  // File does not exist
    STATUS_FILE_READ_ERROR, // Error reading file
    STATUS_FILE_SIZE_EXCEEDED, // File size exceeds limit
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


#endif // COMMON_DATA_H