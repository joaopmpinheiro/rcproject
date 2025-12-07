#include "../include/constants.h"
#include "../include/globals.h"
#include "../include/utils.h"

Set settings = {0};
UserNode *users = NULL;
EventNode *events = NULL;


int main(int argc, char *argv[]) {
    /*signal(SIGINT, sig_detected); */ // TODO
     signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE

    parse_arguments(argc, argv);

    server_setup();

    while(1){
        if (select_handler() < 0) {
            // Something went wrong
            continue;
        }

        // Check for UDP connection
        if (FD_ISSET(settings.udp_socket, &settings.temp_fds)) {
            fprintf(stderr, "Received UDP connection\n");
            udp_connection();
            // One of the threads will handle the request, check handle_task() in threads.c
        }

        // Check for TCP connection
        if (FD_ISSET(settings.tcp_socket, &settings.temp_fds)) {
            tcp_connection();
            // One of the threads will handle the request, check handle_task() in threads.c
        }
    }


    return 0;
}