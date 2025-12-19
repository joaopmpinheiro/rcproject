#include "../../include/utils.h"
#include "../../include/globals.h"
#include "../../common/verifications.h"
#include "../../common/common.h"
#include "../../common/parser.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int verify_uid_password(Request* req) {
    if(!verify_argument_count(req->buffer, 3)) return INVALID;

    char UID[UID_LENGTH + 1];
    char command[COMMAND_LENGTH + 1];
    char password[PASSWORD_LENGTH + 1];

    sscanf(req->buffer, "%3s %6s %8s", command, UID, password);

    if (!verify_uid_format(UID)) return INVALID;
    if (!verify_password_format(password)) return INVALID;
    
    return VALID;
}

void handle_udp_request(Request* req) {
    char *cursor = req->buffer;
    
    // Get 3-letter command
    char command_buff[COMMAND_LENGTH + 1];
    if(get_next_arg(&cursor, command_buff) == ERROR){
        send_udp_response("ERR\n", req);
        return;
    }

    RequestType command = identify_command_request(command_buff);

    // If command is UNKNOWN, send ERR response
    if(command == UNKNOWN){
        send_udp_response("ERR\n", req);
        return;
    }

    // Get next arguments: UID and password
    char uid[UID_LENGTH + 1];
    char password[PASSWORD_LENGTH + 1];
    if(get_next_arg(&cursor, uid) == ERROR ||
       get_next_arg(&cursor, password) == ERROR ||
       is_end_of_message(&cursor) == ERROR ||
       !verify_uid_format(uid) ||
       !verify_password_format(password)) {
        char response[16]; 
        snprintf(response, sizeof(response), "%s ERR\n", command_buff);
        send_udp_response(response, req);
        return;
    }

    if(set.verbose) printf("Handling %s (%s) command, from UID: %s, using port %s\n",
                            command_to_str(command), command_buff, uid, set.port);

    switch (command) {
        case LOGIN:
            login_handler(req, uid, password);
            break;
        case LOGOUT:
            logout_handler(req, uid, password);
            break;
        case UNREGISTER:
            unregister_handler(req, uid, password);
            break;
        case MYEVENTS:
           myevents_handler(req, uid, password);
            break;
        case MYRESERVATIONS:
            myreservations_handler(req, uid, password);
            break;
        default:
            send_udp_response("ERR\n", req);
            break;
    }
}

void handle_tcp_request(Request* req) {
    char command_buff[COMMAND_LENGTH + 1];

    memcpy(command_buff, req->buffer, COMMAND_LENGTH);
    command_buff[COMMAND_LENGTH] = '\0';

    RequestType command = identify_command_request(command_buff);

    switch (command) {
        case CREATE:
            create_event_handler(req);
            break;
        case CLOSE:
            close_event_handler(req);
            break;
        case LIST:
            list_events_handler(req);
            break;
        case SHOW:
            show_event_handler(req);
            break;
        case RESERVE:
            reserve_seats_handler(req);
            break;
        case CHANGEPASS:
            change_password_handler(req);
            break;
        default:
            tcp_write(req->client_socket, "ERR\n", strlen("ERR\n"));
            break;
    }
}

// ------------ UDP Requests ---------------
void login_handler(Request* req, char* UID, char* password) {
    sscanf(req->buffer, "LIN %s %s", UID, password);

    if (!user_exists(UID)) {
        if (create_new_user(UID, password) == ERROR) {
            send_udp_response("RLI ERR\n", req);
            fprintf(stderr, "Error creating new user with UID %s\n", UID);
            return;
        }
        send_udp_response("RLI REG\n", req);
        return;
    }
    
    int status = verify_correct_password(UID, password);
    // Error verifying password
    if (status == ERROR) {
        send_udp_response("RLI ERR\n", req);
        fprintf(stderr, "Error verifying password for user with UID %s\n", UID);
        return;
    }
    // Incorrect password
    if (status == INVALID) {
        send_udp_response("RLI NOK\n", req);
        return;
    }

    write_login(UID);
    send_udp_response("RLI OK\n", req);
}


