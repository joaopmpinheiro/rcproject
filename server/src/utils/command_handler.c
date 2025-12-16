#include "../../include/utils.h"
#include "../../include/globals.h"
#include "../../common/verifications.h"
#include "../../common/common.h"

RequestType identify_request_type(char* command_buff){
    if (strncmp(command_buff, "LIN", 3) == 0) return LOGIN;
    if (strncmp(command_buff, "LOU", 3) == 0) return LOGOUT;
    if (strncmp(command_buff, "UNR", 3) == 0) return UNREGISTER;
    if (strncmp(command_buff, "CPS", 3) == 0) return CHANGEPASS;
    if (strncmp(command_buff, "CRE", 3) == 0) return CREATE;
    if (strncmp(command_buff, "CLS", 3) == 0) return CLOSE;
    if (strncmp(command_buff, "LME", 3) == 0) return MYEVENTS;
    if (strncmp(command_buff, "LST", 3) == 0) return LIST;
    if (strncmp(command_buff, "SED", 3) == 0) return SHOW;
    if (strncmp(command_buff, "RID", 3) == 0) return RESERVE;
    if (strncmp(command_buff, "LMR", 3) == 0) return MYRESERVATIONS;
    else return UNKNOWN;
}

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
    char command_buff[3] = {0};
    char command_arg_buffer[BUFFER_SIZE] = {0};

    // Get 3-letter command
    sscanf(req->buffer, "%s %[^\n]", command_buff, command_arg_buffer);
    RequestType command = identify_request_type(command_buff);

    // known command but wrong arguments
    if (command != UNKNOWN && verify_uid_password(req) == INVALID) {
        // build error response
        char response[16]; 
        snprintf(response, sizeof(response), "%s ERR\n", command_buff);
        send_udp_response(response, req);
        return;
    }

    switch (command) {
        case LOGIN:
            login_handler(req);
            break;
        default:
            send_udp_response("ERR\n", req);
            break;
    }
}

void handle_tcp_request(Request* req) {
    char command_buff[COMMAND_LENGTH + 1] = {0};

    strncpy(command_buff, req->buffer, COMMAND_LENGTH);
    RequestType command = identify_request_type(command_buff);

    switch (command) {
        case CREATE:
            create_event_handler(req);
            break;
        case CLOSE:
            // close_event_handler();
            break;
        case LIST:
            // list_events_handler();
            break;
        case SHOW:
            // show_events_handler();
            break;
        case RESERVE:
            // reserve_seats_handler();
            break;
        case CHANGEPASS:
            // change_password_handler();
            break;
        default:
            tcp_write(req->client_socket, "ERR\n", strlen("ERR\n"));
            break;
    }
}



// ------------ UDP Requests ---------------
void login_handler(Request* req) {
    char UID[UID_LENGTH + 1];
    char password[PASSWORD_LENGTH + 1];
    char user_password[PASSWORD_LENGTH + 1];

    sscanf(req->buffer, "LIN %s %s", UID, password);

    if(set.verbose){
        printf("Handling login (LIN), from user with UID %s, using port %s\n",UID, set.port);
    }

    if (!user_exists(UID)) {
        if (create_new_user(UID, password) == ERROR) {
            send_udp_response("RLI ERR\n", req);
            fprintf(stderr, "Error creating new user with UID %s\n", UID);
            return;
        }
        send_udp_response("RLI REG\n", req);
        return;
    }

    get_password(UID, user_password);
    if (strcmp(password, user_password) == 0) {
        write_login(UID);
        send_udp_response("RLI OK\n", req);
    }

    else send_udp_response("RLI NOK\n", req);
}

/** LOU UID password
 * RLO OK - successful logout
 * RLO NOK - user not logged in
 * RLO UNK - user was not registered
**/
void logout_handler(){

}

/* UNR UID password
RUR OK - successful unregister
RUR NOK - not logged
RUR UNK - not reggistered
USER: successful unregister, unknown user, or incorrect unregister attempt
*/
void unregister_handler(){

}


/** UNR UID password

 * USER: list of events created by the user or no events created yet.
**/
void check_user_events_handler(){

}

/*
- list
- user does not have reservations [sends a maximum of 50 reservations - the most recent]
*/
void check_user_reservations_handler(){

}






// ------------- TCP -------------
/* USER: uccessful password change, unknown user, user not logged In or incorrect password. 
*/    
void change_password_handler(){

}

