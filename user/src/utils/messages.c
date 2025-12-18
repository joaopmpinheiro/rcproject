#include "../../include/utils.h"
#include "../../common/data.h"
#include "../../include/client_data.h"


#include <stdio.h>

void usage(const char *prog_name) {
    fprintf(stdout, "Usage: %s [-n server_ip] [-p server_port]\n", prog_name);
    fprintf(stdout, "  -n server_ip    Specify the server IP address\n");
    fprintf(stdout, "  -p server_port  Specify the server port number\n");
}

void print_result(RequestType command, ReplyStatus status, char* extra_info) {
    const char* cmd_name = command_to_str(command);
    
    switch (status) {
        // Success cases
        case STATUS_OK:
            switch (command) {
                case LOGIN:
                    printf("%s successful: User logged in\n", cmd_name);
                    break;
                case CHANGEPASS:
                    printf("%s successful: Password changed\n", cmd_name);
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
                case CLOSE:
                    printf("%s successful: Event closed\n", cmd_name);
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
                case CHANGEPASS:
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
                case CLOSE:
                    printf("%s failed: User does not exist or wrong password\n", cmd_name);
                    break;
                case LIST:
                    printf("%s: No event was created\n", cmd_name);
                    break;
                case SHOW:
                    printf("%s failed: No file or event does not exist\n", cmd_name);
                    break;
                case MYRESERVATIONS:
                    printf("%s: User has no reservations\n", cmd_name);
                    break;
                case RESERVE:
                    printf("%s failed: Event  not active\n", cmd_name);
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
        case STATUS_NO_EVENT_ID:
            printf("%s failed: Event ID does not exist\n", cmd_name);
            break;
        case STATUS_EVENT_WRONG_USER:
            printf("%s failed: Event not created by this user\n", cmd_name);
            break;
        case STATUS_EVENT_SOLD_OUT:
            printf("%s failed: Event has already sold out\n", cmd_name);
            break;
        case STATUS_PAST_EVENT:
            printf("%s failed: Event date has already passed\n", cmd_name);
            break;
        case STATUS_EVENT_CLOSED:
            printf("%s failed: Event was already closed\n", cmd_name);
            break;
        case STATUS_EVENT_CLOSE_CLOSED:
            printf("%s failed: Event was already closed\n", cmd_name);
            break;
        case STATUS_EVENT_RESERVED:
            printf("%s successful: Seats successfully reserved\n", cmd_name);
            break;      
        case STATUS_MALFORMED_COMMAND:
            printf("%s failed: Malformed command.\n Closing connection\n", cmd_name);
            break;
        case STATUS_ERROR:
            printf("%s failed: Server was unable to process the request\n", cmd_name);
            break;

        case CMD_ERROR:
            printf("%s failed: Command not recognized by server\n", cmd_name);
            break;
            
        // Client-side errors
        case STATUS_INVALID_ARGS:
            printf("%s failed: Invalid argument format\n", cmd_name);
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
            printf("%s failed: User not logged in (local)\n", cmd_name);
            break;
        case STATUS_SEND_FAILED:
            printf("%s failed: Failed to send request\n", cmd_name);
            break;
        case STATUS_RECV_FAILED:
            printf("%s failed: Failed to receive response\n", cmd_name);
            break;
        case STATUS_MALFORMED_RESPONSE:
            printf("%s failed: Malformed server response.\n Closing connection\n", cmd_name);
            break;
        case STATUS_UNEXPECTED_RESPONSE:
            printf("%s failed: Unexpected response code\n", cmd_name);
            break;
        case STATUS_UNEXPECTED_STATUS:
            printf("%s failed: Unexpected status\n", cmd_name);
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

void event_message(char* eid) {
    printf("Create successful: event created with EID: %s\n", eid);
}

void show_event_details(char* eid, char* uid, char* event_name, char* event_date,
                        char* total_seats, char* reserved_seats,
                        char* file_name, char* file_size) {
    printf("\n========== EVENT DETAILS ==========\n");
    printf("Event ID:         %s\n", eid);
    printf("Event Name:       %s\n", event_name);
    printf("Date & Time:      %s\n", event_date);
    printf("Created by:       %s\n", uid);
    printf("-----------------------------------\n");
    printf("Total Seats:      %s\n", total_seats);
    printf("Reserved Seats:   %s\n", reserved_seats);
    printf("-----------------------------------\n");
    printf("File Name:        %s\n", file_name);
    printf("File Size:        %s bytes\n", file_size);
    printf("===================================\n\n");
}

void show_events_list(int tcp_fd) {
    ReplyStatus status;
    char eid[4], name[MAX_EVENT_NAME + 1];
    char state[2];
    char event_day[EVENT_DATE_LENGTH + 1], event_time[EVENT_DATE_LENGTH + 1];
    
    printf("\n%-5s %-20s %-12s %-20s\n", "EID", "Name", "State", "Date & Time");
    printf("------------------------------------------------------------\n");
    
    status = read_events_list(tcp_fd, eid, name, state, event_day, event_time);
    while (status == STATUS_UNASSIGNED) {
        const char* state_str;
        switch (state[0]) {
            case '0': state_str = "Past"; break;
            case '1': state_str = "Active"; break;
            case '2': state_str = "Sold out"; break;
            case '3': state_str = "Closed"; break;
            default: state_str = "Unknown"; break;
        }
        printf("%-5s %-20s %-12s %s %s\n", eid, name, state_str, event_day, event_time);
        status = read_events_list(tcp_fd, eid, name, state, event_day, event_time);
    }
}

void show_event_reservations(char* seats_left, char* eid){
    printf("Reserve failed: There is only %s seats available at event with EID: %s \n", seats_left, eid);
}

ReplyStatus show_myreservations(char* cursor_lst){
    char eid[4], event_date[EVENT_DATE_LENGHT_W_SECONDS + 1], seats_reserved[4];
    printf("Your reservations:\n");
    printf("%-5s %-20s %-10s\n", "EID", "    Date & Time     ", "Seats Reserved");
    printf("----- -------------------- --------------\n");

    ReplyStatus status = STATUS_UNASSIGNED;
    status = parse_reservations(&cursor_lst, eid, event_date, seats_reserved);
    while (status == STATUS_UNASSIGNED) {
        printf("%-5s %-20s %-10s\n", eid, event_date, seats_reserved);
        status = parse_reservations(&cursor_lst, eid, event_date, seats_reserved);
    }
    return STATUS_CUSTOM_OUTPUT;
}
