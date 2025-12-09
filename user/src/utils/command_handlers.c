#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>   // stat, S_ISREG
#include <unistd.h>    // access, R_OK

#include "command_handlers.h"
#include "../../common/verifications.h"
#include "../../common/common.h"

#include "client_data.h"

// Helper function to get command name string
static const char* get_command_name(CommandType command) {
    switch (command) {
        case LOGIN: return "Login";
        case CHANGEPASS: return "Change password";
        case UNREGISTER: return "Unregister";
        case LOGOUT: return "Logout";
        case EXIT: return "Exit";
        case CREATE: return "Create";
        case CLOSE: return "Close";
        case MYEVENTS: return "My events";
        case LIST: return "List";
        case SHOW: return "Show";
        case RESERVE: return "Reserve";
        case MYRESERVATIONS: return "My reservations";
        default: return "Unknown";
    }
}

// Centralized result printing
void print_result(CommandType command, ReplyStatus status) {
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

// Helper to parse common protocol status codes
static ReplyStatus parse_status_code(const char* status) {
    if (strcmp(status, "OK") == 0) return STATUS_OK;
    if (strcmp(status, "NOK") == 0) return STATUS_NOK;
    if (strcmp(status, "REG") == 0) return STATUS_REGISTERED;
    if (strcmp(status, "NLG") == 0) return STATUS_NOT_LOGGED_IN;
    if (strcmp(status, "WRP") == 0) return STATUS_WRONG_PASSWORD;
    if (strcmp(status, "UNR") == 0) return STATUS_USER_NOT_REGISTERED;
    if (strcmp(status, "NID") == 0) return STATUS_USER_NOT_FOUND;
    return STATUS_UNEXPECTED_RESPONSE;
}

CommandType identify_command(char* command) {
    if (strcmp(command, "login") == 0) return LOGIN;
    else if (strcmp(command, "changePass") == 0) return CHANGEPASS;
    else if (strcmp(command, "unregister") == 0) return UNREGISTER;
    else if (strcmp(command, "logout") == 0) return LOGOUT;
    else if (strcmp(command, "exit") == 0) return EXIT;
    else if (strcmp(command, "create") == 0) return CREATE;
    else if (strcmp(command, "close") == 0) return CLOSE;
    else if (strcmp(command, "myevents") == 0 || strcmp(command, "mye") == 0) return MYEVENTS;
    else if (strcmp(command, "list") == 0) return LIST;
    else if (strcmp(command, "show") == 0) return SHOW;
    else if (strcmp(command, "reserve") == 0) return RESERVE;
    else if (strcmp(command, "myreservations") == 0 || strcmp(command, "myr") == 0) return MYRESERVATIONS;
    else return UNKNOWN;
}

ReplyStatus login_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len) {
    char uid[32];
    char password[32];
    ssize_t n;

    if (is_logged_in) return STATUS_ALREADY_LOGGED_IN;

    if (!verify_argument_count(args, 2)) return STATUS_INVALID_ARGS;
    sscanf(args, "%31s %31s", uid, password);

    if (!verify_uid_format(uid)) return STATUS_INVALID_UID;
    if (!verify_password_format(password)) return STATUS_INVALID_PASSWORD;

    char request[256];

    // PROTOCOL: LIN <uid> <password>
    snprintf(request, sizeof(request), "LIN %s %s\n", uid, password);

    if (sendto(udp_fd, request, strlen(request), 0, (struct sockaddr*)server_udp_addr,
                udp_addr_len) == ERROR) return STATUS_SEND_FAILED;


    char response[256];
    n = recvfrom(udp_fd, response, sizeof(response) - 1, 0, NULL, NULL);
    if (n == ERROR) return STATUS_RECV_FAILED;

    response[n] = '\0';
    char response_code[4];
    char reply_status[4];

    int parsed = sscanf(response, "%3s %3s", response_code, reply_status);

    if (parsed < 2) return STATUS_MALFORMED_RESPONSE;
    if (strcmp(response_code, "RLI") != 0)  return STATUS_UNEXPECTED_RESPONSE;

    ReplyStatus status = parse_status_code(reply_status);

    // Update global state on successful login
    if (status == STATUS_OK || status == STATUS_REGISTERED) {
        is_logged_in = 1;
        strcpy(current_password, password);
        strcpy(current_uid, uid);
    }

    return status;
}

