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
 * @param req The UDP request to handle.
 */
void handle_udp_request(Request* req);

/**
 * @brief Handles the given TCP request.
 * 
 * @param req The TCP request to handle.
 */
void handle_tcp_request(Request* req);

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

/**
 * @brief Handles create event request: CRE UID password event_name event_fname event_date num_attendees
 * 
 * Sends to user:
 * RCR OK EID - successful event creation with event ID EID
 * RCR NOK - failed event creation
 * @param req 
 */
void create_event_handler(Request* req);
void close_event_handler();
void show_event_handler();
void reserve_seats_handler();


// ------------- file_manager.c ---------------
int check_file(char *fname);
int dir_exists(const char* path);
char* read_file(const char* filename);
int file_exists(const char* filename);

/**
 * @brief Finds the first available EID (001-999) by scanning the EVENTS directory.
 * 
 * @param eid_str Buffer to store the 3-digit EID string (e.g., "001", "042")
 * @return int VALID if an available EID was found, INVALID if all EIDs are taken
 */
int find_available_eid(char* eid_str);

/**
 * @brief Writes event metadata to START_{EID}.txt file in the event directory.
 * 
 * @param eid Event ID (3-digit string, e.g., "001")
 * @param uid User ID (6-digit string)
 * @param event_name Event name
 * @param desc_fname Description filename
 * @param event_attend Total attendance/seats (as string)
 * @param event_date Event date and time (DD-MM-YYYY HH:MM)
 * @return int VALID if file was written successfully, INVALID otherwise
 */
int write_event_start_file(const char* eid, const char* uid, const char* event_name,
                           const char* desc_fname, const char* event_attend,
                           const char* event_date);

/**
 * @brief Creates or updates RES_{EID}.txt file with the number of reserved seats.
 * 
 * If the file doesn't exist, it creates it with the provided reserved_seats count.
 * If the file exists, it reads the current value and increments it by reserved_seats.
 * 
 * @param eid Event ID (3-digit string, e.g., "001")
 * @param reserved_seats Number of seats to add to the reservation count
 * @return int VALID if file was created/updated successfully, INVALID otherwise
 */
int update_reservations_file(const char* eid, int reserved_seats);

/**
 * @brief Creates a DESCRIPTION directory inside the event directory and stores the description file.
 * 
 * @param eid Event ID (3-digit string, e.g., "001")
 * @param file_name Name of the description file
 * @param file_size Size of the file in bytes
 * @param file_content Content of the file
 * @return int SUCCESS if directory and file were created, ERROR otherwise
 */
int write_description_file(const char* eid, const char* file_name, size_t file_size, const char* file_content);

// ------------ users_manager.c ---------------
int user_exists(char* UID);
int create_new_user(char* UID, char* password);
int create_USER_dir(char* UID);
int write_password(char* UID, char* password);
int write_login(char* UID);
int get_password(char* UID, char* password);

// ------------ events_manager.c ---------------
int event_exists(char* EID);
int create_EVENT_dir (char* EID);

#endif
