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
    int smtp_port;
    if ( argc != 2 ) {
        printf("Usage: ./smtpmail <smtp_port>\n");
        exit(0);
    }
    else {
        smtp_port = atoi(argv[1]);
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
	serv_addr.sin_port		    = htons(smtp_port);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}

    if(listen(sockfd, 5) < 0) {
        printf("Listen error\n");
        exit(0);
    }

    char domain[MAX_DOMAIN] = "iitkgp.edu\0";
    char service_ready[100]; sprintf(service_ready, "220 <%s> Service ready\r\n", domain);

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
            memset(buf, 0, sizeof(buf)); sprintf(buf, "%s", service_ready);
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
            // check if the first four characters are HELO
            if (strncmp(buf, "HELO", 4) != 0) {
                memset(buf, 0, sizeof(buf)); sprintf(buf, "500 Syntax error: command unrecognized\r\n");
                send(newsockfd, buf, strlen(buf), 0);
                close(newsockfd);
                exit(0);
            }
            char domain_recv[MAX_DOMAIN]; memset(domain_recv, 0, sizeof(domain_recv));
            int i = 0;
            i = 4;
            while (i < strlen(buf) && (buf[i] == ' ' || buf[i] == '\t')) i++;
            if (i == strlen(buf)) {
                // memset(buf, 0, sizeof(buf)); sprintf(buf, "501 Syntax error in parameters or arguments\r\n");
                // send(newsockfd, buf, strlen(buf), 0);
                // close(newsockfd);
                // exit(0);
            }
            else {
                int j = 0;
                while (buf[i] != '\r' && buf[i] != '\n' && buf[i] != ' ' && buf[i] != '\t') {
                    domain_recv[j] = buf[i];
                    i++;
                    j++;
                }   domain_recv[j] = '\0';                
            }
            // check if @ is present in the HELO command if yes find its index
            
            memset(buf, 0, sizeof(buf)); sprintf(buf, "250 OK Hello %s\r\n", domain_recv);
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
            // check if the first four characters are MAIL
            if (strncmp(buf, "MAIL", 4) != 0) {
                memset(buf, 0, sizeof(buf)); sprintf(buf, "500 Syntax error: command unrecognized\r\n");
                send(newsockfd, buf, strlen(buf), 0);
                close(newsockfd);
                exit(0);
            }
            char recv_time[1000];
            char username[MAX_USERNAME];
            // extract the username and domain from the MAIL FROM command
            i = 0;
            while (buf[i] != '@' && i < strlen(buf)) i++;
            char domain_recv2[MAX_DOMAIN]; memset(domain_recv2, 0, sizeof(domain_recv2));
            if (i == strlen(buf)) {
                memset(buf, 0, sizeof(buf)); sprintf(buf, "501 Syntax error in parameters or arguments\r\n");
                send(newsockfd, buf, strlen(buf), 0);
                close(newsockfd);
                exit(0);
            }
            else {
                int first_time_col = 0;
                int last_time_arrow = strlen(buf) - 1;
                int first_time_arrow = 0;
                int i_backup = i;
                i = 0;
                while (buf[i] != ':' && i<(strlen(buf)-1)) i++;
                first_time_col = i;
                i = 0;
                while (buf[i] != '<' && i<(strlen(buf)-1)) i++;
                first_time_arrow = i;
                i = strlen(buf) - 1;
                while (buf[i] != '>' && i > 0) i--;
                last_time_arrow = i;
                i = i_backup;
                i++; int j = 0;
                while (buf[i] != '\r' && buf[i] != '\n' && buf[i] != ' ' && buf[i] != '\t' && (i < last_time_arrow || last_time_arrow == 0)) {
                    domain_recv2[j] = buf[i];
                    i++;
                    j++;
                }   domain_recv2[j] = '\0';
                i = i_backup;
                j = 0;
                while (buf[i]!= ' ' && buf[i] != '\t' && i > first_time_col && (i > first_time_arrow || first_time_arrow == (strlen(buf)-1))) {
                    i--;
                }
                i++;
                while (i<i_backup) {
                    username[j] = buf[i];
                    i++;
                    j++;
                }   username[j] = '\0';
            }
            // i = 0;
            // while (buf[i] != '<') i++;
            // i++; int j = 0;
            // while (buf[i] != '@') {
            //     // ignore the whitespace characters
            //     if (buf[i] == ' ' || buf[i] == '\t') {
            //         i++;
            //         continue;
            //     }
            //     username[j] = buf[i];
            //     i++;
            //     j++;
            // }   username[j] = '\0';
            // i++; j = 0;
            // while (buf[i] != '>') {
            //     // ignore the whitespace characters
            //     if (buf[i] == ' ' || buf[i] == '\t') {
            //         i++;
            //         continue;
            //     }
            //     domain_recv2[j] = buf[i];
            //     i++;
            //     j++;
            // }   domain_recv2[j] = '\0';

            memset(buf, 0, sizeof(buf)); sprintf(buf, "250 <%s@%s> ... Sender OK\r\n", username, domain_recv2);
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
            // check if the first four characters are RCPT
            if (strncmp(buf, "RCPT", 4) != 0) {
                memset(buf, 0, sizeof(buf)); sprintf(buf, "500 Syntax error: command unrecognized\r\n");
                send(newsockfd, buf, strlen(buf), 0);
                close(newsockfd);
                exit(0);
            }
            // extract the username and domain from the RCPT TO command
            char username_recp[MAX_USERNAME]; memset(username_recp, 0, sizeof(username_recp));
            char domain_recv_recp[MAX_DOMAIN]; memset(domain_recv_recp, 0, sizeof(domain_recv_recp));
            i = 0;
            while (buf[i] != '@' && i < strlen(buf)) i++;
            if (i == strlen(buf)) {
                memset(buf, 0, sizeof(buf)); sprintf(buf, "501 Syntax error in parameters or arguments\r\n");
                send(newsockfd, buf, strlen(buf), 0);
                close(newsockfd);
                exit(0);
            }
            else {
                int first_time_col = 0;
                int last_time_arrow = strlen(buf) - 1;
                int first_time_arrow = 0;
                int i_backup = i;
                i = 0;
                while (buf[i] != ':' && i<(strlen(buf)-1)) i++;
                first_time_col = i;
                i = 0;
                while (buf[i] != '<' && i<(strlen(buf)-1)) i++;
                first_time_arrow = i;
                i = strlen(buf) - 1;
                while (buf[i] != '>' && i > 0) i--;
                last_time_arrow = i;
                i = i_backup;
                i++; int j = 0;
                while (buf[i] != '\r' && buf[i] != '\n' && buf[i] != ' ' && buf[i] != '\t' && (i < last_time_arrow || last_time_arrow == 0)) {
                    domain_recv_recp[j] = buf[i];
                    i++;
                    j++;
                }   domain_recv_recp[j] = '\0';
                i = i_backup;
                j = 0;
                while (buf[i]!= ' ' && buf[i] != '\t' && i > first_time_col && (i > first_time_arrow || first_time_arrow == (strlen(buf)-1))) {
                    i--;
                }
                i++;
                while (i<i_backup) {
                    username_recp[j] = buf[i];
                    i++;
                    j++;
                }   username_recp[j] = '\0';
            }
            char path[MAX_PATH]; memset(path, 0, sizeof(path));
            strcat(path, "./");
            strcat(path, username_recp);
            if (access(path, F_OK) == -1) {
                memset(buf, 0, sizeof(buf)); sprintf(buf, "550 No such user here\r\n");
                send(newsockfd, buf, strlen(buf), 0);
                close(newsockfd);
                exit(0);
            }
            // some recipients checks !! ????
            memset(buf, 0, sizeof(buf)); sprintf(buf, "250 <%s@%s> ... Recipient OK\r\n", username_recp, domain_recv_recp);
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
            // check if the first four characters are DATA
            if (strncmp(buf, "DATA", 4) != 0) {
                memset(buf, 0, sizeof(buf)); sprintf(buf, "500 Syntax error: command unrecognized\r\n");
                send(newsockfd, buf, strlen(buf), 0);
                close(newsockfd);
                exit(0);
            }
            memset(buf, 0, sizeof(buf)); sprintf(buf, "354 Enter mail, end with \".\" on a line by itself\r\n");
            send(newsockfd, buf, strlen(buf), 0);
            char mail[MAX_MAIL]; memset(mail, 0, sizeof(mail));
            while (1) {
                char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                n = recv(newsockfd, temp_buf, MAX_BUFF, 0);
                strcat(mail, temp_buf);
                if (mail[strlen(mail)-5] == '\r' && mail[strlen(mail)-4] == '\n' && mail[strlen(mail)-3] == '.' && mail[strlen(mail)-2] == '\r' && mail[strlen(mail)-1] == '\n') {
                    break;
                }
            }
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            // add date hour and minute
            sprintf(recv_time, "%d-%02d-%02d %02d:%02d:%02d\r\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
            
            // check if the to and from fields are valid, to and from fields are username@domain
            char path_to[MAX_PATH]; memset(path_to, 0, sizeof(path_to));
            strcat(path_to, "./");
            strcat(path_to, username_recp);
            strcat(path_to, "/");
            strcat(path_to, "mymailbox");
            // char path_from[MAX_PATH]; memset(path_from, 0, sizeof(path_from));
            // strcat(path_from, "./");
            // strcat(path_from, domain_recv);
            // strcat(path_from, "/");
            // strcat(path_from, "mymailbox.txt");
            // append the mail to the mailbox of the recipient
            int fd_to = open(path_to, O_WRONLY | O_APPEND);
            if (fd_to == -1) {
                printf("Error in opening the file\n");
                exit(0);
            }
            char mail_to_write[strlen(mail) + 100]; memset(mail_to_write, 0, sizeof(mail_to_write));
            int j=0; int c = 0;
            for (i = 0; i < strlen(mail); i++)
            {
                if (c < 3 && mail[i] == '\r' && mail[i + 1] == '\n')
                {
                    if ( c < 2) c++;
                    else if (c == 2)
                    {
                        mail_to_write[j] = '\r'; mail_to_write[j+1] = '\n';
                        i++;
                        time_t t = time(NULL);
                        struct tm tm = *localtime(&t);
                        sprintf(recv_time, "Received: %d-%02d-%02d %02d:%02d:%02d\r\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                        strcat(mail_to_write, recv_time);
                        j=strlen(mail_to_write);
                        c++;
                        continue;
                    }
                }        
                mail_to_write[j] = mail[i];   
                j++;     
            }
            write(fd_to, mail_to_write, strlen(mail_to_write));     
            close(fd_to);
            memset(buf, 0, sizeof(buf)); sprintf(buf, "250 OK Message accepted for delivery\r\n");
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
            // check if the first four characters are QUIT
            if (strncmp(buf, "QUIT", 4) != 0) {
                memset(buf, 0, sizeof(buf)); sprintf(buf, "500 Syntax error: command unrecognized\r\n");
                send(newsockfd, buf, strlen(buf), 0);
                close(newsockfd);
                exit(0);
            }
            memset(buf, 0, sizeof(buf)); sprintf(buf, "221 <%s> Service closing transmission channel\r\n", domain);
            send(newsockfd, buf, strlen(buf), 0);
            close(newsockfd);
			exit(0);
		}
		close(newsockfd);
	}
	return 0;
}
