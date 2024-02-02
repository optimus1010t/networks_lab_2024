#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>
#include <sys/select.h>


#define MAX_BUFF 4000
#define MAX_DOMAIN 512
#define MAX_USERNAME 100
#define MAX_PASSWORD 100
#define MAX_PATH 1000
#define MAX_MAIL 6000
#define MAX_MAILID 650

// In accordance wirh RFC 5321 and RFC 1939
int main(int argc, char*argv[])
{
    int	sockfd ;
    char server_ip[20];
    int port;
    if ( argc != 3 ) {
        printf("Usage: ./smtpmail <server_ip> <smtp_port>\n");
        exit(0);
    }
    else {
        strcpy(server_ip, argv[1]);
        port = atoi(argv[2]);
    }
    struct sockaddr_in	serv_addr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Unable to create socket\n");
        exit(0);
    }
    serv_addr.sin_family	= AF_INET;
    inet_aton(server_ip, &serv_addr.sin_addr);
    serv_addr.sin_port	= htons(port);
    if ((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) {
        perror("Unable to connect to server");
        exit(0);
    }
    printf("Connected to server\n");
    char buf[MAX_BUFF];
    int n;
    while (1) {
        n = send(sockfd, "Hello from client\n", 18, 0);
        if (n < 0) {
            perror("Error reading from socket");
            exit(0);
        }
        if (n == 0) {
            printf("Server closed connection\n");
            break;
        }
        buf[n] = 0;
        printf("%s", buf);
    }
    close(sockfd);
	return 0;
}

