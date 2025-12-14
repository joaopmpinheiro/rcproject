#include "../../include/utils.h"
#include <stdio.h>

void usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [-n server_ip] [-p server_port]\n", prog_name);
    fprintf(stderr, "  -n server_ip    Specify the server IP address\n");
    fprintf(stderr, "  -p server_port  Specify the server port number\n");
}

void print_result(CommandType command, ReplyStatus status, char* extra_info) {
    const char* cmd_name = get_command_name(command);
    
    switch (status) {
        // Success cases
        case STATUS_OK:
            switch (command) {
                case LOGIN:
                    printf("%s successful: User logged in\n", cmd_name);
                    break;
                case LOGOUT:
                    printf("%s successful: User logged out\n", cmd_name);
                    break;
                case UNREGISTER:
                    printf("%s successful: User unregistered\n", cmd_name);
                    break;
                case CREATE:
                    printf("%s successful: Event created with EID: %s\n", cmd_name, extra_info);
                    break;
                default:
                    printf("%s successful\n", cmd_name);
                    break;
            }
            break;

        case STATUS_REGISTERED:
            printf("%s successful: New user registered\n", cmd_name);
            break;
            
        // Server error cases
        case STATUS_NOK:
            switch (command) {
                case LOGIN:
                    printf("%s failed: Wrong password\n", cmd_name);
                    break;
                case LOGOUT:
                    printf("%s failed: User not logged in\n", cmd_name);
                    break;
                case UNREGISTER:
                    printf("%s failed: User not logged in\n", cmd_name);
                    break;
                case MYEVENTS:
                    printf("%s: User has no events\n", cmd_name);
                    break;
                case CREATE:
                    printf("%s failed: Event could not be created\n", cmd_name);
                    break;
                default:
                    printf("%s failed\n", cmd_name);
                    break;
            }
            break;
        case STATUS_NOT_LOGGED_IN:
            printf("%s failed: User not logged in\n", cmd_name);
            break;
        case STATUS_WRONG_PASSWORD:
            printf("%s failed: Wrong password\n", cmd_name);
            break;
        case STATUS_USER_NOT_REGISTERED:
            printf("%s failed: User not registered\n", cmd_name);
            break;
        case STATUS_USER_NOT_FOUND:
            printf("%s failed: User does not exist\n", cmd_name);
            break;

        case STATUS_ERROR:
            printf("%s failed: Server was unable to process the request\n", cmd_name);
            break;
            
        // Client-side errors
        case STATUS_INVALID_ARGS:
            printf("%s failed: Invalid argument count\n", cmd_name);
            break;
        case STATUS_INVALID_UID:
            printf("%s failed: Invalid UID format (must be 6 digits)\n", cmd_name);
            break;
        case STATUS_INVALID_PASSWORD:
            printf("%s failed: Invalid password format (must be 8 alphanumeric characters)\n", cmd_name);
            break;
        case STATUS_INVALID_EVENT_NAME:
            printf("%s failed: Invalid event name format\n", cmd_name);
            break;
        case STATUS_INVALID_EVENT_DATE:
            printf("%s failed: Invalid event date format (must be DD-MM-YYYY HH:MM)\n", cmd_name);
            break;
        case STATUS_INVALID_SEAT_COUNT:
            printf("%s failed: Invalid seat count\n", cmd_name);
            break;
        case STATUS_INVALID_FILE:
            printf("%s failed: Invalid file format or size\n", cmd_name);
            break;
        case STATUS_FILE_NOT_FOUND:
            printf("%s failed: File not found\n", cmd_name);
            break;
        case STATUS_FILE_READ_ERROR:
            printf("%s failed: Error reading file\n", cmd_name);
            break;
        case STATUS_FILE_SIZE_EXCEEDED:
            printf("%s failed: File size exceeds limit\n", cmd_name);
            break;
        case STATUS_NOT_LOGGED_IN_LOCAL:
            printf("%s failed: User not logged in\n", cmd_name);
            break;
        case STATUS_SEND_FAILED:
            printf("%s failed: Failed to send request\n", cmd_name);
            break;
        case STATUS_RECV_FAILED:
            printf("%s failed: Failed to receive response\n", cmd_name);
            break;
        case STATUS_MALFORMED_RESPONSE:
            printf("%s failed: Malformed server response\n", cmd_name);
            break;
        case STATUS_UNEXPECTED_RESPONSE:
            printf("%s failed: Unexpected response code\n", cmd_name);
            break;
        case STATUS_ALREADY_LOGGED_IN:
            printf("%s failed: User already logged in\n", cmd_name);
            break;
            
        // Special cases - no printing needed
        case STATUS_CUSTOM_OUTPUT:
            break;
        default:
            break;
    }
}