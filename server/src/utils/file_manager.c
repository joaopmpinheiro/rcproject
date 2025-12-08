#include "../../include/constants.h"
#include "../../include/globals.h"



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

    if (read_size != filesize) {
        free(buffer);
        return NULL; // Reading error
    }

    buffer[filesize] = '\0'; // Null-terminate the string
    return buffer;
}