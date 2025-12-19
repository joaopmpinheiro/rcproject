#ifndef COMMAND_HANDLERS_H
#define COMMAND_HANDLERS_H

#include "../common/common.h"
#include <netdb.h>



// ------------ command_handler.c -------------
ReplyStatus handle_response_code(char* resp, RequestType command, int parsed, int n, char* status);
RequestType identify_command(char* command);
ReplyStatus identify_status_code(const char* status);
ReplyStatus command_handler(RequestType command, char** cursor, int udp_fd,
struct sockaddr_in* server_udp_addr);
const char* get_command_response_code(RequestType command);
RequestType identify_command_response(char* command);





// ---------------- commands.c -----------------
int verify_file(char* file_name);

/**
 * @brief Does the login of a user or creates a new user if not registered.
 * 
 * USER INPUT: login UID password 
 * 
 * USER PROTOCOL: UID password 
 * 
 * SERVER PROTOCOL: RLI status
 *  
 * @param cursor 
 * @param udp_fd 
 * @param server_udp_addr 
 * @param udp_addr_len 
 * @return ReplyStatus
 */
ReplyStatus login_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
            socklen_t udp_addr_len);

/**
 * @brief Unregisters the logged-in user.
 * 
 * USER INPUT: unregister
 * 
 * USER PROTOCOL: UNR <uid> <password>
 * 
 * SERVER PROTOCOL: RUR <status>
 * 
 * @param cursor 
 * @param udp_fd 
 * @param server_udp_addr 
 * @param udp_addr_len 
 * @return ReplyStatus 
 */
ReplyStatus unregister_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
                                socklen_t udp_addr_len);

/**
 * @brief Logs out the logged-in user.
 * 
 * USER INPUT: logout
 * 
 * USER PROTOCOL: LOU <uid> <password>
 * 
 * SERVER PROTOCOL: RLO <status>
 * 
 * @param cursor 
 * @param udp_fd 
 * @param server_udp_addr 
 * @param udp_addr_len 
 * @return ReplyStatus 
 */
ReplyStatus logout_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len);

/**
 * @brief Asks the server for the events created by the logged-in user.
 * USER INPUT: myevents or mye
 * USER PROTOCOL: LME <uid> <password>
 * SERVER PROTOCOL: RME <status> [<event1ID state>...]*
 * 
 * @param cursor 
 * @param udp_fd 
 * @param server_udp_addr 
 * @param udp_addr_len 
 * @return ReplyStatus 
 */
ReplyStatus myevent_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
                            socklen_t udp_addr_len);

/**
 * @brief Lists the events reserved by the logged-in user (by up to 50 events).
 * USER INPUT: myreservations or myres
 * USER PROTOCOL: LMR <uid> <password>
 * SERVER PROTOCOL: RMR <status> [<event1ID name event_date seats
 * reserved> <event2ID name event_date seats_reserved> ...]
 * @param 
 */
ReplyStatus myreservations_handler(char** cursor, int udp_fd,
                                struct sockaddr_in* server_udp_addr,
                                socklen_t udp_addr_len);                      


                             
/**
 * @brief Changes the password of the logged-in user.
 * 
 * USER INPUT: changePass old_password new_password
 * 
 * USER PROTOCOL: CPS UID oldPassword newPassword
 * 
 * SERVER PROTOCOL: RCP <status>
 * 
 * @param cursor 
 * @return ReplyStatus 
 */
ReplyStatus changepass_handler(char** cursor);

/**
 * @brief Create a new event by the logged in user.
 * 
 * USER INPUT: create <event_name> <file_name> <event_date> <num_seats> 
 * 
 * USER PROTOCOL: CRE <uid> <password> <name> <event_date> <attendance_size>
 * <Fname> <Fsize> <Fdata>
 * 
 * SERVER PROTOCOL: RCE <status> [<eid>]
 * 
 * @param cursor 
 * @param extra_info 
 * @return ReplyStatus 
 */
ReplyStatus create_event_handler(char** cursor, char** extra_info);

