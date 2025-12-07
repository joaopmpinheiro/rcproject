#include "verifications.h"
#include "common.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

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
    if (uid == NULL) return 0;

    size_t len = strlen(uid);
    if (len > 0 && uid[len - 1] == '\n') {
        uid[len - 1] = '\0';
        len--;
    }

    if (len != 6) return 0;

    for (size_t i = 0; i < 6; i++) {
        if (!isdigit((unsigned char)uid[i])) return 0;
    }

    return 1;
}

int verify_password_format(char* password) {
    if (password == NULL) return 0;

    size_t len = strlen(password);
    if (len > 0 && password[len - 1] == '\n') {
        password[len - 1] = '\0';
        len--;
    }

    if (len != 8) return 0;

    for (size_t i = 0; i < 8; i++) {
        if (!isalnum((unsigned char)password[i])) return 0;
    }

    return 1;
}

