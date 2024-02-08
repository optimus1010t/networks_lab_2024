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
    int pop3_port;
    if ( argc != 2 ) {
        printf("Usage: ./popserver <pop3_port>\n");
        exit(0);
    }
    else {
        pop3_port = atoi(argv[1]);
    }
	int	sockfd, newsockfd ; /* Socket descriptors */
	int	clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		    = htons(pop3_port);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}

    if(listen(sockfd, 5) < 0) {
        printf("Listen error\n");
        exit(0);
    }

    char domain[MAX_DOMAIN] = "iitkgp.edu\0";
    char service_ready[100]; sprintf(service_ready, "+OK POP3 server ready <%s>\r\n", domain);

	while (1) {
        int n, i, j;
        char buf[MAX_BUFF]; memset(buf, 0, sizeof(buf));

        clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}
		if (fork() == 0) {
			close(sockfd);
            
            memset(buf, 0, sizeof(buf)); sprintf(buf, "%s", service_ready);
            send(newsockfd, buf, strlen(buf), 0);
            
            static char username[MAX_USERNAME]; memset(username, 0, sizeof(username));
            static char password[MAX_PASSWORD]; memset(password, 0, sizeof(password));

            memset(buf, 0, sizeof(buf));
			while (1) {
                char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                n = recv(newsockfd, temp_buf, MAX_BUFF, 0);
                strcat(buf, temp_buf);
                if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                    break;
                }
            }

            if (strncmp(buf, "USER", 4) != 0) {
                memset(buf, 0, sizeof(buf)); sprintf(buf, "-ERR Syntax error: command unrecognized username entry expected\r\n");
                send(newsockfd, buf, strlen(buf), 0);
                close(newsockfd);
                exit(0);
            }
            i = 4;
            while (i < strlen(buf) && (buf[i] == ' ' || buf[i] == '\t')) i++;
            if (i == strlen(buf)) {
                memset(buf, 0, sizeof(buf)); sprintf(buf, "-ERR Invalid username: username cannot be empty\r\n");
                send(newsockfd, buf, strlen(buf), 0);
                close(newsockfd);
                exit(0);
            }
            else {
                j = 0;
                while (buf[i] != '\r' && buf[i] != '\n' && buf[i] != ' ' && buf[i] != '\t') {
                    username[j] = buf[i];
                    i++;
                    j++;
                }   username[j] = '\0';                
            }
            char path[MAX_PATH]; memset(path, 0, sizeof(path));
            strcat(path, "./");
            strcat(path, username);
            if (access(path, F_OK) == -1) {
                memset(buf, 0, sizeof(buf)); sprintf(buf, "-ERR No such user here\r\n");
                send(newsockfd, buf, strlen(buf), 0);
                close(newsockfd);
                exit(0);
            }
            printf("subdirectory exists\n");
            fflush(stdout);

            FILE* user_file = fopen("./user.txt", "r");
            if (user_file == NULL) {
                printf("Error opening file.\n");
                return NULL;
            }
            int flag = 0;
            char temp_username[MAX_USERNAME]; memset(temp_username, 0, sizeof(temp_username));
            char temp_password[MAX_PASSWORD]; memset(temp_password, 0, sizeof(temp_password));
            if (user_file == NULL) {
                printf("Error opening file.\n");
                return NULL;
            }

            char line[MAX_USERNAME + MAX_PASSWORD + 2];
            while (fgets(line, sizeof(line), user_file) != NULL) {
                // printf("line: %s\n", line);
                // fflush(stdout);
                int pos = 0;
                bool found_username = false;

                // Skip leading whitespaces
                while (line[pos] != '\0' && (line[pos] == ' ' || line[pos] == '\t')) {
                    pos++;
                }

                // Extract username
                int username_pos = 0;
                while (line[pos] != ' ' && line[pos] != '\t') {
                    temp_username[username_pos++] = line[pos++];
                }
                temp_username[username_pos] = '\0';

                // Skip whitespaces between username and password
                while ((line[pos] == ' ' || line[pos] == '\t')) {
                    pos++;
                }

                // Extract password
                int password_pos = 0;
                while (line[pos] != '\0' && line[pos] != '\n') {
                    temp_password[password_pos++] = line[pos++];
                }
                temp_password[password_pos] = '\0';

                if (strcmp(temp_username, username) == 0) {
                    strncpy(password, temp_password, MAX_PASSWORD);
                    flag = 1;
                    break;
                }
            }
            fclose(user_file);

            if (flag == 0) {
                memset(buf, 0, sizeof(buf)); sprintf(buf, "-ERR No such user here\r\n");
                send(newsockfd, buf, strlen(buf), 0);
                close(newsockfd);
                exit(0);
            }
            memset(buf, 0, sizeof(buf)); sprintf(buf, "+OK %s is a valid mailbox\r\n", username);
            send(newsockfd, buf, strlen(buf), 0);

            memset(buf, 0, sizeof(buf));
			while (1) {
                char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                n = recv(newsockfd, temp_buf, MAX_BUFF, 0);
                strcat(buf, temp_buf);
                if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                    break;
                }
            }

            if (strncmp(buf, "PASS", 4) != 0) {
                memset(buf, 0, sizeof(buf)); sprintf(buf, "-ERR Syntax error: command unrecognized username entry expected\r\n");
                send(newsockfd, buf, strlen(buf), 0);
                close(newsockfd);
                exit(0);
            }
            i = 4;
            while (i < strlen(buf) && (buf[i] == ' ' || buf[i] == '\t')) i++;
            
            j = 0;
            while (buf[i] != '\r' && buf[i] != '\n' && buf[i] != ' ' && buf[i] != '\t') {
                password[j] = buf[i];
                i++;
                j++;
            }   password[j] = '\0';

            if (strcmp(password, temp_password) != 0) {
                memset(buf, 0, sizeof(buf)); sprintf(buf, "-ERR Permission Denied\r\n");
                send(newsockfd, buf, strlen(buf), 0);
                close(newsockfd);
                exit(0);
            }
            memset(buf, 0, sizeof(buf)); sprintf(buf, "+OK maildrop ready\r\n");
            send(newsockfd, buf, strlen(buf), 0);     

            memset(buf, 0, sizeof(buf));
			while (1) {
                char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                n = recv(newsockfd, temp_buf, MAX_BUFF, 0);
                strcat(buf, temp_buf);
                if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                    break;
                }
            }
            while (strncmp(buf, "QUIT", 4) != 0){
                
            }
            
            close(newsockfd);
			exit(0);
		}
		close(newsockfd);
	}
	return 0;
}
