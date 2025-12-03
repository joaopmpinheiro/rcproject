#ifndef __UTILS_H__
#define __UTILS_H__

//connection.c
char* get_server_ip(int argc, char* argv[]);
char* get_server_port(int argc, char* argv[]);
int connect_tcp(char* ip, char* port);
int setup_udp(char* ip, char* port, struct sockaddr_in* server_addr);

// connection.c
void parse_arguments(int argc, char *argv[]);

//error.c
void usage(const char *prog_name);

#endif
