#ifndef __UTILS_H__
#define __UTILS_H__

// socket_manager.c
int select_handler();
void udp_connection();
void tcp_connection();

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


#endif
