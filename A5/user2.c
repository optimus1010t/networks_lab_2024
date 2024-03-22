#include<stdio.h> 
#include<stdlib.h> 
#include<string.h> 
#include "msocket.h"
#include<unistd.h>

// this will run on port 8080, and talk to user 2 on port 8081
int main(int argc, char const *argv[])
{
    int sockfd = m_socket(AF_INET, SOCK_MTP, 0);
    if(sockfd < 0){
        perror("socket creation failed");
        return -1;
    }

    printf("Socket created!\n");


    printf("Binding to user2\n");

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(8081);
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

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

    sleep(10);

    char buf[] = "Hello from user1";

    if(m_sendto(sockfd, buf, strlen(buf), (struct sockaddr*)&addr) < 0){
        perror("sendto failed");
        return -1;
    }
    char buf1[] = "Hello from user2";
    if(m_sendto(sockfd, buf1, strlen(buf), (struct sockaddr*)&addr) < 0){
        perror("sendto failed");
        return -1;
    }
    char buf2[] = "Hello from user3";
    if(m_sendto(sockfd, buf2, strlen(buf), (struct sockaddr*)&addr) < 0){
        perror("sendto failed");
        return -1;
    }
    sleep(100);

    // if(m_close(sockfd) < 0){
    //     perror("close failed");
    //     return -1;
    // }

    return 0;
}