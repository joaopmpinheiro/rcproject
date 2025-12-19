#include "../../include/globals.h"
#include "../../include/utils.h"
#include "../../common/parser.h"


/**
 * @brief 
 * 
 * @param UID 
 * @param password 
 * @return VALID if password matches,
 * INVALID otherwise,
 * ERROR if there was a problem getting the password
 */
int verify_correct_password(char* UID, char* password){
    char stored_password[PASSWORD_LENGTH + 1];

    if (get_password(UID, stored_password) == ERROR) return ERROR;

    if (strcmp(stored_password, password) == 0) return VALID;
    return INVALID;
}

int user_exists(char* UID){
    char UID_dirname[20];
    sprintf(UID_dirname, "USERS/%s", UID);
    return dir_exists(UID_dirname);
}

int create_user (char* UID){
    char UID_dirname[32];
    char created_dirname[64];
    char reserved_dirname[64];
    int ret;

    sprintf(UID_dirname, "USERS/%s", UID);
    ret = mkdir(UID_dirname, 0700);
    if (ret == -1) return ERROR;

    snprintf(created_dirname, sizeof(created_dirname), "USERS/%s/CREATED", UID);
    ret = mkdir(created_dirname, 0700);
    if (ret == -1){
        rmdir(UID_dirname);
        return ERROR;
    }

    snprintf(reserved_dirname, sizeof(reserved_dirname), "USERS/%s/RESERVED", UID);
    ret = mkdir(reserved_dirname, 0700);
    if (ret == -1){
        rmdir(created_dirname);
        rmdir(UID_dirname);
        return ERROR;
    }
    return SUCCESS;
}

int remove_user(char* UID){
    char UID_dirname[32];
    sprintf(UID_dirname, "USERS/%s", UID);
    return remove_directory(UID_dirname);
}

int create_new_user(char* UID, char* password){
    int ret;

    ret = create_user(UID);
    if (ret == ERROR) return ERROR;

    ret = write_password(UID, password);
    if (ret == ERROR) return ERROR;

    ret = write_login(UID);
    if (ret == ERROR) return ERROR;

    return SUCCESS;
}

int is_logged_in(char* UID){
    char login_filename[35];
    sprintf(login_filename, "USERS/%s/%slogin.txt", UID, UID);
    return file_exists(login_filename);
}

int write_login(char* UID){
    char login_filename[35];
    FILE* fp;

    sprintf(login_filename, "USERS/%s/%slogin.txt", UID, UID);
    fp = fopen(login_filename, "w");
    if (fp == NULL){
        return ERROR;
    }
    fprintf(fp, "Logged in\n");
    fclose(fp);
    return SUCCESS;
}

int erase_login(char* UID){
    char login_filename[35];
    sprintf(login_filename, "USERS/%s/%slogin.txt", UID, UID);
    unlink(login_filename);
    return SUCCESS;
}

int get_password(char* UID, char* password){
    char password_filename[40];
    FILE* fp;

    sprintf(password_filename, "USERS/%s/%spassword.txt", UID, UID);
    fp = fopen(password_filename, "r");
    if (fp == NULL){
        return ERROR;
    }

    if (fread(password, 1, PASSWORD_LENGTH, fp) != PASSWORD_LENGTH) {
        fclose(fp);
        return ERROR;
    }
    password[PASSWORD_LENGTH] = '\0';
    fclose(fp);
    return SUCCESS;
}

int write_password(char* UID, char* password){
    char password_filename[40];
    FILE* fp;

    sprintf(password_filename, "USERS/%s/%spassword.txt", UID, UID);
    fp = fopen(password_filename, "w");
    if (fp == NULL){
        return ERROR;
    }
    fprintf(fp, "%s", password);
    fclose(fp);
    return SUCCESS;
}

int verify_event_file(char* event_file_name){
    if (strlen(event_file_name) != 7) return INVALID;

    // Check first 3 characters are digits
    for (int i = 0; i < 3; i++) {
        if (!isdigit((unsigned char)event_file_name[i]))
            return INVALID;
    }
    return strcmp(event_file_name + 3, ".txt") == 0 ? VALID : INVALID;
}

