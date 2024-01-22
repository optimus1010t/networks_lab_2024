#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define MAXLINE 1024

int main(){
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    int n;
    socklen_t len;
    char buffer[MAXLINE];
    memset(buffer, 0, sizeof(buffer));

    // Create a socket file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0){
        perror("Socket Creation Failed");
        exit(EXIT_FAILURE);
    }

    memset (&servaddr, 0, sizeof(servaddr));
    memset (&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8181);                

    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {
        perror("Bind Failed");
        exit(EXIT_FAILURE);
    }

    printf("\nServer Running ... \n");
    FILE *fp;
    int flag = 0;

    while(1) {
        char notf[MAXLINE] = "NOT FOUND ";
        char err_fi[MAXLINE] = "!@#$^&*Error opening file ";
        char err_word[MAXLINE] = "!@#$^&*Word limit exceeded";
        char err_format[MAXLINE] = "!@#$^&*Not in expected format";
        char err_fileempty[MAXLINE] = "!@#$^&*File is empty";
        memset(buffer, 0, sizeof(buffer));
        len = sizeof(cliaddr);                           
        n = recvfrom ( sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *) &cliaddr, &len);
        buffer[n] = '\0';
        char word[MAXLINE]; int i, wordCount;        
        if (sscanf(buffer, "WORD%d", &i) == 1) {
            fseek(fp, 0, SEEK_SET);
            wordCount = 0;
            while (fscanf(fp, "%s", word) == 1) {
                wordCount++;
                if (wordCount == i+1) {
                    sendto(sockfd, (const char*)word, strlen(word), 0, (const struct sockaddr*) &cliaddr, sizeof(cliaddr));
                    if (strcmp(word,"END\0")==0)
                    {
                        fp = NULL;
                        flag = 0;
                    }
                    break;
                }
            }
            if (wordCount < i+1) {
                sendto(sockfd, (const char*)err_word, strlen(err_word), 0, (const struct sockaddr*) &cliaddr, sizeof(cliaddr));
                flag = 0;
                fp = NULL;
                continue;
            }
        } 
        else {
            if (flag == 1) {
                sendto(sockfd, (const char*)err_format, strlen(err_format), 0, (const struct sockaddr*) &cliaddr, sizeof(cliaddr));
                flag = 0;
                fp = NULL;
                continue;
            }
            else {
                struct stat s;
                if (stat(buffer,&s) != 0){
                    strcat(notf,buffer);
                    sendto(sockfd, (const char*)notf, strlen(notf), 0, (const struct sockaddr*) &cliaddr, sizeof(cliaddr));
                    flag = 0;
                    continue;
                }
                else {    
                    fp = fopen(buffer, "r");
                    if (fp == NULL) {
                        strcat(err_fi,buffer);
                        sendto(sockfd, (const char*)err_fi, strlen(err_fi), 0, (const struct sockaddr*) &cliaddr, sizeof(cliaddr));
                        flag = 0;
                        continue;
                    }
                    else {
                        if (fscanf(fp, "%s", word) == 1){
                            sendto(sockfd, (const char*)word, strlen(word), 0, (const struct sockaddr*) &cliaddr, sizeof(cliaddr));
                            if (strcmp(word,"HELLO\0")==0) flag = 1;
                            continue;
                        } else {
                            sendto(sockfd, (const char*)err_fileempty, strlen(err_fileempty), 0, (const struct sockaddr*) &cliaddr, sizeof(cliaddr));
                            flag = 0;
                            fp = NULL;
                            continue;
                        }
                    }
                }
            }
        }
    }
    close(sockfd);
    return 0;
}