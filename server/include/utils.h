#ifndef __UTILS_H__
#define __UTILS_H__

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "globals.h"

// =============== socket_manager.c ===============

/**
 * @brief Handles incoming connections using select() multiplexing.
 * 
 * Monitors both UDP and TCP sockets for incoming data and dispatches
 * to appropriate handlers.
 * 
 * @return int SUCCESS on normal operation, ERROR on critical failure
 */
int select_handler();

/**
 * @brief Handles an incoming UDP connection.
 * 
 * Receives a UDP datagram and dispatches it to the UDP request handler.
 */
void udp_connection();

/**
 * @brief Handles an incoming TCP connection.
 * 
 * Accepts a new TCP connection, reads the request, and dispatches to handler.
 */
void tcp_connection();

/**
 * @brief Sends a UDP response message to the client.
 * 
 * @param message Response message (should end with newline)
 * @param req Request structure containing client address info
 */
void send_udp_response(const char* message, Request *req);


// =============== connection.c ===============

/**
 * @brief Sets up the TCP listening socket for the server.
 * 
 * @return int The socket file descriptor on success, ERROR on failure
 */
int tcp_setup();

/**
 * @brief Sets up the UDP socket for the server.
 * 
 * @return int The socket file descriptor on success, ERROR on failure
 */
int udp_setup();

/**
 * @brief Establishes a TCP connection to a remote server.
 * 
 * @param ip IP address of the remote server
 * @param port Port number of the remote server
 * @return int The socket file descriptor on success, ERROR on failure
 */
int connect_tcp(char* ip, char* port);

/**
 * @brief Sets up a UDP socket and configures the server address structure.
 * 
 * @param ip IP address of the server
 * @param port Port number of the server
 * @param server_addr Pointer to sockaddr_in structure to be filled
 * @return int The socket file descriptor on success, ERROR on failure
 */
int setup_udp(char* ip, char* port, struct sockaddr_in* server_addr);

/**
 * @brief Initializes the server by setting up UDP and TCP sockets.
 */
void server_setup();

/**
 * @brief Parses command line arguments for server configuration.
 * 
 * Handles -p (port) and -v (verbose) flags.
 * 
 * @param argc Argument count
 * @param argv Argument vector
 */
void parse_arguments(int argc, char *argv[]);


// =============== error.c ===============

/**
 * @brief Logs a server message with optional client address information.
 * 
 * Only prints if verbose mode is enabled.
 * 
 * @param message The message to log
 * @param client_addr Client address (can be NULL)
 */
void server_log(const char* message, struct sockaddr_in* client_addr);

/**
 * @brief Prints usage information for the server.
 * 
 * @param prog_name Name of the executable
 */
void usage(const char *prog_name);


// =============== command_handler.c ===============

/**
 * @brief Validates basic UID and password format from request buffer.
 * 
 * @param req Request containing buffer with command, UID, and password
 * @return int VALID if format is correct, INVALID if verification fails
 */
int verify_uid_password(Request* req);

/**
 * @brief Handles an incoming UDP request.
 * 
 * Parses the command, validates UID and password, and dispatches to
 * the appropriate handler (login, logout, unregister, myevents, myreservations).
 * 
 * @param req The UDP request to handle
 */
void handle_udp_request(Request* req);

/**
 * @brief Handles an incoming TCP request.
 * 
 * Parses the command and dispatches to the appropriate handler
 * (create, close, list, show, reserve, changepass).
 * 
 * @param req The TCP request to handle
 */
void handle_tcp_request(Request* req);

/**
 * @brief Handles login request: LIN UID password
 * 
 * Sends to user:
 * - RLI OK - successful login
 * - RLI NOK - incorrect password
 * - RLI REG - new user registered
 * - RLI ERR - server error
 * 
 * @param req The request structure
 * @param UID User ID (6 digits)
 * @param password User password (8 alphanumeric chars)
 */
