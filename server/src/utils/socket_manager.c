#include "../../include/constants.h"
#include "../../include/utils.h"
#include "../../include/globals.h"

int select_handler() {
    int max_fd = set.udp_socket > set.tcp_socket ? set.udp_socket : set.tcp_socket;
    set.temp_fds = set.read_fds;

    if (select(max_fd + 1, &set.temp_fds, NULL, NULL, &set.timeout) < 0) {
        if (set.verbose) perror("Select failed");
        return ERROR;
    }
    return SUCCESS;
}

// TODO: é preciso confirmar se deu porcaria?
void send_udp_response(const char* message, Request *req) {
    // TODO: will need a lock
    sendto(set.udp_socket, message, strlen(message), 0,\
            (struct sockaddr *)&req->client_addr, req->addr_len);
}

void udp_connection() {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Copys the data from the UDP socket into buffer
    ssize_t received_bytes = recvfrom(set.udp_socket, buffer, sizeof(buffer) - 1, 0,
                            (struct sockaddr *)&client_addr, &addr_len);
    
    // FIXME: isto pode/deve se fazer??
    if (received_bytes < 0) {
        if (set.verbose) printf(stderr, "UDP Receive failed");
        return;
    }
    // if the buffer is not empty
    if (received_bytes > 0) {
        // add \0 to be used as a string
        buffer[received_bytes] = '\0';

        // create a new request to be used by handle_request
        // TODO: implement threading for UDP requests
        Request req = {.client_addr = client_addr, .addr_len = addr_len, .is_tcp = 0};
        strncpy(req.buffer, buffer, sizeof(req.buffer));
        handle_udp_request(&req);
    } 
}    

void tcp_connection() {
    char buffer[BUFFER_SIZE];   
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Accept the incoming TCP connection, creating a new socket for this client
    int client_socket = accept(set.tcp_socket, (struct sockaddr *)&client_addr, &addr_len);

    // FIXME: isto pode/deve se fazer??
    if (client_socket < 0) {
        if (set.verbose) printf(stderr, "TCP Accept failed");
        return;
    }

    ssize_t bytes_read = 0;
    ssize_t n;
    while (bytes_read < BUFFER_SIZE - 1) {
        n = read(client_socket, buffer + bytes_read, BUFFER_SIZE - 1 - bytes_read);
        if (n <= 0) break; // Error or connection closed
        bytes_read += n;
        if (buffer[bytes_read - 1] == '\n') break; // End of message
    }
    buffer[bytes_read] = '\0'; // Null-terminate the string

    if (bytes_read > 0) {
        // create a new request to be used by handle_request
        Request req = {.client_socket = client_socket, .client_addr = client_addr, .addr_len = addr_len, .is_tcp = 1};
        strncpy(req.buffer, buffer, sizeof(req.buffer));
        handle_tcp_request(&req);
    }
    /* if (client_socket >= 0) {
        Task task = {.client_socket = client_socket, .client_addr = client_addr, .addr_len = addr_len, .is_tcp = 1};
        task_queue_push(&task_queue, task);
    } */
}