//TODO: melhorar esta funcao
int verify_reservation_file(char* reservation_file_name){
    if (strlen(reservation_file_name) != 27) return INVALID;

    // Check first 3 characters are digits
    for (int i = 0; i < 3; i++) {
        if (!isdigit((unsigned char)reservation_file_name[i]))
            return INVALID;
    }
    return VALID;
}

int has_events(char* UID){
    char path[32];
    snprintf(path, sizeof(path), "USERS/%s/CREATED", UID);
    return is_dir_empty(path) ? FALSE : TRUE;
}


int has_reservations(char* UID){
    char path[32];
    snprintf(path, sizeof(path), "USERS/%s/RESERVED", UID);
    return is_dir_empty(path) ? FALSE : TRUE;
}


/**
 * @brief Returns the number of reservations formatted into response.
 * 
 * @param UID 
 * @param response 
 * @param response_size 
 * @return int 
 */
int format_list_of_user_reservations(char* UID, char* response, size_t response_size) {
    char path[128];
    int count = 0;
    snprintf(path, sizeof(path), "USERS/%s/RESERVED", UID);

    struct dirent **namelist;
    int n = scandir(path, &namelist, NULL, alphasort);
    if (n < 0) {
        perror("scandir");  
        return ERROR;
    }

    int start = (n > 50) ? n - 50 : 0;

    snprintf(response, response_size, "RMR OK");

    for (int i = start; i < n; i++) {
        struct dirent *entry = namelist[i];
        
        /* Skip "." and ".." */
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            free(entry);
            continue;
        }

        char file_path[256];
        snprintf(file_path, sizeof(file_path), "%s/%s", path, entry->d_name);
        FILE *fp = fopen(file_path, "r");
        if (!fp) {
            free(entry);
            continue;
        }

        char eid[EID_LENGTH + 1] = {0};
        char reserved_seats[SEAT_COUNT_LENGTH + 1] ={0};
        char date[DAY_STR_SIZE + 1] = {0};    
        char time[TIME_LENGTH + 4] = {0};
        char file_content[128] = {0};

        if (fgets(file_content, sizeof(file_content), fp) == NULL) {
            fclose(fp);
            free(entry);
            continue;
        }
        char *cursor = file_content;
        if(get_next_arg(&cursor, eid) == ERROR ||
           get_next_arg(&cursor, reserved_seats) == ERROR ||
           get_next_arg(&cursor, date) == ERROR ||
           get_next_arg(&cursor, time) == ERROR) {
            fclose(fp);
            free(entry);
            continue;
        }
        if(!verify_eid_format(eid) ||
           !verify_reserved_seats(reserved_seats, "999")) {
            fclose(fp);
            free(entry);
            continue;
        }

        // Append reservation info to response
        char temp[64];
        snprintf(temp, sizeof(temp), " %s %s %s %s", eid, date, time, reserved_seats);
        strncat(response, temp, response_size - strlen(response) - 1);
        count++;
        fclose(fp);
        free(entry);
    }

    /* Free scandir list */
    //  for (int i = 0; i < n; i++) free(namelist[i]);
    free(namelist);
    strncat(response, "\n", response_size - strlen(response) - 1);
    return count;
}



/* int get_event_list(char *EID, EVENTLIST *list){
    char* UID
    struct dirent **filelist;
    int n_entries, eventsID, i_ent = 0;
    char dirname[55];

    sprintf(dirname, "USERS/%s/CREATED", EID);
    n_entries = scandir(dirname, &filelist, NULL, alphasort);

    events_num = 0;
    if (n_entries =< 0){
        return ERROR;
    }
    else{
        while (i_ent < n_entries){
            if (filelist[i_ent]->d_name[0] != '.'){
               memcpy(list->EID[events_num], filelist[i_ent]->d_name, 3);
               list->EID[events_num][3] = 0;
               events_num++;
            }
            free(filelist[i_ent]);
            i_ent++;
        }
        list->num_events = events_num;
        free(filelist);
    }
    return events_num;
} */