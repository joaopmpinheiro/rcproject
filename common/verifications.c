#include "verifications.h"
#include "common.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>



int is_number(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) return INVALID;
    }
    return VALID;
}

int is_valid_seat_count(const char *seat_str) {
    if (!is_number(seat_str)) return INVALID;

    int seats = atoi(seat_str);
    if (seats < MIN_AVAIL_SEATS || seats > MAX_AVAIL_SEATS) return INVALID;

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

int verify_eid_format(char* eid) {
    if (eid == NULL) return INVALID;
    if(!is_number(eid)) return INVALID;
    int eid_num = atoi(eid);
    if (eid_num < 1 || eid_num > MAX_EVENTS) return INVALID;
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

int verify_event_name_format(char* event_name) {
    if (event_name == NULL) return INVALID;

    size_t len = strlen(event_name);
    if (len > 0 && event_name[len - 1] == '\n') {
        event_name[len - 1] = '\0';
        len--;
    }
    
    if (len == 0 || len > MAX_EVENT_NAME) return INVALID;

    // Verify alphanumeric
    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)event_name[i])) return INVALID;
    }

    return VALID;
}

int verify_event_date_format(char* date_str) {
    // Expected format: DD-MM-YYYY HH:MM
    if (date_str == NULL) return INVALID;
    
    size_t len = strlen(date_str);
    if (len > 0 && date_str[len - 1] == '\n') {
        date_str[len - 1] = '\0';
        len--;
    }
    
    // FIXME: vem sempre 01 no dia ou pode vir 1??
    if (len != 16) return INVALID;  // DD-MM-YYYY HH:MM = 16 characters

    // Check fixed positions for '-', ' ', and ':'
    if (date_str[2] != '-' || date_str[5] != '-' || date_str[10] != ' ' || date_str[13] != ':') {
        return INVALID;
    }

    // Check numeric parts (skip separators at positions 2, 5, 10, 13)
    for (int i = 0; i < 16; i++) {
        if (i == 2 || i == 5 || i == 10 || i == 13) continue; // Skip separators
        if (!isdigit((unsigned char)date_str[i])) return INVALID;
    }

    // Extract day, month, year, hour, minute
    int day = (date_str[0] - '0') * 10 + (date_str[1] - '0');
    int month = (date_str[3] - '0') * 10 + (date_str[4] - '0');
    int year = (date_str[6] - '0') * 1000 + (date_str[7] - '0') * 100 + 
               (date_str[8] - '0') * 10 + (date_str[9] - '0');
    int hour = (date_str[11] - '0') * 10 + (date_str[12] - '0');
    int minute = (date_str[14] - '0') * 10 + (date_str[15] - '0');

    // Validate month
    if (month < 1 || month > 12) return INVALID;

    // Validate hour and minute
    if (hour < 0 || hour > 23) return INVALID;
    if (minute < 0 || minute > 59) return INVALID;

    // Validate day based on month
    int days_in_month;
    if (month == 2) {
        // Check if leap year
        int is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        days_in_month = is_leap ? 29 : 28;
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        days_in_month = 30;
    } else {
        days_in_month = 31;
    }

    if (day < 1 || day > days_in_month) return INVALID;

    // Verify date is today or in the future
    time_t now = time(NULL);
    struct tm *today = localtime(&now);

    int current_day = today->tm_mday;
    int current_month = today->tm_mon + 1;  // tm_mon is 0-11
    int current_year = today->tm_year + 1900;  // tm_year is years since 1900
    int current_hour = today->tm_hour;
    int current_minute = today->tm_min;
    int current_second = today->tm_sec;

    // Compare dates in YYYYMMDD format
    int provided_date = year * 10000 + month * 100 + day;
    int today_date = current_year * 10000 + current_month * 100 + current_day;

    // Date is in the past
    if (provided_date < today_date) return INVALID;

    // If it's today, also check the time (hour:minute:second)
    if (provided_date == today_date) {
        int provided_time = hour * 10000 + minute * 100 + 0;  // Treat as HH:MM:00
        int current_time = current_hour * 10000 + current_minute * 100 + current_second;
        
        if (provided_time < current_time) return INVALID;  // Time is in the past
    }

    return VALID;
}

int verify_seat_count(char* count_str) {
    if (!is_number(count_str)) return INVALID;

    int count = atoi(count_str);
    if (count < MIN_AVAIL_SEATS || count > MAX_AVAIL_SEATS) return INVALID;

    return VALID;
}

int verify_reserved_seats(char* reserved_str, char* total_str) {
    if (!is_number(reserved_str) || !is_number(total_str)) return INVALID;

    int reserved = atoi(reserved_str);
    int total = atoi(total_str);
    if (reserved < 0 || reserved > total) return INVALID;

    return VALID;
}

int verify_file_name_format(char* file_name) {
    if (file_name == NULL) return INVALID;

    size_t len = strlen(file_name);
    if (len > 0 && file_name[len - 1] == '\n') {
        file_name[len - 1] = '\0';
        len--;
    }

    if (len == 0 || len > FILE_NAME_LENGTH) return INVALID;

    // Verify alphanumeric, dots, underscores, and hyphens
    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)file_name[i]) && 
            file_name[i] != '_' && 
            file_name[i] != '-' && 
            file_name[i] != '.') {
            return INVALID;
        }
    }

    return VALID;
}

int verify_file_size(char* file_size) {
    if (!is_number(file_size)) return INVALID;
    long size = atol(file_size);
    if (size == 0 || size > MAX_FILE_SIZE) return INVALID;
    return VALID;
}

