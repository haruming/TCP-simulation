#include "tcp.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    int fd;
    if (argc > 2) {
        /* send SYN */
        /* recvfrom SYN/ACK */
        /* send ACK */
        fd = cli_connect(argv[1], (short)atoi(argv[2]));
        /* connection fail */
        if (fd < 0) {
            perror("client fail to connect to the server");
        }
        printf("Complete the 3 way handshake\n");

        /* recvfrom FIN -> Close Wait(passive close) */
        receive_packet(fd);
        /* immediately send ACK */
        if (isfin(fd)) {
            /* finish final processing, then send FIN -> LAST_ACK */
            /* recvfrom ACK -> CLOSED */
            close(fd);
        }
        printf("Complete the four way hand waves\n");
    } else {
        /* invalid arguments */
    }
}