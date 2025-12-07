#include "../../include/constants.h"
#include "../../include/utils.h"
#include "../../include/globals.h"
#include "../../common/verifications.h"

RequestType identify_request_type(char* command_buff){
    if (strncmp(command_buff, "LIN", 3) == 0) return LOGIN;
    else return UNKNOWN;
}

int verify_args_UID_password(Request* req) {
    if(!verify_argument_count(req->buffer, 3)) return INVALID;

    char UID[UID_LENGTH + 1];
    char command[COMMAND_LENGTH + 1];
    char password[PASSWORD_LENGTH + 1];

    sscanf(req->buffer, "%3s %6s %8s", command, UID, password);

    if (!verify_uid_format(UID)) return INVALID;
    if (!verify_password_format(password)) return INVALID;
    
    return VALID;
}

void handle_UDP_request(Request* req) {
    char command_buff[3] = {0};
    char command_arg_buffer[BUFFER_SIZE] = {0};

    // get 3-letter command
    sscanf(req->buffer, "%s %[^\n]", command_buff, command_arg_buffer);
    RequestType command = identify_request_type(command_buff);

    // known command but wrong arguments
    if (verify_args_UID_password(req) == INVALID && command != UNKNOWN) {

        // build error response
        char response[16]; 
        snprintf(response, sizeof(response), "%s ERR\n", command_buff);

        send_udp_response(response, req);
        return;
    }

    switch (command) {
        case LOGIN:
            // Handle login
            login_handler(req);
            break;
        default:
            send_udp_response("ERR\n", req);
            break;
    }
}

// ------------ Data Management --------------

// TODO: criar um users.c, um events.c e um reservations.c para gerir estas funções
// ou um data_manager.c
User* get_user_by_uid(int UID) {
    UserNode* current = users;
    while (current != NULL) {
        if (current->user.UID == UID) return &current->user;
        current = current->next;
    }
    return NULL;
}

void create_user(int UID, char* password) {
    UserNode* new_node = (UserNode*)malloc(sizeof(UserNode));
    new_node->user.UID = UID;
    strncpy(new_node->user.password, password, PASSWORD_LENGTH);
    new_node->user.status = 1; // logged in
    new_node->user.created_events = NULL;
    new_node->user.reserved_events = NULL;
    new_node->next = users;
    users = new_node;
}



// ------------ UDP Requests ---------------
void login_handler(Request* req) {
    int UID;
    char password[PASSWORD_LENGTH];
    sscanf(req->buffer, "LIN %d %s", &UID, password);

    if(set.verbose){
        printf("Handling login (LIN), from user with UID %d,\
                            using port %s\n",UID, set.port);
    }

    User* user = get_user_by_uid(UID);
    if (user == NULL) {
        create_user(UID, password);
        send_udp_response("RLI REG\n", req);
        return;
    }

    if (strcmp(user->password, password) == 0) {
        user->status = LOGGED_IN; // logged in
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


/*
input: 
    name: string with event name
    event_fname: file
    event_date: date and time 
    num_attendees: interger
USER: successful or not and EID
*/
void create_event_handler(){

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
void close_event_handler(){

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


