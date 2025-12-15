#include "common.h"

int tcp_send_message(int fd, char* message) {
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

int tcp_send_file(int fd, char* file_name) {
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

// TODO FIXME: olhadela wtf void* buf typecast ?!?!?!
int tcp_read(int fd, void* buf, size_t len) {
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

// Reliable helper that keeps writing until everything is sent
int tcp_write(int fd, const char* buffer, size_t length) {
    size_t total = 0;
    while (total < length) {
        ssize_t n = write(fd, buffer + total, length - total);
        if (n <= 0) return ERROR;
        total += (size_t)n;
    }
    return SUCCESS;
}

// Helper function to read a single space-delimited field from TCP socket
int tcp_read_field(int fd, char* buffer, size_t max_len) {
    size_t i = 0;
    char c;
    ssize_t n;

    // Skip leading space if present
    n = read(fd, &c, 1);
    if (n <= 0) return ERROR;
    if (c != ' ') {
        buffer[i++] = c;
    }

    // Read until space or newline
    while (i < max_len) {
        n = read(fd, &c, 1);
        if (n <= 0) return ERROR;
        if (c == ' ') {
            buffer[i] = '\0';
            return SUCCESS;
        } else if (c == '\n') {
            buffer[i] = '\0';
            return EOM;
        }
        buffer[i++] = c;
    }
    buffer[max_len] = '\0';
    return SUCCESS;
}

/*int read_tcp_file(int fd, char* file_name, long file_size) {
    FILE* file = fopen(file_name, "wb");
    if (!file) {
        perror("ERROR: Failed to open file for writing");
        return ERROR;
    }

    char buffer[TCP_BUFFER_SIZE];
    long total_received = 0;
    ssize_t n;

    while (total_received < file_size) {
        size_t to_read = (file_size - total_received) < TCP_BUFFER_SIZE ? (file_size - total_received) : TCP_BUFFER_SIZE;
        n = read(fd, buffer, to_read);
        if (n <= 0) {
            perror("ERROR: Failed to read file data from socket");
            fclose(file);
            return ERROR;
        }
        fwrite(buffer, 1, n, file);
        total_received += n;
    }

    fclose(file);
    return SUCCESS;
}

int read_tcp_argument(int tcp_fd, char* argument, size_t arg_length) {
    // Read argument from TCP socket
    if (read_tcp(tcp_fd, argument, arg_length + 1) != SUCCESS) {
        return ERROR;
    }
    // Remove trailing newline if present
    size_t len = strlen(argument);
    if (len > 0 && argument[len - 1] == '\n') {
        argument[len - 1] = '\0';
    }
    return SUCCESS;
}
