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
    buffer[i] = '\0';
    return SUCCESS;
}

int tcp_read_file(int fd, char* file_name, long file_size) {
    FILE* file = fopen(file_name, "wb");
    if (!file) {
        // TODO FIXME olhar para isto
        // perror("ERROR: Failed to open file for writing");
        return ERROR;
    }

    char buffer[TCP_BUFFER_SIZE];
    long total_received = 0;
    ssize_t n;

    while (total_received < file_size) {
        size_t to_read = (file_size - total_received) < TCP_BUFFER_SIZE ?
                         (file_size - total_received) : TCP_BUFFER_SIZE;
        n = read(fd, buffer, to_read);
        // FIXME TODO: ver se é melhor separar n==0 e n<0 para disconnect e erro respetivamente
        if (n == 0) break;
        if (n < 0) {
            fclose(file);
            return ERROR;
        }
        fwrite(buffer, 1, n, file);
        total_received += n;
    }

    fclose(file);   
    return SUCCESS;
}




// ---------------- RequestType ----------------

// RequestType -> Human-readable command name
const char* command_to_str(RequestType command) {
    switch (command) {
        case LOGIN: return "Login";
        case CHANGEPASS: return "Change password";
        case UNREGISTER: return "Unregister";
        case LOGOUT: return "Logout";
        case EXIT: return "Exit";
        case CREATE: return "Create";
        case CLOSE: return "Close";
        case MYEVENTS: return "My events";
        case LIST: return "List";
        case SHOW: return "Show";
        case RESERVE: return "Reserve";
        case MYRESERVATIONS: return "My reservations";
        default: return "Unknown";
    }
}

// USER -> SERVER
const char* get_command_request(RequestType command) {
    switch (command) {
        case LOGIN: return "LIN";
        case CHANGEPASS: return "CPS";
        case UNREGISTER: return "UNR";
        case LOGOUT: return "LOU";
        case CREATE: return "CRE";
        case CLOSE: return "CLS";
        case MYEVENTS: return "LME";
        case LIST: return "LST";
        case SHOW: return "SED";
        case RESERVE: return "RID";
        case MYRESERVATIONS: return "LMR";
        default: return "UNK";
    }
}

RequestType identify_command_request(char* command_buff) {
    if (strncmp(command_buff, "LIN", 3) == 0) return LOGIN;
    if (strncmp(command_buff, "CPS", 3) == 0) return CHANGEPASS;
    if (strncmp(command_buff, "UNR", 3) == 0) return UNREGISTER;
    if (strncmp(command_buff, "LOU", 3) == 0) return LOGOUT;
    if (strncmp(command_buff, "CRE", 3) == 0) return CREATE;
    if (strncmp(command_buff, "CLS", 3) == 0) return CLOSE;
    if (strncmp(command_buff, "LME", 3) == 0) return MYEVENTS;
    if (strncmp(command_buff, "LST", 3) == 0) return LIST;
    if (strncmp(command_buff, "SED", 3) == 0) return SHOW;
    if (strncmp(command_buff, "RID", 3) == 0) return RESERVE;
    if (strncmp(command_buff, "LMR", 3) == 0) return MYRESERVATIONS;
    else return UNKNOWN;
}


// SERVER -> USER
// "RXX" -> RequestType
RequestType identify_command_response(char* command) {
    if (strcmp(command, "RLI") == 0) return LOGIN;
    if (strcmp(command, "RCP") == 0) return CHANGEPASS;
    if (strcmp(command, "RUR") == 0) return UNREGISTER;
    if (strcmp(command, "RLO") == 0) return LOGOUT;
    if (strcmp(command, "REX") == 0) return EXIT;
    if (strcmp(command, "RCE") == 0) return CREATE;
    if (strcmp(command, "RCL") == 0) return CLOSE;
    if (strcmp(command, "RME") == 0) return MYEVENTS;
    if (strcmp(command, "RLS") == 0) return LIST;
    if (strcmp(command, "RSE") == 0) return SHOW;
    if (strcmp(command, "RRI") == 0) return RESERVE;
    if (strcmp(command, "RMR") == 0) return MYRESERVATIONS;
    if(strcmp(command, "ERR") == 0) return ERROR_REQUEST;
    return UNKNOWN;
}

// SERVER -> USER
// RequestType -> "RXX"
const char* get_command_response_code(RequestType command) {
    switch (command) {
        case LOGIN: return "RLI";
        case CHANGEPASS: return "RCP";
        case UNREGISTER: return "RUR";
        case LOGOUT: return "RLO";
        case EXIT: return "REX";
        case CREATE: return "RCE";
        case CLOSE: return "RCL";
        case MYEVENTS: return "RME";
        case LIST: return "RLS";
        case SHOW: return "RSE";
        case RESERVE: return "RRI";
        case MYRESERVATIONS: return "RMR";
        case ERROR_REQUEST: return "ERR";
        default: return "UNK";
    }
}


// ---------------- ReplyStatus ----------------
// SERVER -> USER
// "XXX" -> ReplyStatus
ReplyStatus identify_status_code(const char* status) {
    if (strcmp(status, "ERR") == 0) return STATUS_ERROR;
    if (strcmp(status, "OK") == 0) return STATUS_OK;
    if (strcmp(status, "NOK") == 0) return STATUS_NOK;
    if (strcmp(status, "REG") == 0) return STATUS_REGISTERED;
    if (strcmp(status, "NLG") == 0) return STATUS_NOT_LOGGED_IN;
    if (strcmp(status, "WRP") == 0) return STATUS_WRONG_PASSWORD;
    if (strcmp(status, "UNR") == 0) return STATUS_USER_NOT_REGISTERED;
    if (strcmp(status, "NID") == 0) return STATUS_USER_NOT_FOUND;
    if (strcmp(status, "NOE") == 0) return STATUS_NO_EVENT_ID;
    if (strcmp(status, "SLD") == 0) return STATUS_EVENT_SOLD_OUT;
    if (strcmp(status, "PST") == 0) return STATUS_PAST_EVENT;
    if (strcmp(status, "CLS") == 0) return STATUS_EVENT_CLOSED;
    if (strcmp(status, "ACC") == 0) return STATUS_EVENT_RESERVED;
    if (strcmp(status, "REJ") == 0) return STATUS_EVENT_RESERVATION_REJECTION;
    if (strcmp(status, "CLO") == 0) return STATUS_EVENT_CLOSE_CLOSED;
    return STATUS_UNEXPECTED_RESPONSE;
}

// SERVER -> USER
// ReplyStatus -> "XXX"
const char* get_status_code(ReplyStatus status) {
    switch (status) {
        case STATUS_ERROR: return "ERR";
        case STATUS_OK: return "OK";
        case STATUS_NOK: return "NOK";
        case STATUS_REGISTERED: return "REG";
        case STATUS_NOT_LOGGED_IN: return "NLG";
        case STATUS_WRONG_PASSWORD: return "WRP";
        case STATUS_USER_NOT_REGISTERED: return "UNR";
        case STATUS_USER_NOT_FOUND: return "NID";
        case STATUS_NO_EVENT_ID: return "NOE";
        case STATUS_EVENT_SOLD_OUT: return "SLD";
        case STATUS_PAST_EVENT: return "PST";
        case STATUS_EVENT_CLOSED: return "CLS";
        case STATUS_EVENT_RESERVED: return "ACC";
        case STATUS_EVENT_RESERVATION_REJECTION: return "REJ";
        case STATUS_EVENT_CLOSE_CLOSED: return "CLO";
        default: return "UNK";
    }
}