ReplyStatus unregister_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
                                socklen_t udp_addr_len) {
    ssize_t n;
    if (!verify_argument_count(args, 0)) return STATUS_INVALID_ARGS;
    if (!is_logged_in) return STATUS_NOT_LOGGED_IN_LOCAL;

    char request[256];

    // PROTOCOL: UNR <uid> <password>
    snprintf(request, sizeof(request), "UNR %s %s\n", current_uid, current_password);

    if (sendto(udp_fd, request, strlen(request), 0, (struct sockaddr*)server_udp_addr,\
        udp_addr_len) == ERROR) return STATUS_SEND_FAILED;

    char response[256];
    n = recvfrom(udp_fd, response, sizeof(response) - 1, 0, NULL, NULL);
    if (n == ERROR) return STATUS_RECV_FAILED;

    response[n] = '\0';

    char response_code[4];
    char reply_status[4];

    int parsed = sscanf(response, "%3s %3s", response_code, reply_status);

    if (parsed < 2) return STATUS_MALFORMED_RESPONSE;

    if (strcmp(response_code, "RUR") != 0) return STATUS_UNEXPECTED_RESPONSE;

    ReplyStatus status = parse_status_code(reply_status);

    // Clear global state on successful unregister
    if (status == STATUS_OK) {
        is_logged_in = 0;
        memset(current_password, 0, sizeof(current_password));
        memset(current_uid, 0, sizeof(current_uid));
    }

    return status;
}

ReplyStatus logout_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len) {

    ssize_t n;

    if (!verify_argument_count(args, 0)) return STATUS_INVALID_ARGS;

    if (!is_logged_in) return STATUS_NOT_LOGGED_IN_LOCAL;

    char request[256];

    // PROTOCOL: LOU <uid> <password>
    snprintf(request, sizeof(request), "LOU %s %s\n", current_uid, current_password);

    if (sendto(udp_fd, request, strlen(request), 0, (struct sockaddr*)server_udp_addr,
                udp_addr_len) == ERROR) return STATUS_SEND_FAILED;

    char response[256];
    n = recvfrom(udp_fd, response, sizeof(response) - 1, 0, NULL, NULL);
    if (n == ERROR) return STATUS_RECV_FAILED;

    response[n] = '\0';

    char response_code[4];
    char reply_status[4];

    int parsed = sscanf(response, "%3s %3s", response_code, reply_status);

    if (parsed < 2) return STATUS_MALFORMED_RESPONSE;

    if (strcmp(response_code, "RLO") != 0) return STATUS_UNEXPECTED_RESPONSE;

    ReplyStatus status = parse_status_code(reply_status);

    // Clear global state on successful logout
    if (status == STATUS_OK) {
        is_logged_in = 0;
        memset(current_password, 0, sizeof(current_password));
        memset(current_uid, 0, sizeof(current_uid));
    }

    return status;
}

ReplyStatus myevent_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
                            socklen_t udp_addr_len) {
    ssize_t n;
    if (!verify_argument_count(args, 0)) return STATUS_INVALID_ARGS;
    if (!is_logged_in) return STATUS_NOT_LOGGED_IN_LOCAL;

    char request[256];

    // PROTOCOL: LME <uid> <password>
    snprintf(request, sizeof(request), "LME %s %s\n", current_uid, current_password);

    if (sendto(udp_fd, request, strlen(request), 0, (struct sockaddr*)server_udp_addr,
                udp_addr_len) == ERROR) return STATUS_SEND_FAILED;

    char response[8192];
    n = recvfrom(udp_fd, response, sizeof(response) - 1, 0, NULL, NULL);
    if (n == ERROR) return STATUS_RECV_FAILED;

    response[n] = '\0';

    char response_code[4];
    char reply_status[4];

    int parsed = sscanf(response, "%3s %3s", response_code, reply_status);

    if (parsed < 2) return STATUS_MALFORMED_RESPONSE;

    if (strcmp(response_code, "RME") != 0) return STATUS_UNEXPECTED_RESPONSE;

    ReplyStatus status = parse_status_code(reply_status);

    // PROTOCOL: RME <status>[ <event1ID state> <event2ID state> ...]
    if (status == STATUS_OK) {
        char* event_list = response + 7;
        
        printf("Your events:\n");
        printf("%-5s %-10s\n", "EID", "State");
        printf("-------------------\n");

        if (strlen(event_list) == 0) {
            printf("(no events)\n");
            return STATUS_CUSTOM_OUTPUT;
        }

        char eid[4];
        int state;
        int offset = 0;
        int chars_read;

        while (sscanf(event_list + offset, " %3s %d%n", eid, &state, &chars_read) == 2) {
            // Validate EID format (3 digits)
            if (strlen(eid) != 3) {
                printf("Warning: Invalid EID format in response\n");
                break;
            }

            const char* state_str;
            switch (state) {
                case 0: state_str = "Past"; break;
                case 1: state_str = "Active"; break;
                case 2: state_str = "Sold out"; break;
                case 3: state_str = "Closed"; break;
                default: state_str = "Unknown"; break;
            }
            printf("%-5s %-10s\n", eid, state_str);
            offset += chars_read;
        }
        return STATUS_CUSTOM_OUTPUT;
    }

    return status;
}

