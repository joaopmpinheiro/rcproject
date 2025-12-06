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
int identify_request_type(Request* req);
int manage_UDP_request(Request* req);
int manage_TCP_request(Request* req);
void handle_request(Request* req);



#endif
