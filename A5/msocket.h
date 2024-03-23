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
#include <errno.h>
#include <sys/stat.h>

#define MAXBLOCK 15
#define RWND 5
#define SWND 10
#define MAXIP 50
#define MAXSEQNO 16
#define MAXSOCKETS 25
#define MAXWNDW 5

#define MAXBUF 16

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
    int send_len[SWND];
    char recv_buf[RWND][MAXBLOCK];
    int recv_len[RWND];
    struct wnd rwnd;
    struct wnd swnd;
    int flag_nospace;

    char dest_ip_addr[MAXIP]; // destination ip
    int dest_port; // destination port

    int send_seq_no; // next sequence number to be sent 
    int recv_seq_no; // next expected sequence number

    int recv_status[RWND];  // 0: can be used, n: yet to be delivered with seq no. n // redundant ig ????
    struct timeval send_time[SWND];

    int swnd_markers[2];  // starting and ending index of swnd
    int rwnd_markers[3];  // starting, ending index and sequence no. of the last message given to the user
};

struct sock_info {
    int sockfd;
    char src_ip[MAXIP];
    int src_port;
    int err;
};

int m_socket(int domain, int type, int protocol);
int m_bind(int sockfd, char* source_ip, int source_port, char* dest_ip, int dest_port);
int m_sendto(int sockfd, char *buf, size_t len, struct sockaddr* dest_addr);
int m_recvfrom(int sockfd, char *buf, size_t len);
int m_close(int fd);

void sighandler (int signum);

int dropMessage(float pp);
