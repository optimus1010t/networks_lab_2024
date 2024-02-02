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

int main()
{
	int	sockfd, newsockfd ; /* Socket descriptors */
	int	clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i;
	char buf[MAX_BUFF];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		    = htons(20001);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}
	listen(sockfd, 5); 	

	while (1) {
        clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}
		if (fork() == 0) {
			close(sockfd);
			int n;
			memset(buf, 0, sizeof(buf));
			n = recv(newsockfd, buf, MAX_BUFF, 0);
			//Extracting the key from the buffer that is charcaters until * is observed
			int i = 0;
			while (buf[i] != '*') {
				i++;
			}
			for (; i < MAX_BUFF; i++) buf[i] = '\0';
			int k = atoi(buf);
			// set the filename to be the ip and the port number of the client
			char filename[FILE_MAX_BUFF];
			memset(filename, 0, sizeof(filename));
			sprintf(filename, "%s.%d.txt", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
			// open the file
			int fd = open(filename, O_WRONLY | O_CREAT, 0666);
			memset(buf, 0, sizeof(buf));
			while (1) {
				n = recv(newsockfd, buf, MAX_BUFF, 0);
				ssize_t buf_len = 0;
				// if (buf[0] != '$') 
				// buf_len = write(fd, buf, strlen(buf));
				if (buf[strlen(buf)-1] == '\n'){
					// buf[strlen(buf) - 1] = '\0';
					buf_len = write(fd, buf, strlen(buf));
					break;
				}
				buf_len = write(fd, buf, strlen(buf));
				if (buf_len == -1) {
					printf("Error in writing to file\n");
					continue;
				}
				// if (n < MAX_BUFF) {
				// 	break;
				// }
				bzero(buf, MAX_BUFF);
			}
			memset(buf, 0, sizeof(buf));
			char filename_enc[FILE_MAX_BUFF];
			sprintf(filename_enc, "%s.enc",filename);
			int fd_enc = open(filename_enc, O_WRONLY | O_CREAT, 0666);
			close(fd);
			fd = open(filename, O_RDONLY);
			ssize_t file_len;
			while ((file_len = read(fd, buf, sizeof(buf))) > 0){
				int j;
				for (j = 0; (ssize_t)j < file_len; j++) {
					if (buf[j] >= 'a' && buf[j] <= 'z') {
						buf[j] = (buf[j] - 'a' + k + 26) % 26 + 'a';
					}
					else if (buf[j] >= 'A' && buf[j] <= 'Z') {
						buf[j] = (buf[j] - 'A' + k + 26) % 26 + 'A';
					}
				}
				int buf_len2 = write(fd_enc, buf, (int)file_len);
				if (buf_len2 == -1) {
					printf("Error in writing to file\n");
					continue;
				}
			}
			close(fd);
			close(fd_enc);
			fd_enc = open(filename_enc, O_RDONLY);
			memset(buf, 0, sizeof(buf));
			int last_size;
			while (read(fd_enc, buf, sizeof(buf)) > 0){
			// 	if (last_size < 100) {
            //     buf[last_size] = '$';
            // }
				last_size = strlen(buf);
				send(newsockfd, buf, strlen(buf), 0);
				memset(buf, 0, sizeof(buf));
			}
			// if (last_size == 100) { 
			// 	buf[0] = '$';
			// 	send(newsockfd, buf, strlen(buf), 0);
			// }
			
			exit(0);
		}
		close(newsockfd);
	}
	return 0;
}
