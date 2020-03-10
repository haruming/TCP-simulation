#include "tcp.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    int fd;
    if (argc > 2) {
        fd = cli_connect(argv[1], (short)atoi(argv[2]));
        /* connection fail */
        if (fd < 0) {
            perror("client fail to connect to the server");
        }
        printf("Complete the 3 way handshake\n");
        receive_packet(fd);
        if (isfin(fd)) {
            close(fd);
        }
        printf("Complete the four way hand waves\n");
    } else {
        /* invalid arguments */
    }
}