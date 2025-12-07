#ifndef __COMMON_H__
#define __COMMON_H__

#define BASE_PORT 58000
#define GROUP_NUMBER 32
#define DEFAULT_PORT "58232"
#define DEFAULT_IP "127.0.0.1"
#define PORTMAX 65535
#define PORTMIN 0 

#define TIMEOUT_SECONDS 5
#define PASSWORD_LENGTH 8
#define COMMAND_LENGTH 3
#define UID_LENGTH 6

#define SUCCESS 1
#define FAILURE 0

#define VALID 1
#define INVALID 0

int is_number(const char *str);
int is_valid_IP(const char *ip);
int is_valid_port(const char *port_str);
#endif