void logout_handler(Request* req, char* UID, char* password) {

    // User not registered
    if (!user_exists(UID)) {
        send_udp_response("RLO UNR\n", req);
        return;
    }

    // User not logged in
    if (!is_logged_in(UID)) {
        send_udp_response("RLO NOK\n", req);
        return;
    }

    int status = verify_correct_password(UID, password);

    // Error verifying password
    if (status == ERROR) {
        send_udp_response("RLO ERR\n", req);
        fprintf(stderr, "Error verifying password for user with UID %s\n", UID);
        return;
    }

    // Incorrect password
    if (status == INVALID) {
        send_udp_response("RLO WRP\n", req);
        return;
    }

    erase_login(UID);
    send_udp_response("RLO OK\n", req);

}

/**
 * @brief Handles unregister request: UNR UID password
 * 
 * Sends to user:
 * RUR OK - successful unregistration,
 * RUR UNR - user was not registered
 * RUR NOK - user not logged in
 * RUR WRP - wrong password
 * 
 * @param req 
 * @param UID 
 * @param password 
 */
void unregister_handler(Request* req, char* UID, char* password) {
    if(!user_exists(UID)) {
        send_udp_response("RUR UNR\n", req);
        return;
    }
    if(!is_logged_in(UID)) {
        send_udp_response("RUR NOK\n", req);
        return;
    }
    int status = verify_correct_password(UID, password);
    if(status == ERROR) {
        send_udp_response("RUR ERR\n", req);
        fprintf(stderr, "Error verifying password for user with UID %s\n", UID);
        return;
    }
    if(status == INVALID) {
        send_udp_response("RUR WRP\n", req);
        return;
    }

    // Proceed to unregister user
    if(remove_user(UID) == ERROR) {
        send_udp_response("RUR ERR\n", req);
        fprintf(stderr, "Error deleting user with UID %s\n", UID);
        return;
    }
    send_udp_response("RUR OK\n", req);
}


/**
 * @brief Handles myevents request: MYE UID password
 * 
 * * Sends to user:
 * RME OK [EID1 state>]*
 * RME NOK - user has no events
 * RME NLG - user not logged in
 * RME WRP - wrong password
 * 
 * @param req 
 * @param UID 
 * @param password 
 */
void myevents_handler(Request* req, char* UID, char* password){
    if(!user_exists(UID)) {
        send_udp_response("RME ERR\n", req);
        return;
    }
    if(!is_logged_in(UID)) {
        send_udp_response("RME NLG\n", req);
        return;
    }
    int status = verify_correct_password(UID, password);
    if(status == ERROR) {
        send_udp_response("RME ERR\n", req);
        fprintf(stderr, "Error verifying password for user with UID %s\n", UID);
        return;
    }

    if(status == INVALID) {
        send_udp_response("RME WRP\n", req);
        return;
    }

    if(has_events(UID) == FALSE) {
        send_udp_response("RME NOK\n", req);
        return;
    }


    char response[999 * 6 + 10]; // Max 999 events, each with "EID state " (6 chars) + null terminator
    if(format_list_of_user_events(UID, response, sizeof(response)) == ERROR) {
        send_udp_response("RME ERR\n", req);
        fprintf(stderr, "Error formatting list of events for user with UID %s\n", UID);
        return;
    }
    send_udp_response(response, req);
}   

/* 
states:
1 - accepting reservations
0 - in the past
2 - in the future but sold out
3 - closed
*/

int format_list_of_user_events(char* UID, char* message, size_t message_size) {
    char path[32];
    snprintf(path, sizeof(path), "USERS/%s/CREATED", UID);

    struct dirent **namelist;
    int n = scandir(path, &namelist, NULL, alphasort);
    if (n < 0) {
        perror("scandir");
        return ERROR;
    }

    snprintf(message, message_size, "RME OK");

    for (int i = 0; i < n; i++) {
        struct dirent *entry = namelist[i];
        // Skip non-event files
        if (verify_event_file(entry->d_name) == INVALID) {
            free(entry);
            continue;
        }

        char event_EID[4];
        strncpy(event_EID, entry->d_name, 3);
        event_EID[3] = '\0';

        int state;
        if (is_event_closed(event_EID)) state = CLOSED;
        else if (is_event_past(event_EID)) state = PAST;
        else if (is_event_sold_out(event_EID)) state = SOLD_OUT;
        else state = ACCEPTING;

        char temp[16];
        snprintf(temp, sizeof(temp), " %s %c", event_EID, state);
        strncat(message, temp, message_size - strlen(message) - 1);

        free(entry); // free each dirent allocated by scandir
    }
    free(namelist); // free the array itself
    strncat(message, "\n", message_size - strlen(message) - 1);
    return SUCCESS;
}

