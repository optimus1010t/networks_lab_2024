#include<stdio.h> 
#include<stdlib.h> 
#include<string.h> 
#include "msocket.h"

// this will run on port 8081, and talk to user 1 on port 8080
int main(int argc, char const *argv[])
{
    int sem = semget(ftok("user2.txt",5), 1, IPC_CREAT | 0666);
    int sockfd = m_socket(AF_INET, SOCK_MTP, 0);
    if(sockfd < 0){
        perror("socket creation failed");
        return -1;
    }
    printf("Socket created!\n");


    printf("Binding to user1\n");

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(8080);
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    if(m_bind(sockfd, "127.0.0.1", 8080, "127.0.0.1", 8081) < 0){
        perror("bind failed");
        return -1;
    }
    printf("Bind done\n");

    sleep(20);
    char buf[1024]; memset(buf, 0, 1024);
    FILE *f = fopen("user1.txt", "w");

    while (1) {
        int n = m_recvfrom(sockfd,buf,1024);
        if(n > 0){
            write(fileno(f), buf, strlen(buf));
            if (buf[strlen(buf)-1] == '\n') break;
            memset(buf, 0, 1024);
        }
    }
    struct sembuf vop;
    vop.sem_num = 0;
    vop.sem_op = 1;
    vop.sem_flg = 0;
    semop(sem, &vop, 1);
    return 0;
}