void login_handler(Request* req, char* UID, char* password);

/**
 * @brief Handles logout request: LOU UID password
 * 
 * Sends to user:
 * - RLO OK - successful logout
 * - RLO UNR - user was not registered
 * - RLO NOK - user not logged in
 * - RLO WRP - wrong password
 * 
 * @param req The request structure
 * @param UID User ID
 * @param password User password
 */
void logout_handler(Request* req, char* UID, char* password);

/**
 * @brief Handles unregister request: UNR UID password
 * 
 * Sends to user:
 * - RUR OK - successful unregistration
 * - RUR UNR - user was not registered
 * - RUR NOK - user not logged in
 * - RUR WRP - wrong password
 * 
 * @param req The request structure
 * @param UID User ID
 * @param password User password
 */
void unregister_handler(Request* req, char* UID, char* password);

/**
 * @brief Handles myevents request: LME UID password
 * 
 * Sends to user:
 * - RME OK [EID state]* - list of user's events with states
 * - RME NOK - user has no events
 * - RME NLG - user not logged in
 * - RME WRP - wrong password
 * 
 * Event states: 0=past, 1=active, 2=sold out, 3=closed
 * 
 * @param req The request structure
 * @param UID User ID
 * @param password User password
 */
void myevents_handler(Request* req, char* UID, char* password);

/**
 * @brief Formats the list of events created by a user.
 * 
 * Scans the user's CREATED directory and builds a response string
 * with each event's EID and current state.
 * 
 * @param UID User ID
 * @param message Buffer to store the formatted response
 * @param message_size Size of the message buffer
 * @return int SUCCESS on success, ERROR on failure
 */
int format_list_of_user_events(char* UID, char* message, size_t message_size);

/**
 * @brief Handles myreservations request: LMR UID password
 * 
 * Sends to user:
 * - RMR OK [EID date value]* - list of user's reservations (max 50)
 * - RMR NOK - user has no reservations
 * - RMR NLG - user not logged in
 * - RMR WRP - wrong password
 * 
 * @param req The request structure
 * @param UID User ID
 * @param password User password
 */
void myreservations_handler(Request* req, char* UID, char* password);

/**
 * @brief Handles create event request: CRE UID password name date seats fname fsize fdata
 * 
 * Sends to user:
 * - RCE OK EID - successful event creation with assigned EID
 * - RCE NOK - failed event creation
 * - RCE NLG - user not logged in
 * - RCE WRP - wrong password
 * - RCE ERR - error in request or server
 * 
 * @param req The request structure
 */
void create_event_handler(Request* req);

/**
 * @brief Handles close event request: CLS UID password EID
 * 
 * Sends to user:
 * - RCL OK - successful event closure
 * - RCL NLG - user not logged in
 * - RCL NOK - wrong password
 * - RCL NOE - no event with given EID
 * - RCL EOW - user is not the event creator
 * - RCL SLD - event is sold out
 * - RCL PST - event date has already passed
 * - RCL CLO - event is already closed
 * 
 * @param req The request structure
 */
void close_event_handler(Request* req);

/**
 * @brief Handles list events request: LST
 * 
 * Sends to user:
 * - RLS OK [EID name state event_date]* - list of all events
 * - RLS NOK - no events available
 * 
 * @param req The request structure
 */
void list_events_handler(Request* req);

/**
 * @brief Handles show event request: SED EID
 * 
 * Sends to user:
 * - RSE OK UID name date seats reserved fname fsize fdata - event details with file
 * - RSE NOK - event does not exist or other problem
 * 
 * @param req The request structure
 */
void show_event_handler(Request* req);

/**
 * @brief Handles change password request: CPS UID oldPassword newPassword
 * 
 * Sends to user:
 * - RCP OK - successful password change
 * - RCP NLG - user not logged in
 * - RCP NOK - wrong password
 * - RCP NID - unknown user
 * 
 * @param req The request structure
 */
