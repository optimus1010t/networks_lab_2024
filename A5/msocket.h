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
#define MAXSEQNO 10
#define MAXSOCKETS 25

#define SOCK_MTP 9999
#define T 5
#define p 0.5

struct wnd {
    int size;
    int seq_no[10];
};

struct m_socket_handler {
    short int is_alloted;
    pid_t process_id;
    int socket_id;
    char src_ip_addr[MAXIP];
    unsigned short int src_port;
    char send_buf[SWND][MAXBLOCK];
    char recv_buf[RWND][MAXBLOCK];
    struct wnd rwnd;
    struct wnd swnd;

    char dest_ip_addr[MAXIP];
    unsigned short int dest_port;

    int send_status[SWND];  // -1: can be used, 0: sent, 1: acked
    int recv_status[RWND];

    int send_seq_no;
    int recv_seq_no;

    int swnd_markers[2];  // starting and ending+1 index of swnd
    int rwnd_markers[2];  // starting and ending+1 index of rwnd
};

int m_socket(int domain, int type, int protocol);
int m_bind(int sockfd, char source_ip[MAXIP], unsigned short int source_port, char dest_ip[MAXIP], unsigned short int dest_port);
int m_sendto(int sockfd, const void *buf, size_t len, int flags);
int m_recvfrom(int sockfd, void *buf, size_t len, int flags);
int m_close(int fd);

void* R();
void* S();
void* G();

int dropMessage(float pp);
