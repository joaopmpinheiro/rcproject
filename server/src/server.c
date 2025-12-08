#include "../include/constants.h"
#include "../include/globals.h"
#include "../include/utils.h"

Settings set = {0};
UserNode *users = NULL;
EventNode *events = NULL;

void clean_server() {
    // Free users
    UserNode *current_user = users;
    while (current_user != NULL) {
        UserNode *temp = current_user;
        current_user = current_user->next;
        free(temp);
    }

    // Free events
    EventNode *current_event = events;
    while (current_event != NULL) {
        EventNode *temp = current_event;
        current_event = current_event->next;
        free(temp);
    }
}

// POSIX async-signal safe functions
void sig_detected(int signum) {
    clean_server();
    close(set.udp_socket);
    close(set.tcp_socket);
    _exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[]) {
    signal(SIGINT, sig_detected);
    signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE

    parse_arguments(argc, argv);

    server_setup();

    while(1){
        if (select_handler() == ERROR) {
            // Something went wrong
            continue;
        }

        // Check for UDP connection
        if (FD_ISSET(set.udp_socket, &set.temp_fds)) {
            fprintf(stderr, "Received UDP connection\n");
            udp_connection();
            // One of the threads will handle the request, check handle_task() in threads.c
        }

        // Check for TCP connection
        if (FD_ISSET(set.tcp_socket, &set.temp_fds)) {
            tcp_connection();
            // One of the threads will handle the request, check handle_task() in threads.c
        }
    }


    return 0;
}