void change_password_handler(Request* req);

/**
 * @brief Handles reserve seats request: RID UID password EID num_seats
 * 
 * Sends to user:
 * - RRI ACC - reservation successful
 * - RRI NOK - event not active
 * - RRI NLG - user not logged in
 * - RRI WRP - wrong password
 * - RRI CLS - event closed
 * - RRI SLD - event sold out
 * - RRI REJ n_seats - not enough available seats
 * - RRI PST - event date has passed
 * 
 * @param req The request structure
 */
void reserve_seats_handler(Request* req);

/**
 * @brief Formats event details for show event response.
 * 
 * Reads event metadata and builds the response string.
 * 
 * @param EID Event ID
 * @param message Buffer to store the formatted response
 * @param message_size Size of the message buffer
 * @param file_name Buffer to store the description filename
 * @param file_size Pointer to store the file size
 * @return int SUCCESS on success, ERROR on failure
 */
int format_event_details(char* EID, char* message, size_t message_size, char* file_name, long* file_size);


// =============== file_manager.c ===============

/**
 * @brief Checks if a file exists and is readable.
 * 
 * @param fname File path to check
 * @return int TRUE if file exists and is accessible, FALSE otherwise
 */
int check_file(char *fname);

/**
 * @brief Checks if a directory exists.
 * 
 * @param path Directory path to check
 * @return int TRUE if directory exists, FALSE otherwise
 */
int dir_exists(const char* path);

/**
 * @brief Checks if a directory is empty (contains no entries except . and ..).
 * 
 * @param path Directory path to check
 * @return int TRUE if empty, FALSE if contains files
 */
int is_dir_empty(const char* path);

/**
 * @brief Reads the entire contents of a file into a dynamically allocated string.
 * 
 * @param filename File path to read
 * @return char* Allocated string with file contents (caller must free), NULL on error
 */
char* read_file(const char* filename);

/**
 * @brief Checks if a file exists.
 * 
 * @param filename File path to check
 * @return int TRUE if file exists, FALSE otherwise
 */
int file_exists(const char* filename);

/**
 * @brief Recursively removes a directory and all its contents.
 * 
 * @param path Directory path to remove
 * @return int SUCCESS on success, ERROR on failure
 */
int remove_directory(const char *path);

/**
 * @brief Finds the first available EID (001-999) by scanning the EVENTS directory.
 * 
 * @param eid_str Buffer to store the 3-digit EID string (e.g., "001", "042")
 * @return int SUCCESS if an available EID was found, ERROR if all EIDs are taken
 */
int find_available_eid(char* eid_str);

/**
 * @brief Writes event metadata to EVENTS/{EID}/START_{EID}.txt file.
 * 
 * @param eid Event ID (3-digit string, e.g., "001")
 * @param uid User ID (6-digit string)
 * @param event_name Event name
 * @param desc_fname Description filename
 * @param event_attend Total attendance/seats (as string)
 * @param event_date Event date and time (DD-MM-YYYY HH:MM)
 * @return int SUCCESS if file was written successfully, ERROR otherwise
 */
int write_event_start_file(const char* eid, const char* uid, const char* event_name,
                           const char* desc_fname, const char* event_attend,
                           const char* event_date);

/**
 * @brief Writes event end marker to EVENTS/{EID}/END_{EID}.txt.
 * 
 * Creates the file to indicate the event has been closed.
 * 
 * @param eid Event ID (3-digit string, e.g., "001")
 * @return int SUCCESS if file was created successfully, ERROR otherwise
 */
int write_event_end_file(const char* eid);

/**
 * @brief Writes event metadata to USERS/{UID}/CREATED/{EID}.txt file.
 * 
 * Creates a record of the event in the user's CREATED directory.
 * 
 * @param eid Event ID (3-digit string, e.g., "001")
 * @param uid User ID (6-digit string)
 * @param event_name Event name
 * @param desc_fname Description filename
 * @param event_attend Total attendance/seats (as string)
 * @param event_date Event date and time (DD-MM-YYYY HH:MM)
 * @return int SUCCESS if file was written successfully, ERROR otherwise
 */
