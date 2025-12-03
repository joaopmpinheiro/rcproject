#include "../include/constants.h"
#include "../include/globals.h"
#include "../include/utils.h"

Set settings = {0};


int main(int argc, char *argv[]) {
/*     signal(SIGINT, sig_detected);
 */    signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE

    parse_arguments(argc, argv);

    server_setup();


    return 0;
}