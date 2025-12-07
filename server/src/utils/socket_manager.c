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

void udp_connection() {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // copys the data from the UDP socket into buffer
    ssize_t len = recvfrom(set.udp_socket, buffer, sizeof(buffer) - 1, 0,
                            (struct sockaddr *)&client_addr, &addr_len);

    // if the buffer is not empty
    if (len > 0) {
        // add \0 to be used as a string
        buffer[len] = '\0';

        // create a new request to be used by handle_request
        // TODO: implement threading for UDP requests
        Request req = {.client_addr = client_addr, .addr_len = addr_len, .is_tcp = 0};
        strncpy(req.buffer, buffer, sizeof(req.buffer));
        handle_UDP_request(&req);
    } 
}


void send_udp_response(const char* message, Request *req) {
    // TODO: will need a lock
    sendto(set.udp_socket, message, strlen(message), 0,\
            (struct sockaddr *)&req->client_addr, req->addr_len);
}

void tcp_connection() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_socket = accept(set.tcp_socket, (struct sockaddr *)&client_addr, &addr_len);
    /* if (client_socket >= 0) {
        Task task = {.client_socket = client_socket, .client_addr = client_addr, .addr_len = addr_len, .is_tcp = 1};
        task_queue_push(&task_queue, task);
    } */
}