/*
- list
- user does not have reservations [sends a maximum of 50 reservations - the most recent]
*/

/**
 * @brief Handles myreservations request: LMR UID password
 * 
 * Sends to user:
 * RMR status - [EID date value]*
 * RMR NOK - user has no reservations
 * RMR NLG - user not logged in
 * RMR WRP - wrong password
 * 
 * @param req 
 * @param UID 
 * @param password 
 */
void myreservations_handler(Request* req, char* UID, char* password){
    if(!user_exists(UID)) {
        send_udp_response("RMR ERR\n", req);
        return;
    }
    if(!is_logged_in(UID)) {
        send_udp_response("RMR NLG\n", req);
        return;
    }
    int status = verify_correct_password(UID, password);
    if(status == ERROR) {
        send_udp_response("RMR ERR\n", req);
        fprintf(stderr, "Error verifying password for user with UID %s\n", UID);
        return;
    }
    if(status == INVALID) {
        send_udp_response("RMR WRP\n", req);
        return;
    }
    if(has_reservations(UID) == INVALID) {
        send_udp_response("RMR NOK\n", req);
        return;
    }

    char response[4096];
    int err = format_list_of_user_reservations(UID, response, sizeof(response));
    if (err <= 0) {
        send_udp_response("RMR ERR\n", req);
        fprintf(stderr, "Error formatting list of reservations for user with UID %s\n", UID);
        return;
    }

    fprintf(stderr, "Formatted reservations response: %s", response);
    send_udp_response(response, req);
} 






// ------------- TCP -------------
/* USER: uccessful password change, unknown user, user not logged In or incorrect password. 
*/    
// Helper function to read a field or send a error and close connection in create_event_handler
static int read_field_or_error(int fd, char* dst, size_t len, char* code) {
    char response[16] = {0};
    if (tcp_read_field(fd, dst, len) == ERROR) {
        snprintf(response, sizeof(response), "%s ERR\n", code);
        tcp_write(fd, response, strlen(response));
        return ERROR;
    }
    return SUCCESS;
}

// Helper function to consume remaining file content from socket
static void consume_file(int fd, size_t file_size) {
    char buffer[4096];
    size_t total_read = 0;
    while (total_read < file_size) {
        size_t to_read = (file_size - total_read) < sizeof(buffer) ? 
                       (file_size - total_read) : sizeof(buffer);
        ssize_t n = read(fd, buffer, to_read);
        if (n <= 0) break;
        total_read += n;
    }
}



/**
 * @brief Handles change password request: CPS UID oldPassword newPassword 
 * 
 * Sends to user:
 * RCP OK - successful password change,
 * RCP NLG - user not logged in
 * RCP NOK - wrong password
 * RCP NID - unknown user
 * 
 * @param req 
 */
void change_password_handler(Request* req){
    char UID[UID_LENGTH + 1];
    char old_password[PASSWORD_LENGTH + 1];
    char new_password[PASSWORD_LENGTH + 1];
    int status;

    status = read_field_or_error(req->client_socket, UID, UID_LENGTH, "RCP");
    if (status != SUCCESS) return;
    status = read_field_or_error(req->client_socket, old_password, PASSWORD_LENGTH, "RCP");
    if (status != SUCCESS) return;
    status = read_field_or_error(req->client_socket, new_password, PASSWORD_LENGTH, "RCP");
    if (status != SUCCESS) return;

    // FIXME isto é burro e podia ser chamado no command handler
    char log[BUFFER_SIZE];
    snprintf(log, sizeof(log),
     "Handling change password (RCP), for user with UID %s, using port %s",
     UID, set.port);
    server_log(log);
    
    if(!user_exists(UID)) {
        tcp_write(req->client_socket, "RCP NID\n", strlen("RCP NID\n"));
        return;
    }
    if(!is_logged_in(UID)) {
        tcp_write(req->client_socket, "RCP NLG\n", strlen("RCP NLG\n"));
        return;
    }
    status = verify_correct_password(UID, old_password);
    if(status == ERROR) {
        tcp_write(req->client_socket, "RCP ERR\n", strlen("RCP ERR\n"));
        fprintf(stderr, "Error verifying password for user with UID %s\n", UID);
        return;
    }
    if(status == INVALID) {
        tcp_write(req->client_socket, "RCP NOK\n", strlen("RCP NOK\n"));
        return;
    }

    // Proceed to change password
    if(write_password(UID, new_password) == ERROR) {
        tcp_write(req->client_socket, "RCP ERR\n", strlen("RCP ERR\n"));
        fprintf(stderr, "Error changing password for user with UID %s\n", UID);
        return;
    }
    tcp_write(req->client_socket, "RCP OK\n", strlen("RCP OK\n"));
}


