#include "verifications.h"
#include "common.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>



int is_number(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) return INVALID;
    }
    return VALID;
}

int is_valid_port(const char *port_str) {
    if (!is_number(port_str)) return INVALID;

    int port = atoi(port_str);
    if (port < PORTMIN || port > PORTMAX) return INVALID;

    return VALID;
}

int verify_argument_count(char* args, int expected_count) {
    if (args == NULL) return expected_count == 0;

    int count = 0;
    char* temp = strdup(args);
    char* token = strtok(temp, " ");
    while (token != NULL) {
        count++;
        token = strtok(NULL, " ");
    }
    free(temp);

    return count == expected_count;
}

int verify_uid_format(char* uid) {
    if (uid == NULL) return INVALID;

    size_t len = strlen(uid);
    if (len > 0 && uid[len - 1] == '\n') {
        uid[len - 1] = '\0';
        len--;
    }

    if (len != UID_LENGTH) return INVALID;
    if(!is_number(uid)) return INVALID;

    return VALID;
}

int verify_password_format(char* password) {
    if (password == NULL) return INVALID;

    size_t len = strlen(password);
    if (len > 0 && password[len - 1] == '\n') {
        password[len - 1] = '\0';
        len--;
    }

    if (len != PASSWORD_LENGTH) return INVALID;

    // verify alphanumeric
    for (size_t i = 0; i < PASSWORD_LENGTH; i++) {
        if (!isalnum((unsigned char)password[i])) return INVALID;
    }

    return VALID;
}
