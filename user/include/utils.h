#ifndef COMMAND_HANDLERS_H
#define COMMAND_HANDLERS_H

#include "../common/common.h"
#include <netdb.h>



// ------------ command_handler.c -------------
ReplyStatus handle_response_code(char* resp, RequestType command, int parsed, int n, char* status);
RequestType identify_command(char* command);
const char* get_command_name(RequestType command);
ReplyStatus parse_status_code(const char* status);
void command_handler(RequestType command, char** cursor, int udp_fd,
     struct sockaddr_in* server_udp_addr);
const char* get_command_response_code(RequestType command);
const char* get_command_code(RequestType command);
RequestType identify_command_response(char* command);


// ------------ commands.c -------------
int verify_file(char* file_name);
ReplyStatus login_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
            socklen_t udp_addr_len);

ReplyStatus changepass_handler(char** cursor);

ReplyStatus unregister_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
                                socklen_t udp_addr_len);
ReplyStatus logout_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len);
ReplyStatus myevent_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
                            socklen_t udp_addr_len);
/**
 * @brief Sends TCP request to create a new event and handles the response.
 * Receives:
 * NOK - event could not be created
 * NGL - user not logged in
 * OK EID- event created successfully 
 * PROTOCOL: CRE <uid> <password> <name> <event_date> <attendance_size> <Fname> <Fsize> <Fdata>
 * 
 * @param args [event_name event_file_name event_date num_seats]
 * @return ReplyStatus 
 */
ReplyStatus create_event_handler(char** cursor, char** extra_info);
ReplyStatus close_event_handler(char** cursor);
ReplyStatus list_handler(char** cursor);
ReplyStatus show_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
                            socklen_t udp_addr_len);


// ---------- messages.c ----------
void usage(const char *prog_name);
void print_result(RequestType command, ReplyStatus status, char* extra_info);
void show_event_details(char* eid, char* uid, char* event_name, char* event_date,
                        char* total_seats, char* reserved_seats,
                        char* file_name, char* file_size);


// ---------- socket_manager.c ----------
int setup_udp(const char* ip, const char* port, struct sockaddr_in* server_addr);
int connect_tcp(const char* ip, const char* port);
ReplyStatus udp_send_receive(int udp_fd, struct sockaddr_in* server_udp_addr,
                            socklen_t udp_addr_len, char* request, char* response, 
                            size_t response_size);
ReplyStatus tcp_send_receive(char* request,  char* response, size_t response_size);


// ---------- user_parser.c ---------- 
ReplyStatus parse_create_event(char **cursor, char* event_name, char* file_name,
                            char* date, char* num_seats);   
ReplyStatus parse_close(char** cursor,  char* eid);
ReplyStatus parse_change_password(char** cursor, char* old_password,
                                 char* new_password, char* current_password);                         
ReplyStatus parse_events_list(char** event_list, char* eid, char* name, char state, char* event_day, char* event_time);
ReplyStatus parse_show(char** cursor, char* eid);

// ---------- server_response_parser.c ----------
ReplyStatus read_show_response_header(char* response, int tcp_fd,
                                       char* resp_code, char* rep_status, 
                                       char* uid, char* event_name, char* event_date,
                                       char* total_seats, char* reserved_seats,
                                       char* file_name, char* file_size);

#endif