/*
input: 
    name: string with event name
    event_fname: file
    event_date: date and time 
    num_attendees: interger
USER: successful or not and EID
*/
void create_event_handler(Request* req){
    char UID[UID_LENGTH + 1];
    char password[PASSWORD_LENGTH + 1];
    char event_name[MAX_EVENT_NAME + 1];
    char event_date[EVENT_DATE_LENGTH + 1];
    char seat_count[SEAT_COUNT_LENGTH + 1]; // max 999, so 3 digits + null
    char EID[EID_LENGTH + 1];

    char file_name[FILE_NAME_LENGTH + 1];
    char file_size_str[FILE_SIZE_LENGTH + 1]; // max 8 digits for file size (10MB = 10000000)
    size_t file_size;
    char* file_content = NULL;

    char protocol[4] = "RCE";

    int fd = req->client_socket;

    // PROTOCOL: CRE <uid> <password> <event_name> <event_date> <seat_count> 
    // <file_name> <file_size> <file_content>
    // TODO VER CASO DO EOM AQUI
    int field_status;
    field_status = read_field_or_error(fd, UID, UID_LENGTH, protocol);
    if (field_status == ERROR || field_status == EOM) return;

    field_status = read_field_or_error(fd, password, PASSWORD_LENGTH, protocol);
    if (field_status == ERROR || field_status == EOM) return;

    field_status = read_field_or_error(fd, event_name, MAX_EVENT_NAME, protocol);
    if (field_status == ERROR || field_status == EOM) return;

    // Read event_date (16 chars: DD-MM-YYYY HH:MM)
    // Date has a space in it, so we need to read date and time separately
    char date_part[11]; // DD-MM-YYYY
    char time_part[6];  // HH:MM
    field_status = read_field_or_error(fd, date_part, 10, protocol);
    if (field_status == ERROR || field_status == EOM) return;

    field_status = read_field_or_error(fd, time_part, 5, protocol);
    if (field_status == ERROR || field_status == EOM) return;

    snprintf(event_date, EVENT_DATE_LENGTH + 1, "%s %s", date_part, time_part);

    // Read seat_count (max 3 digits)
    field_status = read_field_or_error(fd, seat_count, 3, protocol);
    if (field_status == ERROR || field_status == EOM) return;

    field_status = read_field_or_error(fd, file_name, FILE_NAME_LENGTH, protocol);
    if (field_status == ERROR || field_status == EOM) return;

    field_status = read_field_or_error(fd, file_size_str, FILE_SIZE_LENGTH, protocol);
    if (field_status == ERROR || field_status == EOM) return;

    if (!verify_file_size(file_size_str)) {
        tcp_write(fd, "RCE ERR\n", 8);
        file_size = (size_t)atol(file_size_str);
        consume_file(fd, file_size);
        return;
    }

    char log[BUFFER_SIZE];
    snprintf(log, sizeof(log),
     "Handling create event (CRE), from user with UID %s, using port %s", UID, set.port);
    server_log(log);
    
    // Validate all fields
    if (!verify_uid_format(UID) ||
        !verify_password_format(password) ||
        !verify_event_name_format(event_name) ||
        !verify_event_date_format(event_date) ||
        !verify_seat_count(seat_count) ||
        !verify_file_name_format(file_name)) {
        printf("Field validation failed\n");
        tcp_write(fd, "RCE ERR\n", 8);
        file_size = (size_t)atol(file_size_str);
        consume_file(fd, file_size);
        return;
    }

    if (!is_logged_in(UID)) {
        tcp_write(fd, "RCE NLG\n", 8);
        file_size = (size_t)atol(file_size_str);
        consume_file(fd, file_size);
        return;
    }

    if (!verify_correct_password(UID, password)) {
        tcp_write(fd, "RCE WRP\n", 8);
        file_size = (size_t)atol(file_size_str);
        consume_file(fd, file_size);
        return;
    }
  

    // Allocate buffer for file content
    file_size = (size_t)atol(file_size_str);
    file_content = (char*)malloc(file_size + 1);
    if (file_content == NULL) {
        tcp_write(fd, "RCE NOK\n", 8);
        return;
    }

    // Read file content (exactly file_size bytes)
    size_t total_read = 0;
    while (total_read < file_size) {
        ssize_t n = read(fd, file_content + total_read, file_size - total_read);
        if (n <= 0) {
            free(file_content);
            tcp_write(fd, "RCE ERR\n", 8);
            return;
        }
        total_read += n;
    }
    file_content[file_size] = '\0';


    if (find_available_eid(EID) == ERROR) {
        tcp_write(fd, "RCE NOK\n", 8);
        return;
    }

    if (create_eid_dir(atoi(EID)) == ERROR) {
        tcp_write(fd, "RCE NOK\n", 8);
        return;
    }

    if (write_event_start_file(EID, UID, event_name, file_name, seat_count,
                               event_date) == ERROR) {
        tcp_write(fd, "RCE NOK\n", 8);
        return;
    }

    if (write_event_information_file(EID, UID, event_name, file_name, seat_count,
                               event_date) == ERROR) {
        tcp_write(fd, "RCE NOK\n", 8);
        return;
    }

    if (update_reservations_file(EID, 0) == ERROR) {
        tcp_write(fd, "RCE NOK\n", 8);
        return;
    }

    if (write_description_file(EID, file_name, file_size, file_content) == ERROR) {
        tcp_write(fd, "RCE NOK\n", 8);
        return;
    }
    
    free(file_content);

    // Send success response with EID
    char response[16];
    snprintf(response, sizeof(response), "RCE OK %s\n", EID);
    tcp_write(fd, response, strlen(response));
}

