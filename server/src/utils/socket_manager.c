#include "../../include/utils.h"
#include "../../include/globals.h"

int select_handler() {
    int max_fd = set.udp_socket > set.tcp_socket ? set.udp_socket : set.tcp_socket;
    set.temp_fds = set.read_fds;

    if (select(max_fd + 1, &set.temp_fds, NULL, NULL, &set.timeout) < 0) {
        // FIXME TODO: ns se tá certo
        server_log("Select error");
        return ERROR;
    }
    return SUCCESS;
}

void send_udp_response(const char* message, Request *req) {
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
        // FIX ME TODO: ns se tá certo
        server_log("UDP Receive failed");
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
    char request_type[4];   
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Accept the incoming TCP connection, creating a new socket for this client
    int client_socket = accept(set.tcp_socket, (struct sockaddr *)&client_addr, &addr_len);

    // FIXME: isto pode/deve se fazer??
    if (client_socket < 0) {
        server_log("TCP Accept failed");
        return;
    }

    // Read only the 3-letter command using the helper that handles delimiters
    ssize_t cmd_len = tcp_read_field(client_socket, request_type, 3);
    if (cmd_len <= 0) {
        // é supost printar aqui tbm? FIXME TODO
        server_log("TCP Read failed or connection closed");
        close(client_socket);
        return;
    }

    // create a new request to be used by handle_request
    Request req = {.client_socket = client_socket, .client_addr = client_addr, .addr_len = addr_len, .is_tcp = 1};
    strncpy(req.buffer, request_type, sizeof(req.buffer));
    handle_tcp_request(&req);
    /* if (client_socket >= 0) {
        Task task = {.client_socket = client_socket, .client_addr = client_addr, .addr_len = addr_len, .is_tcp = 1};
        task_queue_push(&task_queue, task);
    } */
}