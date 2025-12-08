#ifndef __UTILS_H__
#define __UTILS_H__

// socket_manager.c

int select_handler();
void udp_connection();
void tcp_connection();
void send_udp_response(const char* message, Request *req);   


//connection.c
int tcp_setup();
int udp_setup();
int connect_tcp(char* ip, char* port);
int setup_udp(char* ip, char* port, struct sockaddr_in* server_addr);
void server_setup();

// connection.c
void parse_arguments(int argc, char *argv[]);

//error.c
void usage(const char *prog_name);


// command_handler.c
/**
 * @brief Verifies if the request has valid UID and password arguments.
 * 
 * @param req The request to verify.
 * @return int Returns VALID if both UID and password are valid, otherwise returns INVALID.
 */
int verify_uid_password(Request* req);

/**
 * @brief Identifies the request based on the command string.
 * 
 * @param command The string of the command to identify.
 * @return RequestType
 */
RequestType identify_request_type(char* command);

/**
 * @brief Handles the given request.
 * 
 * @param req The request to handle.
 */
void handle_udp_request(Request* req);

/**
 * @brief Get the user by UID.
 * 
 * @param UID int
 * @return User* 
 */
User* get_user_by_uid(int UID);

/**
 * @brief Creates a new user with the given UID and password.
 * 
 * @param UID int
 * @param password char*
 */
void create_user(int UID, char* password);

/**
 * @brief Handles log in request: LIN UID password.
 * 
 * Sends to user:
 * RLI OK - successful login,
 * RLI NOK - incorrect password,
 * RLI REG - new user registered
 * @param req 
 */
void login_handler(Request* req);
void logout_handler();
void unregister_handler();
void change_password_handler();
void create_event_handler();
void close_event_handler();
void show_event_handler();
void reserve_seats_handler();


// ------------- file_manager.c ---------------
int check_file(char *fname);
int dir_exists(const char* path);
char* read_file(const char* filename);

// ------------ users_manager.c ---------------
int user_exists(char* UID);
int create_new_user(char* UID, char* password);
int create_USER_dir(char* UID);
int write_password(char* UID, char* password);
int write_login(char* UID);
int get_password(char* UID, char* password);

// ------------ events_manager.c ---------------
// To be implemented

#endif
