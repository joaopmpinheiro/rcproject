#ifndef CLIENT_DATA_H
#define CLIENT_DATA_H

#include "../common/common.h"
#include "../common/data.h"

extern char current_uid[UID_LENGTH + 1];
extern char current_password[PASSWORD_LENGTH + 1];
extern int is_logged_in;
extern char IP[MAX_HOSTNAME_LENGTH];
extern char PORT[6];

#endif