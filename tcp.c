#include "tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>

/* copy the header data from packet and transform it to PSegment structure */
inline PSegment get_segment(char* pkg, char* header) {
    memcpy(header, pkg, HEADER_LEN);
    return (PSegment)header;
}

/* copy the header info into a packet  */
inline void create_pkg(char* pkg, Segment s) {
    memcpy(pkg, &s, sizeof(s));
}

/*
    server listen on a specific port
    @param port number
    @return success -> file descriptor fd
            fail -> -1
*/
int srv_listen(short port) {
    int fd;
    struct sockaddr_in srv_addr;
    struct sockaddr_in cli_addr;
    int cli_len = sizeof(cli_addr);
    srand(time(NULL));
    
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return -1;

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    inet_aton("127.0.0.1", &srv_addr.sin_addr);

    if (bind(fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) < 0) return -1;

    /* set global variables */
    glb_rtt = 200;
    glb_threshold = 65535;
    glb_mss = 512;
    printf("==========\n");
    printf("Set RTT delay = %d ms\n", glb_rtt);
    printf("Set threshold = %d bytes\n", glb_threshold);
    printf("Set MSS = %d bytes\n", glb_mss);
    printf("Buffer size = %d bytes\n", BUF_SIZE);
    
    /* server starts listening */
    printf("Server's ip is %s\n", inet_ntoa(srv_addr.sin_addr));
    printf("Server is listening on port %hu\n", port);
    printf("==========\n");
    printf("listening......\n");

    /* establish connection with client */
    
    /* wait for client's SYN packet */
    recvfrom(fd, pkg, PKG_LEN, 0, (struct sockaddr*)&cli_addr, &cli_len);
    printf("Server received a SYN packet from %s : %hu\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    pseg = get_segment(pkg, header);
    printf("\t packet (seq = %d , ack = %d)\n", pseg->seq_num, pseg->ack_num);

    if (pseg->syn) {
        /* send SYN/ACK */
        /* set data field in the sending packet */
        seg.src_port = port;
        seg.dest_port = pseg->src_port;
        seg.syn = 1;
        seg.ack = 1;
        seg.ack_num = pseg->seq_num + 1;
        seg.seq_num = (short)(rand() % 10000) + 1;
        /* copy packet to the pkg variable */
        create_pkg(pkg, seg);

        /* send to client */
        if (sendto(fd, pkg, PKG_LEN, 0, (struct sockaddr*)&cli_addr, &cli_len) == -1) return -1;
        printf("Server send a SYN/ACK packet to %s : %hu\n", inet_ntoa(cli_addr.sin_addr), pseg->dest_port);
    } else {
        fprintf(stderr, "SYN bit in the packet is not set! \n");
        return -1;
    }

    /* server wait for the ack from client */
    recvfrom(fd, pkg, PKG_LEN, 0, (struct sockaddr*)&cli_addr, &cli_len);
    printf("Server received an ACK packet from %s : %hu\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    pseg = get_segment(pkg, header);
    printf("\t packet (seq = %d , ack = %d)\n", pseg->seq_num, pseg->ack_num);

    if (pseg->ack && pseg->ack_num == seg.seq_num + 1) {
        printf("Complete three way handshake\n");
        printf("==========\n");
        glb_srv_addr = srv_addr;
        glb_cli_addr = cli_addr;
        my_role = server;
        return fd;
    }
    fprintf(stderr, "expected ACK number not found. \n");
    return -1;
}


/*
    client connect to server
    @param server ip
    @param server port
    @return success -> file descriptor fd
            fail -> -1
*/
int cli_connect(char* srv_ip, short srv_port) {
    int fd;
    struct sockaddr_in cli_sock;
    struct sockaddr_in srv_sock;
    int srv_len = sizeof(srv_sock);
    srand(time(NULL));

    /* configure server socket address structure */
    /* The inet_pton() function converts a presentation format address (that is,
     printable form as held in a character string) to network format (usually
     a struct in_addr or some other internal binary representation, in network
     byte order) */
    if (inet_pton(AF_INET, srv_ip, &srv_sock.sin_addr) == 0) return -1;
    srv_sock.sin_family = AF_INET;
    srv_sock.sin_port = htons(srv_port);    // Convert 16-bit positive integers from host to network byte order.

    /* configure client socket address structure */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return -1;

    memset(&cli_sock, 0, sizeof(cli_sock));
    cli_sock.sin_family = AF_INET;
    cli_sock.sin_addr.s_addr = htonl(INADDR_ANY);     // Convert 32-bit from host to network byte order and get local host ip
    cli_sock.sin_port = htons(CLIENT_PORT);     // self-defined client port

    if (bind(fd, (struct sockaddr*)&cli_sock, sizeof(cli_sock)) < 0) return -1;

    /* establish the connection with server */

    /* client send SYN packet */
    memset(&seg, 0, sizeof(seg));   // reset the segment data
    seg.src_port = CLIENT_PORT;
    seg.dest_port = srv_port;
    seg.seq_num = (short)(rand() % 10000);
    seg.syn = 1;
    /* copy segment header data into packet */
    create_pkg(pkg, seg);

    /* send to server */
    if (sendto(fd, pkg, PKG_LEN, 0, (struct sockaddr*)&srv_sock, sizeof(srv_sock)) < 0) return -1;
    printf("Client send a SYN packet to %s : %hu\n", srv_ip, srv_port);

    /* wait for an ack from server */
    recvfrom(fd, pkg, PKG_LEN, 0, (struct sockaddr*)&srv_sock, &srv_len);
    printf("Client received a SYN/ACK packet from %s : %hd\n", inet_ntoa(srv_sock.sin_addr), ntohs(srv_sock.sin_port));

    /* get segment header data */
    pseg = get_segment(pkg, header);
    printf("\t Get a packet (seq = %d , ack = %d)\n", pseg->seq_num, pseg->ack_num);
    /* parse data from segment */
    if (pseg->syn && pseg->ack && pseg->ack_num == seg.seq_num + 1) {
        // set segment data
        seg.src_port = CLIENT_PORT;
        seg.dest_port = srv_port;
        seg.syn = 0;
        seg.ack = 1;
        seg.ack_num = pseg->seq_num + 1;
        seg.seq_num = pseg->ack_num;
        create_pkg(pkg, seg);
        if (sendto(fd, pkg, PKG_LEN, 0, (struct sockaddr*)&srv_sock, sizeof(srv_sock)) < 0) return -1;
        printf("Client send an ACK packet to %s : %hu\n", srv_ip, srv_port);
    } else {
        fprintf(stderr, "expected ACK number not found. \n");
        return -1;
    }
    glb_srv_addr = srv_sock;
    glb_cli_addr = cli_sock;
    printf("Complete three way handshake\n");
    printf("==========\n");
    my_role = client;
    return fd;
}

bool isfin(int fd) {
    struct sockaddr_in dest_addr;
    dest_addr = (my_role == client ? glb_srv_addr : glb_cli_addr);
    /* get segment header data from the current packet */
    pseg = get_segment(pkg, header);
    if (!pseg->fin) return false;
    printf("Received a FIN packet from %s : %hu\n", inet_ntoa(dest_addr.sin_addr), pseg->src_port);
    printf("\t Get a packet (seq = %d , ack = %d)\n", pseg->seq_num, pseg->ack_num);

    /* send back an ack packet to fin */
    memset(&seg, 0, sizeof(seg));   // reset the segment header data
    seg.src_port = pseg->dest_port;
    seg.dest_port = pseg->src_port;
    seg.seq_num = pseg->ack_num;
    seg.ack_num = pseg->seq_num + 1;
    seg.ack = 1;
    create_pkg(pkg, seg);

    /* send to destination socket address */
    if (sendto(fd, pkg, PKG_LEN, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) return -1;
    printf("Send an ACK packet to %s : %hu\n", inet_ntoa(dest_addr.sin_addr), seg.dest_port);
    return true;
}