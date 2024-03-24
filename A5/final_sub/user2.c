#include<stdio.h> 
#include<stdlib.h> 
#include<string.h> 
#include "msocket.h"

int main(int argc, char const *argv[])
{
    // int sem = semget(ftok("tcp_udp.png",5), 1, IPC_CREAT | 0666);
    int sockfd = m_socket(AF_INET, SOCK_MTP, 0);
    if(sockfd < 0){
        perror("socket creation failed");
        return -1;
    }
    printf("Socket created!\n");

    if(m_bind(sockfd, "127.0.0.1", 8080, "127.0.0.1", 8081) < 0){
        perror("bind failed");
        return -1;
    }
    printf("Bind done\n");

    // sleep(20);
    char buf[MAXBLOCK]; memset(buf, 0, MAXBLOCK);
    FILE *f = fopen("user2.txt", "w");
    int count_recv = 0;
    while (1) {
        int n = m_recvfrom(sockfd,buf,MAXBLOCK);
        count_recv++;
        if(n > 0){
            write(fileno(f), buf, n);
            if (buf[n-1] == '\n') break;
        }
        else{
            // perror("recv failed");
            sleep(1);
        }
        memset(buf, 0, MAXBLOCK);
    }
    sleep(1000); // can be done by semaphores if on the same device as shown by commenting it out, done so that buffer is not cleared and it is still allocated until all the packets have been sent
    // struct sembuf vop;
    // vop.sem_num = 0;
    // vop.sem_op = 1;
    // vop.sem_flg = 0;
    // semop(sem, &vop, 1);
    printf("Total m_recvfrom made: %d\n", count_recv);
    return 0;
}
