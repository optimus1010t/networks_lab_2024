#include<stdio.h> 
#include<stdlib.h> 
#include<string.h> 
#include "msocket.h"
#include<unistd.h>

int main(int argc, char const *argv[])
{
    // int sem = semget(ftok("tcp_udp.png",5), 1, IPC_CREAT | 0666);
    // semctl(sem, 0, SETVAL, 0);
    int sockfd = m_socket(AF_INET, SOCK_MTP, 0);
    if(sockfd < 0){
        perror("socket creation failed");
        return -1;
    }
    printf("Socket created!\n");

    char* dest_ip = "127.0.0.1";
    int dest_port = 8080;

    if(m_bind(sockfd, "127.0.0.1", 8081, "127.0.0.1", 8080) < 0){
        perror("bind failed");
        return -1;
    }
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(8080);
    addr.sin_family = AF_INET;

    printf("Bind done\n");

    char buf[MAXBLOCK]; memset(buf, 0, MAXBLOCK);
    FILE *f = fopen("user1.txt", "r"); int n;
    int count_send = 0;
    while (n=read(fileno(f), buf, MAXBLOCK)){
        int check = m_sendto(sockfd, buf, n, dest_ip, dest_port);
        count_send++;
        while(check < 0){
            // perror("send failed");
            sleep(1);
            check = m_sendto(sockfd, buf, n, dest_ip, dest_port);
            count_send++;
        };
        if (buf[n-1] == '\n') break;
        memset(buf, 0, MAXBLOCK);        
    }
    sleep(1000); // can be done by semaphores if on the same device as shown by commenting it out, done so that buffer is not cleared and it is still allocated until all the packets have been sent
    // struct sembuf pop;
    // pop.sem_num = 0;
    // pop.sem_op = -1;
    // pop.sem_flg = 0;
    // semop(sem, &pop, 1); 
    printf("Total m_sendto calls sent: %d\n", count_send);
    return 0;
}