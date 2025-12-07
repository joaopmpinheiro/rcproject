#include "../../include/constants.h"
#include "../../include/utils.h"
#include "../../include/globals.h"

int select_handler() {
    int max_fd = settings.udp_socket > settings.tcp_socket ? settings.udp_socket : settings.tcp_socket;
    settings.temp_fds = settings.read_fds;

    if (select(max_fd + 1, &settings.temp_fds, NULL, NULL, &settings.timeout) < 0) {
        if (settings.verbose) {
            perror("Select failed");
        }
        return -1;
    }
    return 0;
}

void udp_connection() {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // copys the data from the UDP socket into buffer
    ssize_t len = recvfrom(settings.udp_socket, buffer, sizeof(buffer) - 1, 0,
                            (struct sockaddr *)&client_addr, &addr_len);

    // if the buffer is not empty
    if (len > 0) {
        // add \0 to be used as a string
        buffer[len] = '\0';

        // create a new request to be used by handle_request
        // TODO: implement threading for UDP requests
        Request req = {.client_addr = client_addr, .addr_len = addr_len, .is_tcp = 0};
        strncpy(req.buffer, buffer, sizeof(req.buffer));
        handle_request(&req);
    } 
}


void send_udp_response(const char* message, struct sockaddr_in* client_addr, socklen_t addr_len, int udp_socket) {
    // TODO: will need a lock
    sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *)client_addr, addr_len);
}

void tcp_connection() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_socket = accept(settings.tcp_socket, (struct sockaddr *)&client_addr, &addr_len);
    /* if (client_socket >= 0) {
        Task task = {.client_socket = client_socket, .client_addr = client_addr, .addr_len = addr_len, .is_tcp = 1};
        task_queue_push(&task_queue, task);
    } */
}