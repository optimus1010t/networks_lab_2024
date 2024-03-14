#include<stdio.h> 
#include<stdlib.h> 
#include<string.h> 
#include "msocket.h"

// this will run on port 8081, and talk to user 1 on port 8080
int main(int argc, char const *argv[])
{
    int sockfd = m_socket(AF_INET, SOCK_MTP, 0);
    if(sockfd < 0){
        perror("socket creation failed");
        return -1;
    }

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(8080);
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    fflush(stdout);
    if(m_bind(sockfd, "127.0.0.1", 8080, "127.0.0.1", 8081) < 0){
        perror("bind failed");
        return -1;
    }
    fflush(stdout);

    sleep(10);
    char buf[1024];

    if(m_recvfrom(sockfd,buf,1024) < 0){
        perror("recvfrom failed");
        return -1;
    }

    printf("Received: %s\n", buf);

    sleep(100);

    if(m_close(sockfd) < 0){
        perror("close failed");
        return -1;
    }

    return 0;
}