/**
 * @brief Closes an event created by the logged-in user.
 * 
 * USER INPUT: close <eid>
 * 
 * USER PROTOCOL: CLS <uid> <password> <eid>
 * 
 * SERVER PROTOCOL: RCL <status>
 * 
 * @param cursor 
 * @return ReplyStatus 
 */
ReplyStatus close_event_handler(char** cursor);

/**
 * @brief Lists all events in the server.
 * 
 * USER INPUT: list
 * 
 * USER PROTOCOL: LST
 * 
 * SERVER PROTOCOL: RST <status> [<event1ID> <name> <state> <date>] * 
 * 
 * @param cursor 
 * @return ReplyStatus 
 */
ReplyStatus list_handler(char** cursor);

/**
 * @brief Shows the details of a specific event.
 * 
 * USER INPUT: show <eid>
 * 
 * USER PROTOCOL: SED <eid>
 * 
 * SERVER PROTOCOL: RSE status [UID name event_date attendance_size Seats_reserved Fname
 * Fsize Fdata]
 * 
 * @param cursor 
 * @return ReplyStatus 
 */
ReplyStatus show_handler(char** cursor);

/**
 * @brief Reserve seats for an event.
 * 
 * USER INPUT: reserve <eid> <num_seats>
 * 
 * USER PROTOCOL: RID UID password EID people.
 * 
 * SEVER PROTOCOL: RRI status [ n_seats ]*.
 * 
 * @param cursor 
 * @return ReplyStatus 
 */
ReplyStatus reserve_handler(char** cursor);





// ---------- messages.c ----------
void usage(const char *prog_name);
void print_result(RequestType command, ReplyStatus status, char* extra_info);
void event_message(char* eid);
void show_event_details(char* eid, char* uid, char* event_name, char* event_date,
                        char* total_seats, char* reserved_seats,
                        char* file_name, char* file_size);
void show_events_list(int tcp_fd);       
void show_event_reservations(char* seats_left, char* eid); 
ReplyStatus show_myreservations(char* cursor_lst);            




// ---------- socket_manager.c ----------
int setup_udp(const char* ip, const char* port, struct sockaddr_in* server_addr);
int connect_tcp(const char* ip, const char* port);
ReplyStatus udp_send_receive(int udp_fd, struct sockaddr_in* server_udp_addr,
                            socklen_t udp_addr_len, char* request, char* response, 
                            size_t response_size);
ReplyStatus tcp_send_receive(char* request,  char* response, size_t response_size);




// ---------- user_parser.c ---------- 
ReplyStatus parse_eid(char **cursor, char* eid);
ReplyStatus parse_login(char **cursor, char* uid, char* password);
ReplyStatus parse_reserve(char **cursor, char* eid, char* num_seats);
ReplyStatus parse_create_event(char **cursor, char* event_name, char* file_name,
                             char* date, char* num_seats);
ReplyStatus parse_change_password(char** cursor, char* old_password,
                                 char* new_password, char* current_password); 
ReplyStatus parse_reservations(char **cursor, char* eid, char* event_date,
                              char* seats_reserved);                     
                            
ReplyStatus parse_response_header(char** cursor, RequestType request_type);                             



                                 
// ---------- read_from_server.c ----------
ReplyStatus read_command(int tcp_fd, char* command, RequestType expected_command);

/**
 * @brief Reads the command and status from the server. Verifies they match the~
 * expected command and if so the status is parsed and returned.
 * 
 * @param tcp_fd int
 * @param expected_command RequestType
 * @return ReplyStatus given by server or STATUS_UNEXPECTED_RESPONSE
 */
ReplyStatus read_cmd_status(int tcp_fd, RequestType expected_command);
ReplyStatus read_show_response_header(int tcp_fd,
                                       char* uid, char* event_name,
                                       char* event_date, char* attendance_size,
                                       char* reserved_seats, char* file_name,
                                       char* file_size);
ReplyStatus read_events_list(int fd_tcp, char* eid, char* name, char* state,
                              char* event_day, char* event_time);  
#endif