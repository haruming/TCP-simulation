#ifndef TCP_H_
#define TCP_H_

#include <arpa/inet.h>
#include <stdbool.h>

/* tcp parameter config */
#define CLIENT_PORT 10001
#define SERVER_PORT 10002
#define HEADER_LEN 32
#define PKG_LEN 544
#define BUF_SIZE 10240

enum role {client, server, unknown};

/* tcp segment structure */
typedef struct
{
    short src_port;         // 16-bit source port
    short dest_port;        // 16-bit destination port
    int seq_num;            // 32-bit sequence number
    int ack_num;            // 32-bit acknowledgement number(if ACK set)
    short head_len:4,       // 4-bit header length
          not_use:6,        // 6-bit reserved
          urg:1,            // 1-bit urgent flag
          ack:1,            // 1-bit acknowledgement flag
          psh:1,            // 1-bit push flag
          rst:1,            // 1-bit reset flag
          syn:1,            // 1-bit syn flag
          fin:1;            // 1-bit finish flag
    short win_size;         // 16-bit receive window size

} Segment, *PSegment;

/* global variable */
extern int glb_rtt;                    // RTT delay time(millisecond)
extern int glb_threshold;              // Threshold(byte)
int glb_mss;                    // Maximum segment size(byte)

/* local variable */
extern enum role my_role;              // Client or Server
extern struct sockaddr_in glb_srv_addr;    // server socket address
extern struct sockaddr_in glb_cli_addr;    // client socket address
extern char pkg[PKG_LEN];              // package data
extern char header[HEADER_LEN];        // package header
extern Segment seg;                    // variable storing TCP segment
extern PSegment pseg;                  // pointer to the segment

int send_data(int, char*, int);      // send data
int receive_data(int);               // receive data
int close(int);                 // terminate a tcp connection
bool is_final(int);

int srv_listen(short);              // server listen for connection on a given port
int cli_connect(char*, short);      // client connect to server by given the IP address and port

#endif