/*
input:
    eid: integer
    user: nao sei se vem no log in mas verificar que é o user que criou o evento
Before closing, check the status of the event.
USER:
- no event: wrong user or non-existing EID
- event already expired: if the date has passed
- event is sold out: if the event is fully reserved
- sucessful event closure
*/
void close_event_handler(Request* req){
    char UID[UID_LENGTH + 1];
    char password[PASSWORD_LENGTH + 1];
    char EID[EID_LENGTH + 1];

    int fd = req->client_socket;

    char protocol[4] = "RCL";

    // PROTOCOL: CLS <uid> <password> <eid>
    int status = read_field_or_error(fd, UID, UID_LENGTH, protocol);
    if (status == ERROR || status == EOM) return;

    status = read_field_or_error(fd, password, PASSWORD_LENGTH, protocol);
    if (status == ERROR || status == EOM) return;

    status = read_field_or_error(fd, EID, MAX_EVENT_NAME, protocol);
    if (status == ERROR) return;

    // FIXME TODO MUDAR PORT PARA IP
    char log[BUFFER_SIZE];
    snprintf(log, sizeof(log),
     "Handling close event (CLS), from user with UID %s, using port %s", UID, set.port);
    server_log(log);

    // Validate all fields
    if (!verify_uid_format(UID) ||
        !verify_password_format(password) ||
        !verify_eid_format(EID)) {
        tcp_write(fd, "RCE ERR\n", 8);
        return;
    }

    if (!is_logged_in(UID)) {
        tcp_write(fd, "RCL NLG\n", 8);
        return;
    }

    if (!verify_correct_password(UID, password) || !user_exists(UID)) {
        tcp_write(fd, "RCL NOK\n", 8);
        return;
    }

    if (!event_exists(EID)) {
        tcp_write(fd, "RCL NOE\n", 8);
        return;
    }

    if (!is_event_creator(UID, EID)) {
        tcp_write(fd, "RCL EOW\n", 8);
        return;
    }

    if (is_event_sold_out(EID)) {
        tcp_write(fd, "RCL SLD\n", 8);
        return;
    }

    if (is_event_closed(EID)) {
        fprintf(stderr, "Event %s is already closed.\n", EID);
        tcp_write(fd, "RCL CLO\n", 8);
        return;
    }

    if (is_event_past(EID)) {
        tcp_write(fd, "RCL PST\n", 8);
        return;
    }

    if (write_event_end_file(EID) == ERROR) {
        tcp_write(fd, "RCL ERR\n", 8);
        return;
    }

    tcp_write(fd, "RCL OK\n", 7); 
}