int write_event_information_file(const char* eid, const char* uid, const char* event_name,
                           const char* desc_fname, const char* event_attend,
                           const char* event_date);

/**
 * @brief Creates or updates RES_{EID}.txt with the reserved seats count.
 * 
 * If the file doesn't exist, creates it with the provided count.
 * If it exists, reads the current value and increments by reserved_seats.
 * 
 * @param eid Event ID (3-digit string, e.g., "001")
 * @param reserved_seats Number of seats to add to the reservation count
 * @return int SUCCESS if file was created/updated successfully, ERROR otherwise
 */
int update_reservations_file(const char* eid, int reserved_seats);

/**
 * @brief Creates DESCRIPTION directory and stores the event description file.
 * 
 * @param eid Event ID (3-digit string, e.g., "001")
 * @param file_name Name of the description file
 * @param file_size Size of the file in bytes
 * @param file_content Content of the file
 * @return int SUCCESS if directory and file were created, ERROR otherwise
 */
int write_description_file(const char* eid, const char* file_name, size_t file_size, const char* file_content);

/**
 * @brief Writes a reservation record to USERS/{UID}/RESERVED/{EID}.txt.
 * 
 * @param UID User ID
 * @param EID Event ID
 * @param num_seats Number of seats reserved
 * @return int SUCCESS on success, ERROR on failure
 */
int write_reservation(char* UID, char* EID, int num_seats);


// =============== users_manager.c ===============

/**
 * @brief Checks if a user with the given UID exists.
 * 
 * @param UID User ID to check
 * @return int TRUE if user exists, FALSE otherwise
 */
int user_exists(char* UID);

/**
 * @brief Creates a new user with the given UID and password.
 * 
 * Creates the user directory structure and stores the password.
 * 
 * @param UID User ID
 * @param password User password
 * @return int SUCCESS on success, ERROR on failure
 */
int create_new_user(char* UID, char* password);

/**
 * @brief Creates the directory structure for a new user.
 * 
 * @param UID User ID
 * @return int SUCCESS on success, ERROR on failure
 */
int create_user(char* UID);

/**
 * @brief Removes a user and all their data.
 * 
 * Deletes the user's directory and all its contents.
 * 
 * @param UID User ID
 * @return int SUCCESS on success, ERROR on failure
 */
int remove_user(char* UID);

/**
 * @brief Removes the login marker file for a user (logs them out).
 * 
 * @param UID User ID
 * @return int SUCCESS on success, ERROR on failure
 */
int erase_login(char* UID);

/**
 * @brief Writes the user's password to their password file.
 * 
 * @param UID User ID
 * @param password Password to store
 * @return int SUCCESS on success, ERROR on failure
 */
int write_password(char* UID, char* password);

/**
 * @brief Creates the login marker file for a user (logs them in).
 * 
 * @param UID User ID
 * @return int SUCCESS on success, ERROR on failure
 */
int write_login(char* UID);

/**
 * @brief Reads the user's password from their password file.
 * 
 * @param UID User ID
 * @param password Buffer to store the password
 * @return int SUCCESS on success, ERROR on failure
 */
int get_password(char* UID, char* password);

/**
 * @brief Checks if a user is currently logged in.
 * 
 * @param UID User ID
 * @return int TRUE if logged in, FALSE otherwise
 */
int is_logged_in(char* UID);

/**
 * @brief Verifies if a filename is a valid event file (3 digits + .txt).
 * 
 * @param event_file_name Filename to verify
 * @return int VALID if valid event file, INVALID otherwise
 */
int verify_event_file(char* event_file_name);

