#include "tcp.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    int fd;
    /* recvfrom SYN */
    /* send SYN/ACK */
    /* recvfrom ACK */
    fd = srv_listen(SERVER_PORT);
    if (fd < 0) {
        perror("server fail to listen to the port");
    }
    printf("Server complete the 3 way handshakes\n");
    
    /* send FIN -> FIN_WAIT_1 */
    /* recvfrom ACK -> FIN_WAIT_2 */
    close(fd);
    /* recvfrom FIN -> TIME_WAIT */
    receive_packet(fd);
    /* send final ACK -> CLOSED */
    if (isfin(fd)) {
        printf("Server complete the four way hand waves\n");
    }
    return 0;
}