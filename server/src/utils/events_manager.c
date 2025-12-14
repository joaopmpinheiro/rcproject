#include "../../include/constants.h"
#include "../../include/globals.h"
#include "../../include/utils.h"

int event_exists(char* EID){
    char EID_dirname[20];
    sprintf(EID_dirname, "EVENTS/%s", EID);
    return dir_exists(EID_dirname);
}

int create_eid_dir (int EID){
    char EID_dirname[15];
    char RES_dirname[25];
    char DESC_dirname[25];
    int ret;

    if (EID < 1 || EID > MAX_EVENTS) return ERROR;

    sprintf(EID_dirname, "EVENTS/%03d", EID);

    ret = mkdir(EID_dirname, 0700);
    if (ret == -1) return ERROR;

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