// Helper function to read a field or send a error and close connection in create_event_handler
static int read_field_or_error(int fd, char* dst, size_t len, char* code) {
    char response[16] = {0};
    if (tcp_read_field(fd, dst, len) == ERROR) {
        snprintf(response, sizeof(response), "%s ERR\n", code);
        tcp_write(fd, response, strlen(response));
        close(fd);
        return ERROR;
    }
    return SUCCESS;
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
        close(fd);
        return;
    }

    char log[BUFFER_SIZE];
    snprintf(log, sizeof(log),
     "Handling create event (CRE), from user with UID %s, using port %s\n", UID, set.port);
    server_log(log);
    
    // Validate all fields
    if (!verify_uid_format(UID) ||
        !verify_password_format(password) ||
        !verify_event_name_format(event_name) ||
        !verify_event_date_format(event_date) ||
        !verify_seat_count(seat_count) ||
        !verify_file_name_format(file_name)) {
        tcp_write(fd, "RCE ERR\n", 8);
        close(fd);
        return;
    }

    if (!is_logged_in(UID)) {
        tcp_write(fd, "RCE NLG\n", 8);
        close(fd);
        return;
    }

    if (!verify_correct_password(UID, password)) {
        tcp_write(fd, "RCE WRP\n", 8);
        close(fd);
        return;
    }

    // Allocate buffer for file content
    file_size = (size_t)atol(file_size_str);
    file_content = (char*)malloc(file_size + 1);
    if (file_content == NULL) {
        tcp_write(fd, "RCE NOK\n", 8);
        close(fd);
        return;
    }

    // Read file content (exactly file_size bytes)
    // TODO: abstrair
    size_t total_read = 0;
    while (total_read < file_size) {
        ssize_t n = read(fd, file_content + total_read, file_size - total_read);
        if (n <= 0) {
            free(file_content);
            tcp_write(fd, "RCE ERR\n", 8);
            close(fd);
            return;
        }
        total_read += n;
    }
    file_content[file_size] = '\0';


    // TODO: funçao de rollback, pensar nisso
    if (find_available_eid(EID) == ERROR) {
        tcp_write(fd, "RCE NOK\n", 8);
        close(fd);
        return;
    }

    if (create_eid_dir(atoi(EID)) == ERROR) {
        tcp_write(fd, "RCE NOK\n", 8);
        close(fd);
        return;
    }

    if (write_event_start_file(EID, UID, event_name, file_name, seat_count,
                               event_date) == ERROR) {
        tcp_write(fd, "RCE NOK\n", 8);
        close(fd);
        return;
    }

    // TODO pode ser preciso por o eid no ficheiro q isto faz tbm
    if (write_event_information_file(EID, UID, event_name, file_name, seat_count,
                               event_date) == ERROR) {
        tcp_write(fd, "RCE NOK\n", 8);
        close(fd);
        return;
    }

    if (update_reservations_file(EID, 0) == ERROR) {
        tcp_write(fd, "RCE NOK\n", 8);
        close(fd);
        return;
    }

    if (write_description_file(EID, file_name, file_size, file_content) == ERROR) {
        tcp_write(fd, "RCE NOK\n", 8);
        close(fd);
        return;
    }
    
    free(file_content);

    // Send success response with EID
    char response[16];
    snprintf(response, sizeof(response), "RCE OK %s\n", EID);
    tcp_write(fd, response, strlen(response));
    close(fd);
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
    // TODO VER CASO DO EOM AQUI
    int status = read_field_or_error(fd, UID, UID_LENGTH, protocol);
    if (status == ERROR || status == EOM) return;

    status = read_field_or_error(fd, password, PASSWORD_LENGTH, protocol);
    if (status == ERROR || status == EOM) return;

    status = read_field_or_error(fd, EID, MAX_EVENT_NAME, protocol);
    if (status == ERROR) return;

    // FIXME TODO MUDAR PORT PARA IP
    char log[BUFFER_SIZE];
    snprintf(log, sizeof(log),
     "Handling close event (CLS), from user with UID %s, using port %s\n", UID, set.port);
    server_log(log);

    // Validate all fields
    if (!verify_uid_format(UID) ||
        !verify_password_format(password) ||
        !verify_eid_format(EID)) {
        // TODO, maybe printar logs nestes casos (?)
        tcp_write(fd, "RCE ERR\n", 8);
        close(fd);
        return;
    }

    if (!is_logged_in(UID)) {
        tcp_write(fd, "RCL NLG\n", 8);
        close(fd);
        return;
    }

    if (!verify_correct_password(UID, password) || !user_exists(UID)) {
        tcp_write(fd, "RCL NOK\n", 8);
        close(fd);
        return;
    }

    if (!event_exists(EID)) {
        tcp_write(fd, "RCL NOE\n", 8);
        close(fd);
        return;
    }

    //    if (!is_event_creator(UID, EID)) {
    //        tcp_write(fd, "RCL EOW\n", 8);
    //        close(fd);
    //        return;
    //    }


//    if (is_event_sold_out(EID)) {
//        tcp_write(fd, "RCL SLD\n", 8);
//        close(fd);
//        return;
//    }

//    if (is_event_past(EID)) {
//        tcp_write(fd, "RCL PST\n", 8);
//        close(fd);
//        return;
//    }

//    if (is_event_closed(EID)) {
//        tcp_write(fd, "RCL CLO\n", 8);
//        close(fd);
//        return;
//    }

    if (write_event_end_file() == ERROR) {
        tcp_write(fd, "RCL NOK\n", 8);
        close(fd);
        return;
    }


}

/*
input: EID
USER: [events details]
- event_name
-total seats available, 
- num of seates reserved
- file
OR error message*/
void show_events_handler(){

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
void reserve_seats_handler(){

}