/*
create name event_fname event_date num_attendees – the
User application establishes a TCP session with the ES and sends a message
asking to create a new event, whose short description name is name, providing
the file describing the event, stored in the file event_fname, indicating the date
and time (dd-mm-yyyy hh:mm) of the event, event_date, and the number of
people who can attend the event, num_attendees. the file event_fname
should exist in the same folder.
In reply, the ES sends a message indicating whether the request was successful,
and the assigned event identifier, EID, which should be displayed to the user.
After receiving the reply from the ES, the User closes the TCP connection.
*/


int verify_file(char* file_name) {
    struct stat st;
    return (stat(file_name, &st) == 0 && //Accessible file
            S_ISREG(st.st_mode) && // Regular file
            access(file_name, R_OK) == 0 && // Readable file 
            st.st_size <= MAX_FILE_SIZE) ? // Size limit
            VALID : INVALID;
}

/**
 * @brief Sends TCP request to create a new event and handles the response.
 * 
 * PROTOCOL: CRE <uid> <password> <name> <event_date> <attendance_size> <Fname> <Fsize> <Fdata>
 * 
 * @param args [event_name event_file_name event_date num_seats]
 * @return ReplyStatus 
 */
ReplyStatus create_event_handler(char* args) {
    char event_name[MAX_EVENT_NAME + 1];
    char file_name[24 + 1]; // TODO: maximum file name length?
    char date[EVENT_DATE_LENGTH + 1];
    char num_seats[4];

    if (!verify_argument_count(args, 4)) return STATUS_INVALID_ARGS;

    // name event_fname event_date, num_attendees
    // TODO: corrigir tamanho do scanf
    sscanf(args,"%10s %25s %33s %s", event_name, file_name, date, num_seats);
    if (!verify_event_name_format(event_name) || !verify_file(file_name) ||
        !verify_event_date_format(date) || !verify_seat_count(num_seats))
        return STATUS_INVALID_ARGS;
    
    int tcp_fd = connect_tcp(IP, PORT);
    if (tcp_fd == -1) return STATUS_SEND_FAILED;

    // PROTOCOL: CRE <uid> <password> <name> <event_date> <attendance_size> <Fname> <Fsize> <Fdata>
    // Get file size
    struct stat st;
    stat(file_name, &st);
    long file_size = st.st_size;

    // Prepare request header
    char request_header[512];
    snprintf(request_header, sizeof(request_header), "CRE %s %s %s %s %s %s %ld",
             current_uid, current_password, event_name, date, num_seats, file_name, file_size);
    
    // Send request header
    if (send_tcp_message(tcp_fd, request_header) == ERROR) {
        close(tcp_fd);
        return STATUS_SEND_FAILED;
    }

    // Send file
    if (send_tcp_file(tcp_fd, file_name) == ERROR) {
        close(tcp_fd);
        return STATUS_SEND_FAILED;
    }   
    return STATUS_UNASSIGNED; // TODO: handle response
}

void command_handler(CommandType command, char* args, int udp_fd,
     struct sockaddr_in* server_udp_addr) {
    
    socklen_t udp_addr_len = sizeof(*server_udp_addr);
    ReplyStatus status = STATUS_UNASSIGNED;
    
    switch (command) {
        case LOGIN:
            status = login_handler(args, udp_fd, server_udp_addr, udp_addr_len);
            break;
        case CHANGEPASS:
            // Handle change password
            break;
        case UNREGISTER:
            status = unregister_handler(args, udp_fd, server_udp_addr, udp_addr_len);
            break;
        case LOGOUT:
            status = logout_handler(args, udp_fd, server_udp_addr, udp_addr_len);
            break;
        case EXIT:
            if (is_logged_in) {
                status = logout_handler(args, udp_fd, server_udp_addr, udp_addr_len);
                print_result(LOGOUT, status);
            }
            printf("Exiting application.\n");
            close(udp_fd);
            exit(0);
            break;
        case CREATE:
            // Handle create event
            status = create_event_handler(args);
            break;
        case CLOSE:
            // Handle close event
            break;
        case MYEVENTS:
            status = myevent_handler(args, udp_fd, server_udp_addr, udp_addr_len);
            break;
        case LIST:
            // Handle list events
            break;
        case SHOW:
            // Handle show event details
            break;
        case RESERVE:
            // Handle reserve event
            break;
        case MYRESERVATIONS:
            // Handle my reservations
            break;
        default:
            printf("Unknown command\n");
            break;
    }
    if(status != STATUS_UNASSIGNED) print_result(command, status);
}