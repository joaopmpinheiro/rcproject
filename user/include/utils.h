#ifndef COMMAND_HANDLERS_H
#define COMMAND_HANDLERS_H

#include "../common/common.h"
#include <netdb.h>



// ------------ command_handler.c -------------
ReplyStatus handle_response_code(char* resp, CommandType command, int parsed, int n, char* status);
CommandType identify_command(char* command);
const char* get_command_name(CommandType command);
ReplyStatus parse_status_code(const char* status);
void command_handler(CommandType command, char** cursor, int udp_fd,
     struct sockaddr_in* server_udp_addr);
const char* get_command_response_code(CommandType command);
const char* get_command_code(CommandType command);


// ------------ commands.c -------------
int verify_file(char* file_name);
ReplyStatus login_handler(char** cursor, int udp_fd, struct sockaddr_in* server_udp_addr,
            socklen_t udp_addr_len);
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
ReplyStatus create_event_handler(char** args, char** extra_info);


// ---------- messages.c ----------
void usage(const char *prog_name);
void print_result(CommandType command, ReplyStatus status, char* extra_info);


// ---------- socket_manager.c ----------
int setup_udp(const char* ip, const char* port, struct sockaddr_in* server_addr);
int connect_tcp(const char* ip, const char* port);
ReplyStatus udp_send_receive(int udp_fd, struct sockaddr_in* server_udp_addr,
                            socklen_t udp_addr_len, char* request, char* response);



#endif