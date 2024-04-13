#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/select.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>

int main()
{
    struct sockaddr_in servaddr, cliaddr;
    int sockfd;
    socklen_t serv_len, clilen;
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(60000);
    inet_aton("127.0.0.1", &servaddr.sin_addr); // changed and corrected
    serv_len = sizeof(servaddr);

    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port   = htons(30001);
    inet_aton("127.0.0.1", &cliaddr.sin_addr);  // changed and corrected
    clilen = sizeof(cliaddr);

    if (bind(sockfd,(struct sockaddr*)&cliaddr,clilen) < 0) {
        perror("bind : ");
        exit(0);
    }

    if (connect(sockfd, (struct sockaddr*)&servaddr, serv_len) < 0) {
        perror("connect : ");
        exit(0);
    }

    long buff;
    recv(sockfd,&buff,sizeof(long),0);
    buff = ntohl(buff); // added
    printf("No. of entities : %d",buff);

    char buf[2020]; memset(buf,0,2020);

    if (buff != 0) {
        while (1) {
            char temp_buf[2020]; memset(temp_buf,0,2020);
            int n = recv (sockfd,temp_buf,2020,0);
            if (n <= 0) { break; printf("error\n"); }
            strcat(buf,temp_buf);
            if (buf[strlen(buf)-1] == '\n') break;                                                            
        }
    }
    buf[strlen(buf)-1] = '\0';

    int count = 0;
    printf("Entities : \n");
    for (int i = 0; i < 2020 && count < buff; i++) {
        if (buf[i] == '\0') {
            printf("\n");
            count++;    // added
            if (count==buff) break; // added
        }
        printf("%c",buf[i]);        
    }
    char name[203]; memset(name,0,203);

    fgets(name,203,stdin);
    send (sockfd,name,203,0);
    close(sockfd);
    exit(0);    

}