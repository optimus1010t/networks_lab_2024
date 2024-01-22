#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define MAXLINE 1024

int main(){
    int sockfd, err;
    struct sockaddr_in servaddr;
    int n;
    socklen_t len;
    char filename[101];

    printf("Enter the filename: ");
    fgets(filename, sizeof(filename), stdin);
    if (filename[strlen(filename) - 1] == '\n') {
        filename[strlen(filename) - 1] = '\0';
    }
        
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    memset (&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;              
    servaddr.sin_port = htons(8181);
    err = inet_aton("127.0.0.1", &servaddr.sin_addr);
    if (err == 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    sendto(sockfd, (const char*)filename, strlen(filename), 0, (const struct sockaddr*) &servaddr, sizeof(servaddr));
    
    char buffer[MAXLINE];
    char filename_r[MAXLINE];
    memset(buffer, 0, sizeof(buffer));
    memset(filename_r, 0, sizeof(filename_r));
    len = sizeof(servaddr);                           
    n = recvfrom ( sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *) &servaddr, &len);
    buffer[n] = '\0';
    char *hello = "HELLO";
    char *end = "END";
    FILE *file;
    int i = 1;
    if (i<0){
        printf("i cannot have a negative value that is %d\n", i);
        exit(0);
    }
    if (strncmp(buffer, "NOT FOUND", strlen("NOT FOUND")) == 0) {
        sscanf(buffer, "NOT FOUND %s", filename_r);
        printf("File %s Not Found\n",filename_r);
        exit(0);
    }
    if (strncmp(buffer, "!@#$^&*Error opening file", strlen("!@#$^&*Error opening file")) == 0) {
        sscanf(buffer, "!@#$^&*Error opening file %s", filename_r);
        printf("Error opening file %s\n",filename_r);
        exit(0);
    }
    if (strcmp(buffer,"!@#$^&*File is empty") == 0){
        printf("ERROR : The file %s is empty.\n", filename);
        exit(0);
    }
    int flag = 0;
    if (strcmp(buffer,hello)==0){
        char file_out[101];
        printf("Enter the output filename with extension: ");
        fgets(file_out, sizeof(file_out), stdin);
        if (file_out[strlen(file_out) - 1] == '\n') {
            file_out[strlen(file_out) - 1] = '\0';
        }
        flag = 1;
        file = fopen(file_out, "w");
        while (strcmp(buffer,end) != 0){
            char spacey[MAXLINE];
            sprintf(spacey, "WORD%d", i);
            sendto(sockfd, (const char*)spacey, strlen(spacey), 0, (const struct sockaddr*) &servaddr, sizeof(servaddr));
            len = sizeof(servaddr);                           
            n = recvfrom ( sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *) &servaddr, &len);
            buffer[n] = '\0';
            if (strcmp(buffer,"!@#$^&*Word limit exceeded") == 0){
                fprintf(file, "ERROR : The file %s does not have %d word(s).\n", filename, i);
                exit(0);
            }
            if (strcmp(buffer,"!@#$^&*Not in expected format") == 0){
                fprintf(file, "The sent prompt %s is not in the format WORD<integer>\n", spacey);
                exit(0);
            }
            if (strcmp(buffer,end) != 0) fprintf(file, "%s\n", buffer);
            i += 1;
        }
    }
    
    if (flag == 1) fclose(file);
    close(sockfd);
    return 0;

}

/*  
    UDP is an unreliable protocol. The way of sending WORD<i> might work like an acknowledgement for the previous word.
    But this causes a lot of extra packets for the acknowledgement and makes the overall process slow as it is getting transferred
    word by word.
 */