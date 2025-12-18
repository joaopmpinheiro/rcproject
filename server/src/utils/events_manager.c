#include "../../include/globals.h"
#include "../../include/utils.h"
#include <time.h>

int event_exists(char* EID){
    char EID_dirname[20];
    sprintf(EID_dirname, "EVENTS/%s", EID);
    return dir_exists(EID_dirname);
}

// VALID if event is closed INVALID otherwise
int is_event_closed(char* EID){
    char state_path[30];
    sprintf(state_path, "EVENTS/%s/END_%s.txt", EID, EID);
    return file_exists(state_path) ? TRUE : FALSE;
}

int verify_event_dir(char* event_dir_name){
    // Check length is exactly 3
    if (strlen(event_dir_name) != 3) return INVALID;

    // Check all 3 characters are digits
    for (int i = 0; i < 3; i++) {
        if (!isdigit((unsigned char)event_dir_name[i]))
            return INVALID;
    }
    
    // Check range 001-999
    int eid = atoi(event_dir_name);
    if (eid < 1 || eid > 999) return INVALID;
    
    return VALID;
}

int is_event_creator(char* UID, char* EID){
    char event_info_fname[30];
    sprintf(event_info_fname, "EVENTS/%s/START_%s.txt", EID, EID);

    FILE* fp = fopen(event_info_fname, "r");
    if (fp == NULL) {
        return FALSE;
    }

    char file_UID[UID_LENGTH + 1];
    if (fscanf(fp, "%6s", file_UID) != 1) {
        fclose(fp);
        return FALSE;
    }
    fclose(fp);

    if (strcmp(UID, file_UID) == 0) {
        return TRUE;
    }
    return FALSE;
}

int is_event_sold_out(char* EID){
    char event_info_fname[30];
    char reservations_fname[30];
    sprintf(event_info_fname, "EVENTS/%s/START_%s.txt", EID, EID);
    sprintf(reservations_fname, "EVENTS/%s/RES_%s.txt", EID, EID);

    FILE* fp = fopen(event_info_fname, "r");
    if (fp == NULL) {
        return FALSE;
    }

    char seat_count_str[SEAT_COUNT_LENGTH + 1];
    if (fscanf(fp, "%*s %*s %*s %3s", seat_count_str) != 1) {
        fclose(fp);
        return FALSE;
    }
    fclose(fp);

    int seat_count = atoi(seat_count_str);
    int reserved_seats = 0;

    fp = fopen(reservations_fname, "r");
    if (fp != NULL) {
        fscanf(fp, "%d", &reserved_seats);
        fclose(fp);
    }

    if (reserved_seats >= seat_count) {
        return TRUE;
    }
    return FALSE;
}

int is_event_past(char* EID){
    char event_info_fname[30];
    sprintf(event_info_fname, "EVENTS/%s/START_%s.txt", EID, EID);

    FILE* fp = fopen(event_info_fname, "r");
    if (fp == NULL) {
        return FALSE;
    }

    char date_str[11];  // DD-MM-YYYY
    char time_str[6];   // HH:MM
    // Format: UID event_name filename seat_count date time
    if (fscanf(fp, "%*s %*s %*s %*s %10s %5s", date_str, time_str) != 2) {
        fclose(fp);
        return FALSE;
    }
    fclose(fp);

    // Parse date: DD-MM-YYYY HH:MM
    int day, month, year, hour, minute;
    if (sscanf(date_str, "%d-%d-%d", &day, &month, &year) != 3 ||
        sscanf(time_str, "%d:%d", &hour, &minute) != 2) {
        return FALSE;
    }

    // Get current time
    time_t now = time(NULL);
    struct tm* current_time = localtime(&now);

    // Create a struct tm for the event time
    struct tm event_tm = {0};
    event_tm.tm_year = year - 1900;  // years since 1900
    event_tm.tm_mon = month - 1;     // months are 0-11
    event_tm.tm_mday = day;
    event_tm.tm_hour = hour;
    event_tm.tm_min = minute;
    event_tm.tm_sec = 0;
    event_tm.tm_isdst = -1;           // auto-detect DST

    time_t event_time = mktime(&event_tm);
    if (event_time == -1) {
        return FALSE;
    }

    // Compare times
    if (event_time < now) {
        return TRUE;  // Event is in the past
    }
    return FALSE;    // Event is in the future
}

int read_event_start_file(char* EID, char* event_name, char* event_date) {
    
    char start_file_path[30];
    sprintf(start_file_path, "EVENTS/%s/START_%s.txt", EID, EID);

    FILE* fp = fopen(start_file_path, "r");
    if (fp == NULL) return ERROR;

    char date_str[11];  // DD-MM-YYYY
    char time_str[6];   // HH:MM
    // Format: UID event_name filename seat_count date time
    if (fscanf(fp, "%*s %10s %*s %*s %10s %5s", event_name, date_str, time_str) != 3) {
        fclose(fp);
        return ERROR;
    }

    fclose(fp);
    // Combine date and time into event_date
    snprintf(event_date, EVENT_DATE_LENGTH + 1, "%s %s", date_str, time_str);
    return SUCCESS;
}

int create_eid_dir (int EID){
    char EID_dirname[15];
    char RES_dirname[25];
    char DESC_dirname[25];
    int ret;

    if (EID < 1 || EID > MAX_EVENTS) return ERROR;

    sprintf(EID_dirname, "EVENTS/%03d", EID);

    ret = mkdir(EID_dirname, 0700);
    if (ret == -1) {
        if (errno == EEXIST) return DIR_ALREADY_EXISTS;
        return ERROR;   
    }

    snprintf(RES_dirname, sizeof(RES_dirname), "EVENTS/%03d/RESERVATIONS", EID);
    ret = mkdir(RES_dirname, 0700);
    if (ret == -1){
        rmdir(EID_dirname);
        return ERROR;
    }
    
    snprintf(DESC_dirname, sizeof(DESC_dirname), "EVENTS/%03d/DESCRIPTION", EID);
    ret = mkdir(DESC_dirname, 0700);
    if (ret == -1){
        rmdir(RES_dirname);
        rmdir(EID_dirname);
        return ERROR;
    }
    return SUCCESS;
}