/**
 * @brief Checks if a user has created any events.
 * 
 * @param UID User ID
 * @return int TRUE if user has events, FALSE otherwise
 */
int has_events(char* UID);

/**
 * @brief Checks if a user has made any reservations.
 * 
 * @param UID User ID
 * @return int VALID if user has reservations, INVALID otherwise
 */
int has_reservations(char* UID);

/**
 * @brief Formats the list of user's reservations for the myreservations response.
 * 
 * @param UID User ID
 * @param response Buffer to store the formatted response
 * @param response_size Size of the response buffer
 * @return int Number of reservations formatted, or ERROR on failure
 */
int format_list_of_user_reservations(char* UID, char* response, size_t response_size);

/**
 * @brief Verifies if the provided password matches the user's stored password.
 * 
 * @param UID User ID
 * @param password Password to verify
 * @return int VALID if password matches, INVALID if wrong, ERROR on failure
 */
int verify_correct_password(char* UID, char* password);


// =============== events_manager.c ===============

/**
 * @brief Checks if event directory exists
 * @param EID Event ID to check
 * @return TRUE if EVENTS/EID directory exists, FALSE otherwise
 */
int event_exists(char* EID);

/**
 * @brief Verifies if a directory name is a valid event directory (3 digits).
 * 
 * @param event_dir_name Directory name to verify
 * @return int VALID if valid event directory, INVALID otherwise
 */
int verify_event_dir(char* event_dir_name);

/**
 * @brief Checks if a user is the creator of an event.
 * 
 * @param UID User ID
 * @param EID Event ID
 * @return int TRUE if user created the event, FALSE otherwise
 */
int is_event_creator(char* UID, char* EID);

/**
 * @brief Checks if an event is sold out (no seats available).
 * 
 * @param EID Event ID
 * @return int TRUE if sold out, FALSE otherwise
 */
int is_event_sold_out(char* EID);

/**
 * @brief Checks if an event's date has passed.
 * 
 * @param EID Event ID
 * @return int TRUE if event date is in the past, FALSE otherwise
 */
int is_event_past(char* EID);

/**
 * @brief Checks if an event has been closed by its creator.
 * 
 * @param EID Event ID
 * @return int TRUE if event is closed, FALSE otherwise
 */
int is_event_closed(char* EID);

/**
 * @brief Creates the directory structure for a new event.
 * 
 * @param EID Event ID (as integer)
 * @return int SUCCESS on success, ERROR on failure
 */
int create_eid_dir(int EID);

/**
 * @brief Reads basic event info for the list command.
 * 
 * @param EID Event ID
 * @param event_name Buffer to store event name
 * @param event_date Buffer to store event date
 * @return int SUCCESS on success, ERROR on failure
 */
int get_list_event_info(char* EID, char* event_name, char* event_date);

/**
 * @brief Reads all event details for the show command.
 * 
 * @param EID Event ID
 * @param UID Buffer to store creator's UID
 * @param event_name Buffer to store event name
 * @param event_date Buffer to store event date
 * @param total_seats Buffer to store total seats
 * @param reserved_seats Buffer to store reserved seats count
 * @param file_name Buffer to store description filename
 * @return int SUCCESS on success, ERROR on failure
 */
int read_event_full_details(char* EID, char* UID, char* event_name,
                            char* event_date, char* total_seats,
                            char* reserved_seats, char* file_name);

/**
 * @brief Gets the number of available (unreserved) seats for an event.
 * 
 * @param EID Event ID
 * @return int Number of available seats, or ERROR on failure
 */
int get_available_seats(char* EID);

/**
 * @brief Creates a reservation for a user.
 * 
 * Creates records in both the event and user directories.
 * 
 * @param UID User ID
 * @param EID Event ID
 * @param num_seats Number of seats to reserve
 * @return int SUCCESS on success, ERROR on failure
 */
int make_reservation(char* UID, char* EID, int num_seats);


#endif
