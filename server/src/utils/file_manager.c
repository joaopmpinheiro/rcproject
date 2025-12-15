#include "../../include/globals.h"
#include "../../common/verifications.h"


/**
 * @brief Checks if the description file exists and returns its size.
 * 
 * @param fname 
 * @return int - size of the file or EMPTY_FILE if it doesn't exist or if it's empty.
 */
int check_file(char *fname){
    struct stat filestat;
    int ret_stat;

    ret_stat = stat(fname, &filestat);
    if (ret_stat == -1 || filestat.st_size == 0) return EMPTY_FILE;
    return filestat.st_size;
}

int file_exists(const char* filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0) ? VALID : INVALID;
}

int dir_exists(const char* path) {
    struct stat info;

    if (stat(path, &info) != 0) return INVALID;
    else if (S_ISDIR(info.st_mode)) return VALID;
    return INVALID;
}

char* read_file(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return NULL; // File couldn't be opened
    }

    // Go to the end to determine file size
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    rewind(fp); // Go back to the beginning

    if (filesize == 0) {
        fclose(fp);
        return NULL; // Empty file
    }

    // Allocate memory for file content (+1 for null terminator)
    char* buffer = malloc(filesize + 1);
    if (buffer == NULL) {
        fclose(fp);
        return NULL; // Memory allocation failed
    }

    // Read the file into the buffer
    size_t read_size = fread(buffer, 1, filesize, fp);
    fclose(fp);

    if (read_size != (size_t)filesize) {
        free(buffer);
        return NULL; // Reading error
    }

    buffer[filesize] = '\0'; // Null-terminate the string
    return buffer;
}

/**
 * @brief Finds the first available EID (001-999) by checking the EVENTS directory.
 * 
 * @param eid_str Buffer to store the 3-digit EID string (e.g., "001", "042")
 * @return int SUCCESS if an available EID was found, ERROR if all EIDs are taken or error occurs
 */
int find_available_eid(char* eid_str) {
    if (eid_str == NULL) return ERROR;

    DIR* dir = opendir("EVENTS");
    if (dir == NULL) {
        // EVENTS directory doesn't exist, so EID 001 is available
        snprintf(eid_str, 4, "001");
        return SUCCESS;
    }

    // Create a set to track which EIDs are taken
    int taken[1000] = {0};  // Index 0-999, 0=not taken, 1=taken

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        // Check if entry is a directory and is a valid 3-digit number
        struct stat st;
        if (stat(entry->d_name, &st) == 0 && S_ISDIR(st.st_mode)) {
            if (strlen(entry->d_name) == 3 && is_number(entry->d_name)) {
                int eid = atoi(entry->d_name);
                if (eid >= 1 && eid <= 999) {
                    taken[eid] = 1;
                }
            }
        }
    }
    closedir(dir);

    // Find the first available EID
    for (int i = 1; i <= 999; i++) {
        if (taken[i] == 0) {
            snprintf(eid_str, 4, "%03d", i);
            return SUCCESS;
        }
    }

    // All EIDs are taken
    return ERROR;
}

/**
 * @brief Writes event metadata to START_{EID}.txt file in the event directory.
 * 
 * @param eid Event ID (3-digit string, e.g., "001")
 * @param uid User ID (6-digit string)
 * @param event_name Event name
 * @param desc_fname Description filename
 * @param event_attend Total attendance/seats (as string)
 * @param event_date Event date and time (DD-MM-YYYY HH:MM)
 * @return int SUCCESS if file was written successfully, ERROR otherwise
 */
int write_event_start_file(const char* eid, const char* uid, const char* event_name,
                           const char* desc_fname, const char* event_attend,
                           const char* event_date) {
    if (eid == NULL || uid == NULL || event_name == NULL || desc_fname == NULL || 
        event_attend == NULL || event_date == NULL) {
        return ERROR;
    }

    // Create file path: EVENTS/{EID}/START_{EID}.txt
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "EVENTS/%s/START_%s.txt", eid, eid);

    FILE* fp = fopen(file_path, "w");
    if (fp == NULL) {
        return ERROR;
    }

    // Write single line: UID event_name desc_fname event_attend event_date
    int ret = fprintf(fp, "%s %s %s %s %s\n", uid, event_name, desc_fname, 
                      event_attend, event_date);
    
    fclose(fp);

    return (ret > 0) ? SUCCESS : ERROR;
}

/**
 * @brief Creates or updates RES_{EID}.txt file with the number of reserved seats.
 * 
 * If the file doesn't exist, it creates it with reserved_seats = 0.
 * If the file exists, it reads the current value and increments it by reserved_seats.
 * 
 * @param eid Event ID (3-digit string, e.g., "001")
 * @param reserved_seats Number of seats to add to the reservation count
 * @return int SUCCESS if file was created/updated successfully, ERROR otherwise
 */
int update_reservations_file(const char* eid, int reserved_seats) {
    if (eid == NULL) return ERROR;

    // Create file path: EVENTS/{EID}/RES_{EID}.txt
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "EVENTS/%s/RES_%s.txt", eid, eid);

    int current_reservations = 0;

    // Try to read existing reservations
    FILE* fp = fopen(file_path, "r");
    if (fp != NULL) {
        if (fscanf(fp, "%d", &current_reservations) != 1) {
            current_reservations = 0;
        }
        fclose(fp);
    }

    // Update with new reservation count
    int new_reservations = current_reservations + reserved_seats;

    // Write updated count
    fp = fopen(file_path, "w");
    if (fp == NULL) {
        return ERROR;
    }

    int ret = fprintf(fp, "%d\n", new_reservations);
    fclose(fp);

    return (ret > 0) ? SUCCESS : ERROR;
}

/**
 * @brief Creates a DESCRIPTION directory inside the event directory and stores the description file.
 * 
 * @param eid Event ID (3-digit string, e.g., "001")
 * @param file_name Name of the description file
 * @param file_size Size of the file in bytes
 * @param file_content Content of the file
 * @return int SUCCESS if directory and file were created, ERROR otherwise
 */
int write_description_file(const char* eid, const char* file_name, size_t file_size, const char* file_content) {
    if (eid == NULL || file_name == NULL || file_content == NULL) {
        return ERROR;
    }

    // Create directory path: EVENTS/{EID}/DESCRIPTION
    char dir_path[256];
    snprintf(dir_path, sizeof(dir_path), "EVENTS/%s/DESCRIPTION", eid);

    // Create the DESCRIPTION directory if it doesn't exist
    if (mkdir(dir_path, 0755) == -1 && errno != EEXIST) {
        return ERROR;
    }

    // Create full file path: EVENTS/{EID}/DESCRIPTION/{file_name}
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "EVENTS/%s/DESCRIPTION/%s", eid, file_name);

    // Write the file content
    FILE* fp = fopen(file_path, "wb");
    if (fp == NULL) {
        return ERROR;
    }

    size_t written = fwrite(file_content, 1, file_size, fp);
    fclose(fp);

    return (written == file_size) ? SUCCESS : ERROR;
}


