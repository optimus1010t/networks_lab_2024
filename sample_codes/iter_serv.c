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
    int smtp_port;
    if ( argc != 2 ) {
        printf("Usage: ./smtpmail <smtp_port>\n");
        exit(0);
    }
    else {
        smtp_port = atoi(argv[1]);
    }
	int	sockfd, sockfd1, sockfd2 ; /* Socket descriptors */
	int	clilen1, clilen2;
	struct sockaddr_in	cli_addr1, cli_addr2, serv_addr;

    int    i, len, num, rc, on = 1;
    char   buffer[80];
    struct sockaddr_in6   addr;
    if (argc >= 2)
        num = atoi(argv[1]);
    else
        num = 1;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		    = htons(smtp_port);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}

    if(listen(sockfd, 5) < 0) {
        printf("Listen error\n");
        exit(0);
    }

    clilen1 = sizeof(cli_addr1);
    clilen2 = sizeof(cli_addr2);
    int count1 = 0, count2 = 0;
    listen(sockfd, 5);
    int num_sockets = 10;
    fd_set fds;
    FD_ZERO (&fds);
    while(1) {
        FD_SET (sockfd1, &fds);
        FD_SET (sockfd2, &fds);
        size_t nfds = max(sockfd1, sockfd2) + 1;
        select(nfds, &fds, 0, 0, 0);
        if (FD_ISSET(sockfd1, &fds))
        {
            if (count1 == 0){
                sockfd1 = accept(sockfd, (struct sockaddr *) &cli_addr1, &clilen1);
                count1++;
            }
            else {
                char buffer[MAX_MAIL];
                int n = read(sockfd1, buffer, MAX_MAIL);
                if (n < 0) {
                    printf("Error reading from socket\n");
                    exit(0);
                }
                printf("Here is the message: %s\n", buffer);
                n = write(sockfd1, "I got your message", 18);
                if (n < 0) {
                    printf("Error writing to socket\n");
                    exit(0);
                }
            }            
        }
        if (FD_ISSET(sockfd2, &fds))
        {
            if (count2 == 0) {
                sockfd2 = accept(sockfd, (struct sockaddr *) &cli_addr2, &clilen2);
                count2++;
            }
            else {
                char buffer[MAX_MAILID];
                int n = read(sockfd2, buffer, MAX_MAILID);
                if (n < 0) {
                    printf("Error reading from socket\n");
                    exit(0);
                }
                printf("Here is the message: %s\n", buffer);
                n = write(sockfd2, "I got your message", 18);
                if (n < 0) {
                    printf("Error writing to socket\n");
                    exit(0);
                }
            }
        }
    }
	return 0;
}

