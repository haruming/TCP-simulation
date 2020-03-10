#include "tcp.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    int fd;
    fd = srv_listen(SERVER_PORT);
    if (fd < 0) {
        perror("server fail to listen to the port");
    }

    close(fd);
    receive_packet(fd);
    if (isfin(fd)) {
        printf("Complete the four way hand waves\n");
    }
    return 0;
}