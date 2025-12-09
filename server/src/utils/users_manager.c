#include "../../include/constants.h"
#include "../../include/globals.h"
#include "../../include/utils.h"


int user_exists(char* UID){
    char UID_dirname[20];
    sprintf(UID_dirname, "USERS/%s", UID);
    return dir_exists(UID_dirname);
}

int create_USER_dir (char* UID){
    char UID_dirname[32];
    char created_dirname[32];
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

    snprintf(created_dirname, sizeof(created_dirname), "USERS/%s/EVENTS", UID);
    ret = mkdir(created_dirname, 0700);
    if (ret == -1){
        rmdir(UID_dirname);
        return ERROR;
    }
    return SUCCESS;
}

int create_new_user(char* UID, char* password){
    int ret;

    ret = create_USER_dir(UID);
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