/**
 * @brief Handles list events request: LST
 * 
 * Sends to user:
 * RLS OK [EID name state event_date]*
 * RLS NOK - no events available
 * 
 * @param req 
 */
void list_events_handler(Request* req){
    int fd = req->client_socket;
    
    // FIXME TODO MUDAR PORT PARA IP
    char log[BUFFER_SIZE];
    snprintf(log, sizeof(log),
     "Handling list event (LST), using port %s", set.port);
    server_log(log);

    if (is_dir_empty("EVENTS")) {
        tcp_write(fd, "RLS NOK\n", 7);   
        return;
    }
    
    // Send initial OK response
    tcp_write(fd, "RLS OK ", 7);

    char event_EID[EID_LENGTH + 1];
    char event_name[MAX_EVENT_NAME + 1];
    char event_date[EVENT_DATE_LENGTH + 1];
    int state = ' ';

    // Loop from 001 to 999
    for (int eid = 1; eid <= 999; eid++) {
        snprintf(event_EID, EID_LENGTH + 1, "%03d", eid);
        
        // Check if event exists
        if (!event_exists(event_EID)) continue;

        // Read event details
        if (get_list_event_info(event_EID, event_name, event_date) == ERROR) continue;

        // Determine event state
        if (is_event_closed(event_EID)) state = CLOSED;
        if (is_event_past(event_EID)) state = PAST;
        else if (is_event_sold_out(event_EID)) state = SOLD_OUT;
        else state = ACCEPTING;

        // Append event details to response
        // PROTOCOLO: <EID name state event_date>
        char event_entry[256];
        snprintf(event_entry, sizeof(event_entry), "%s %s %c %s ",
                 event_EID, event_name, state, event_date);

        tcp_write(fd, event_entry, strlen(event_entry));
    }

    tcp_write(fd, "\n", 1);
    
}

/**
 * @brief Handles show event request: SHO EID
 * 
 * Sends to user:
 * RSE OK [UID name event_date attendance_size Seats_reserved Fname Fsize Fdata]
 * RSE NOK - event does not exist or other problem
 * 
 * @param req 
 */
void show_event_handler(Request* req){
    char EID[EID_LENGTH + 1];

    int fd = req->client_socket;
    char protocol[4] = "RSE";

    int status = read_field_or_error(fd, EID, EID_LENGTH, protocol);
    if (status == ERROR) return;

    char log[BUFFER_SIZE];
    snprintf(log, sizeof(log),
     "Handling show event (SHO), for EID %s, using port %s", EID, set.port);
    server_log(log);

    // Validate EID
    if (!verify_eid_format(EID)) {
        tcp_write(fd, "RSE NOK\n", 8);
        return;
    }

    if (!event_exists(EID)) {
        tcp_write(fd, "RSE NOK\n", 8);
        return;
    }

    char response[BUFFER_SIZE];
    char file_name[FILE_NAME_LENGTH + 1];
    long file_size;
    if (format_event_details(EID, response, sizeof(response), file_name, &file_size) == ERROR) {
        tcp_write(fd, "RSE NOK\n", 8);
        return;
    }

    char description_path[128];
    snprintf(description_path, sizeof(description_path), "EVENTS/%s/DESCRIPTION/%s", EID, file_name);
    tcp_write(fd, response, strlen(response));
    tcp_send_file(fd, description_path);
}

