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
void command_handler(CommandType command, char* args);
int login_handler(char* request, char* args);

#endif