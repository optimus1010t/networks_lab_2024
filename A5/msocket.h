#ifndef MSOCKET_H
#define MSOCKET_H
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sys/sem.h>

#define MAXBLOCK 1024
#define RWND 10
#define SWND 5
#define MAXIP 50
#define MAXSEQNO 16
#define MAXSOCKETS 25
#define MAXWNDW 5

#define MAXBUF 1030

#define SOCK_MTP 9999
#define T 5
#define p 0.5

struct ACKPacket {
    char message[4]; 
    int lastInorderSeqNum;
    int windowSize;
};

struct wnd {
    int size;
    int seq_no[RWND > SWND ? RWND : SWND];
};

struct m_socket_handler {
    int is_alloted;
    pid_t process_id;
    int socket_id;
    char src_ip_addr[MAXIP];
    int src_port;
    char send_buf[SWND][MAXBLOCK];
    char recv_buf[RWND][MAXBLOCK];
    struct wnd rwnd;
    struct wnd swnd;

    char dest_ip_addr[MAXIP]; // destination ip
    int dest_port; // destination port

    int send_seq_no; // next sequence number to be sent 
    int recv_seq_no; // next expected sequence number

    int send_status[SWND];  // 0: can be used, 1: havent sent or acked
    int recv_status[RWND];  // 0: can be used, n: yet to be delivered with seq no. n
    // set timeval to store current time for send of messages
    struct timeval send_time[SWND];

    int swnd_markers[2];  // starting and ending index of swnd
    int rwnd_markers[3];  // starting and ending index of rwnd
};

struct sock_info {
    int sockfd;
    char src_ip[MAXIP];
    int src_port;
    int err;
};

int m_socket(int domain, int type, int protocol);
int m_bind(int sockfd, char* source_ip, int source_port, char* dest_ip, int dest_port);
int m_sendto(int sockfd, const void *buf, size_t len);
int m_recvfrom(int sockfd, void *buf, size_t len);
int m_close(int fd);

int dropMessage(float pp);
