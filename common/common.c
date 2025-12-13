#include "common.h"

int send_tcp_message(int fd, char* message) {
    size_t total_written = 0;
    size_t message_length = strlen(message);

    while (total_written < message_length) {
        ssize_t bytes_written = write(fd, message + total_written, message_length - total_written);
        if (bytes_written < 0) {
            perror("ERROR: Failed to send message");
            return ERROR;
        }
        total_written += bytes_written;
    }
    return SUCCESS;
}


int send_tcp_file(int fd, char* file_name) {
    // Open file for byte reading
    FILE* file = fopen(file_name, "rb");
    if (!file) {
        perror("ERROR: Failed to open file");
        return ERROR;
    }
    // Read and send file in chunks of 1024 bytes
    char buffer[TCP_BUFFER_SIZE];
    size_t bytes_read;
    ssize_t bytes_sent;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        size_t total_sent = 0;
        while (total_sent < bytes_read) {
            bytes_sent = write(fd, buffer + total_sent, bytes_read - total_sent);
            if (bytes_sent < 0) {
                perror("ERROR: Failed to send file data");
                fclose(file);
                return ERROR;
            }
            total_sent += bytes_sent;
        }
    }
    bytes_sent = write(fd,"\n",1); // Indicate end of file transfer
    if (bytes_sent < 0) {
        perror("ERROR: Failed to send end of file indicator");
    }
    fclose(file);
    return SUCCESS;
}

int read_tcp(int fd, void* buf, size_t len) {
    ssize_t bytes_read = 0;
    ssize_t n;
    while (bytes_read < len - 1) {
        n = read(fd, buf + bytes_read, len - 1 - bytes_read);
        if (n <= 0) break; // Connection closed
        if (n < 0) return ERROR; // Read error
        bytes_read += n;
        //TODO: confirmar que nao pode acabar sem \n
        if (((char*)buf)[bytes_read - 1] == '\n') break; // End of message 
    }
    ((char*)buf)[bytes_read] = '\0'; // Null-terminate the string 
    return SUCCESS;
}
