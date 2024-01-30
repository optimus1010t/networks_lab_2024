#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define MAX_BUFF 100
#define FILE_MAX_BUFF 100

// In accordance wirh RFC 5321 and RFC 1939
int main()
{
	int	sockfd, newsockfd ; /* Socket descriptors */
	int	clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		    = htons(20000);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}
	listen(sockfd, 5);

    char domain_recv[100] = "iitkgp.edu\0";
    char service_ready[100];
    fprintf(service_ready, "220 <%s> Service ready\0", domain);

	while (1) {
        int n;
        char buf[MAX_BUFF]; memset(buf, 0, sizeof(buf));
        clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}
		if (fork() == 0) {
			close(sockfd);
            memset(buf, 0, sizeof(buf)); fprintf(buf, "%s", service_ready);
            send(newsockfd, buf, strlen(buf), 0);
			n = recv(newsockfd, buf, MAX_BUFF, 0);
            // check if the first four characters are HELO
            if (strncmp(buf, "HELO", 4) != 0) {
                memset(buf, 0, sizeof(buf)); fprintf(buf, "500 Syntax error: command unrecognized\0");
                send(newsockfd, buf, strlen(buf), 0);
                close(newsockfd);
                exit(0);
            }
            // check if the domain is correct by extracting the domain from the HELO enclosed in <>
            char domain_recv[100];
                int i = 0;
                while (buf[i] != '<') i++;
                i++; int j = 0;
                while (buf[i] != '>') {
                    // ignore the whitespace characters
                    if (buf[i] == ' ' || buf[i] == '\t') {
                        i++;
                        continue;
                    }
                    domain_recv[j] = buf[i];
                    i++;
                    j++;
                }   domain_recv[j] = '\0';
			exit(0);
		}
		close(newsockfd);
	}
	return 0;
}
