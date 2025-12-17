#include "../../include/globals.h"
#include "../../include/utils.h"
#include <time.h>

int event_exists(char* EID){
    char EID_dirname[20];
    sprintf(EID_dirname, "EVENTS/%s", EID);
    return dir_exists(EID_dirname);
}

int is_event_closed(char* EID){
    char state_path[30];
    sprintf(state_path, "EVENTS/%s/END_%s.txt", EID, EID);
    return file_exists(state_path) ? SUCCESS : ERROR;
}

int is_event_creator(char* UID, char* EID){
    char event_info_fname[30];
    sprintf(event_info_fname, "EVENTS/%s/START_%s.txt", EID, EID);

    FILE* fp = fopen(event_info_fname, "r");
    if (fp == NULL) {
        return INVALID;
    }

    char file_UID[UID_LENGTH + 1];
    if (fscanf(fp, "%6s", file_UID) != 1) {
        fclose(fp);
        return INVALID;
    }
    fclose(fp);

    if (strcmp(UID, file_UID) == 0) {
        return VALID;
    }
    return INVALID;
}

int is_event_sold_out(char* EID){
    char event_info_fname[30];
    char reservations_fname[30];
    sprintf(event_info_fname, "EVENTS/%s/START_%s.txt", EID, EID);
    sprintf(reservations_fname, "EVENTS/%s/RES_%s.txt", EID, EID);

    FILE* fp = fopen(event_info_fname, "r");
    if (fp == NULL) {
        return INVALID;
    }

    char seat_count_str[SEAT_COUNT_LENGTH + 1];
    if (fscanf(fp, "%*s %*s %*s %3s", seat_count_str) != 1) {
        fclose(fp);
        return INVALID;
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
        return VALID;
    }
    return INVALID;
}

int is_event_past(char* EID){
    char event_info_fname[30];
    sprintf(event_info_fname, "EVENTS/%s/START_%s.txt", EID, EID);

    FILE* fp = fopen(event_info_fname, "r");
    if (fp == NULL) {
        return INVALID;
    }

    char date_str[11];  // DD-MM-YYYY
    char time_str[6];   // HH:MM
    // Format: UID event_name filename seat_count date time
    if (fscanf(fp, "%*s %*s %*s %*s %10s %5s", date_str, time_str) != 2) {
        fclose(fp);
        return INVALID;
    }
    fclose(fp);

    // Parse date: DD-MM-YYYY HH:MM
    int day, month, year, hour, minute;
    if (sscanf(date_str, "%d-%d-%d", &day, &month, &year) != 3 ||
        sscanf(time_str, "%d:%d", &hour, &minute) != 2) {
        return INVALID;
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
        return INVALID;
    }

    // Compare times
    if (event_time < now) {
        return VALID;  // Event is in the past
    }
    return INVALID;    // Event is in the future
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


