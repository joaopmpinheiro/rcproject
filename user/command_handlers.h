#ifndef COMMAND_HANDLERS_H
#define COMMAND_HANDLERS_H

#include <netdb.h>

typedef enum CommandType {
    LOGIN,
    CHANGEPASS,
    UNREGISTER,
    LOGOUT,
    EXIT,
    CREATE,
    CLOSE,
    MYEVENTS,
    LIST,
    SHOW,
    RESERVE,
    MYRESERVATIONS,
    UNKNOWN,
} CommandType;

CommandType identify_command(char* command);

void command_handler(CommandType command, char* args, int udp_fd,
     struct sockaddr_in* server_udp_addr);

int login_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len);
int unregister_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len);
int logout_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len);
int myevent_handler(char* args, int udp_fd, struct sockaddr_in* server_udp_addr,
     socklen_t udp_addr_len);


#endif