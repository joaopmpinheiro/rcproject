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

    // copia o conteudo enviado para o UDP para dentro do buffer
    ssize_t len = recvfrom(settings.udp_socket, buffer, sizeof(buffer) - 1, 0,
                            (struct sockaddr *)&client_addr, &addr_len);

    // se o buffer nao estiver vazio
    if (len > 0) {
        //acrecentar \0 para poder ser usado como string
        buffer[len] = '\0';

        // criar um novo request para poder ser usado yay
        Request req = {.client_addr = client_addr, .addr_len = addr_len, .is_tcp = 0};
        strncpy(req.buffer, buffer, sizeof(req.buffer));
        manage_UDP_request(&req);
        task_queue_push(&task_queue, req);
    } 
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