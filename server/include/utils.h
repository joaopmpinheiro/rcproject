#ifndef __UTILS_H__
#define __UTILS_H__

// socket_manager.c
int select_handler();
void udp_connection();
void tcp_connection();
void send_udp_response(const char* message, struct sockaddr_in* client_addr, socklen_t addr_len, int udp_socket);   


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
int correct_args_UID_password(Request* req);
RequestType identify_request_type(char* command);
void handle_request(Request* req);

User* get_user_by_uid(int UID);
void create_user(int UID, char* password);

void login_handler(Request* req);
void logout_handler();
void unregister_handler();
void change_password_handler();
void create_event_handler();
void close_event_handler();
void show_event_handler();
void reserve_seats_handler();




#endif