int format_event_details(char* EID, char* message, size_t message_size, char* file_name, long* file_size) {
    char UID[UID_LENGTH + 1];
    char event_name[MAX_EVENT_NAME + 1];
    char event_date[EVENT_DATE_LENGTH + 1];
    char total_seats[SEAT_COUNT_LENGTH + 1];
    char reserved_seats[SEAT_COUNT_LENGTH + 1];

    if (read_event_full_details(EID, UID, event_name,
                            event_date, total_seats,
                            reserved_seats, file_name) == ERROR)
        return ERROR;
    
    if (!verify_eid_format(EID) ||
       !verify_uid_format(UID) ||
       !verify_event_name_format(event_name) ||
       !verify_event_date_format(event_date) ||
       !verify_seat_count(total_seats) ||
       !verify_reserved_seats(reserved_seats, total_seats) ||
       !verify_file_name_format(file_name)) 
        return ERROR;
    
    // Check if description file exists
    char description_path[128];
    snprintf(description_path, sizeof(description_path), "EVENTS/%s/DESCRIPTION/%s", EID, file_name);
    if(!file_exists(description_path)) return ERROR;
    
    struct stat st;
    if (stat(description_path, &st) != 0) return ERROR;
    *file_size = st.st_size;
    
    snprintf(message, message_size,
             "RSE OK %s %s %s %s %s %s %ld ",
             UID, event_name, event_date,
             total_seats, reserved_seats,
             file_name, *file_size);
    return SUCCESS;
}


/*
input:
    eid: integer
    num_seats: integer
USER: s
- accepted
- refused + num of available seats (if num_seats > available seats)
- no longer active
*/

/**
 * @brief Handles reserve seats request: RES UID password EID num_seats
 * 
 * Sends to user:
 * RRI ACC - reservation successful,
 * RRI NOK - event not active
 * RRI NLG - user not logged in
 * RRI WRP - wrong password
 * RRI CLS - event closed
 * RRI SLD - event sold out
 * RRI REJ n_seats- not enough available seats
 * RRI PST - event date has passed
 * 
 * @param req 
 */
void reserve_seats_handler(Request* req){
    char UID[UID_LENGTH + 1];
    char password[PASSWORD_LENGTH + 1];
    char EID[EID_LENGTH + 1];
    char seat_count[SEAT_COUNT_LENGTH + 1]; // max 3 digits

    int fd = req->client_socket;

    char protocol[4] = "RRI";

    // PROTOCOL: RES <uid> <password> <eid> <num_seats>
    if(read_field_or_error(fd, UID, UID_LENGTH, protocol) != SUCCESS ||
       read_field_or_error(fd, password, PASSWORD_LENGTH, protocol) != SUCCESS ||
       read_field_or_error(fd, EID, EID_LENGTH, protocol) != SUCCESS ||
       read_field_or_error(fd, seat_count, SEAT_COUNT_LENGTH, protocol) != SUCCESS) return;
    

    // FIXME isto é burro e podia ser chamado no command handler
    char log[BUFFER_SIZE];
    snprintf(log, sizeof(log),
     "Handling reserve seats (RID), from user with UID %s, using port %s",
     UID, set.port);
    server_log(log);

    // Validate all fields
    if (!verify_uid_format(UID) ||
        !verify_password_format(password) ||
        !verify_eid_format(EID) ||
        !verify_reserved_seats(seat_count, "999")) {
        tcp_write(fd, "RRI ERR\n", 8);
        return;
    }

    if (!is_logged_in(UID)) {
        tcp_write(fd, "RRI NLG\n", 8);
        return;
    }

    if (!verify_correct_password(UID, password) || !user_exists(UID)) {
        tcp_write(fd, "RRI WRP\n", 8);
        return;
    }

    if (!event_exists(EID)) {
        tcp_write(fd, "RRI NOK\n", 8);
        return;
    }

    if (is_event_closed(EID)) {
        tcp_write(fd, "RRI CLS\n", 8);
        return;
    }

    if (is_event_sold_out(EID)) {
        tcp_write(fd, "RRI SLD\n", 8);
        return;
    }

    if (is_event_past(EID)) {
        tcp_write(fd, "RRI PST\n", 8);
        return;
    }
    int available_seats = get_available_seats(EID);
    if(available_seats == ERROR) {
        tcp_write(fd, "RRI ERR\n", 8);
        return;
    }

    int requested_seats = atoi(seat_count);
    if (requested_seats > available_seats) {
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "RRI REJ %d\n", available_seats);
        tcp_write(fd, response, strlen(response));
        return;
    }

    if (update_reservations_file(EID, requested_seats) == ERROR) {
        tcp_write(fd, "RRI ERR\n", 8);
        return;
    }

    // Create reservation record files
    if (make_reservation(UID, EID, requested_seats) == ERROR) {
        tcp_write(fd, "RRI ERR\n", 8);
        return;
    }

    tcp_write(fd, "RRI ACC\n", 8);
}

    