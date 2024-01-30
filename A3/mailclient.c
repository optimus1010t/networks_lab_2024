#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define MAX_BUFF 4000
#define MAX_DOMAIN 512
#define MAX_USERNAME 100
#define MAX_PASSWORD 100

// In accordance wirh RFC 5321 and RFC 1939

int main(int argc, char*argv[]) {
    char server_ip[100];
    int smtp_port, pop3_port;
    if (argc != 4) {
        printf("Usage: ./mailclient <server_ip> <smtp_port> <pop3_port>\n");
        exit(0);
    }
    strcpy(server_ip, argv[1]);
    smtp_port = atoi(argv[2]);
    pop3_port = atoi(argv[3]);
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    printf("Enter the username: ");
    fgets(username, sizeof(username), stdin);
    if (username[strlen(username) - 1] == '\n') {
        username[strlen(username) - 1] = '\0';
    }
    printf("Enter the password: ");
    fgets(password, sizeof(password), stdin);
    if (password[strlen(password) - 1] == '\n') {
        password[strlen(password) - 1] = '\0';
    }

    char domain[MAX_DOMAIN] = "iitkgp.edu\0";
    int choice;
    while (1)
	{
        printf("1. Manage Mail\n2. Send Mail\n 3. Quit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        switch(choice) 
        {
            case 1: {
                break;
            }
            case 2: {
                char buf[MAX_BUFF]; int n;

                int	sockfd ;
                struct sockaddr_in	serv_addr;
                if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                    perror("Unable to create socket\n");
                    exit(0);
                }
                serv_addr.sin_family	= AF_INET;
                inet_aton(server_ip, &serv_addr.sin_addr);
                serv_addr.sin_port	= htons(smtp_port);
                if ((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) {
                    perror("Unable to connect to server");
                    exit(0);
                }
                memset(buf, 0, sizeof(buf));
                while (1) {
                    char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                    n = recv(sockfd, temp_buf, MAX_BUFF, 0);
                    strcat(buf, temp_buf);
                    if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                        break;
                    }
                }
                printf("S:\t%s\n", buf);
                if (buf[0] != '2' || buf[1] != '2' || buf[2] != '0') {
                    printf("Error in connection\n");
                    exit(0);
                }
                int i = 0;
                while (buf[i] != '<') i++;
                i++; int j = 0;
                char domain_recv[MAX_DOMAIN];
                while (buf[i] != '>') {
                    if (buf[i] == ' ' || buf[i] == '\t') {
                        i++;
                        continue;
                    }
                    domain_recv[j] = buf[i];
                    i++;
                    j++;
                }   domain_recv[j] = '\0';
                memset(buf, 0, sizeof(buf)); fprintf(buf, "HELO <%s>\r\n", domain);
                send(sockfd, buf, strlen(buf), 0);
                memset(buf, 0, sizeof(buf));
                while (1) {
                    char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                    n = recv(sockfd, temp_buf, MAX_BUFF, 0);
                    strcat(buf, temp_buf);
                    if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                        break;
                    }
                }
                if (buf[0] == '5' && buf[1] == '0' && (buf[2] == '0' || buf[2] == '1')) {
                    printf("Error in sending mail: %s", buf);
                    exit(0);
                }
                if (buf[0] != '2' || buf[1] != '5' || buf[2] != '0') {
                    printf("Error in connection\n");
                    exit(0);
                }
                memset(buf, 0, sizeof(buf)); fprintf(buf, "MAIL FROM: <%s@%s>\r\n", username, domain);
                send(sockfd, buf, strlen(buf), 0);

                memset(buf, 0, sizeof(buf));
                while (1) {
                    char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                    n = recv(sockfd, temp_buf, MAX_BUFF, 0);
                    strcat(buf, temp_buf);
                    if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                        break;
                    }
                }
                if (buf[0] == '5' && buf[1] == '0' && (buf[2] == '0' || buf[2] == '1')) {
                    printf("Error in sending mail: %s", buf);
                    exit(0);
                }
                if (buf[0] != '2' || buf[1] != '5' || buf[2] != '0') {
                    printf("Error in connection\n");
                    exit(0);
                }
                char receiver[MAX_USERNAME];
                printf("Enter the receiver's username: ");
                fgets(receiver, sizeof(receiver), stdin);
                if (receiver[strlen(receiver) - 1] == '\n') {
                    receiver[strlen(receiver) - 1] = '\0';
                }
                memset(buf, 0, sizeof(buf)); fprintf(buf, "RCPT TO: <%s@%s>\r\n", receiver, domain);
                send(sockfd, buf, strlen(buf), 0);

                memset(buf, 0, sizeof(buf));
                while (1) {
                    char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                    n = recv(sockfd, temp_buf, MAX_BUFF, 0);
                    strcat(buf, temp_buf);
                    if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                        break;
                    }
                }
                if (buf[0] == '5' && buf[1] == '0' && (buf[2] == '0' || buf[2] == '1')) {
                    printf("Error in sending mail: %s", buf);
                    exit(0);
                }
                if (buf[0] != '5' || buf[1] != '5' || buf[2] != '0') {
                    printf("Error in sending mail: %s", buf);
                    exit(0);
                }
                if (buf[0] != '2' || buf[1] != '5' || buf[2] != '0') {
                    printf("Error in connection\n");
                    exit(0);
                }
                memset(buf, 0, sizeof(buf)); fprintf(buf, "DATA\r\n");
                send(sockfd, buf, strlen(buf), 0);

                memset(buf, 0, sizeof(buf));
                while (1) {
                    char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                    n = recv(sockfd, temp_buf, MAX_BUFF, 0);
                    strcat(buf, temp_buf);
                    if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                        break;
                    }
                }
                if (buf[0] == '5' && buf[1] == '0' && (buf[2] == '0' || buf[2] == '1')) {
                    printf("Error in sending mail: %s", buf);
                    exit(0);
                }
                if (buf[0] != '3' || buf[1] != '5' || buf[2] != '4') {
                    printf("Error in connection\n");
                    exit(0);
                }
                char subject[MAX_BUFF];
                printf("Enter the subject: ");
                fgets(subject, sizeof(subject), stdin);
                if (subject[strlen(subject) - 1] == '\n') {
                    subject[strlen(subject) - 1] = '\0';
                }
                memset(buf, 0, sizeof(buf)); fprintf(buf, "From: <%s@%s>\r\n", username, domain);
                send(sockfd, buf, strlen(buf), 0);
                memset(buf, 0, sizeof(buf)); fprintf(buf, "To: <%s@%s>\r\n", receiver, domain);
                send(sockfd, buf, strlen(buf), 0);
                memset(buf, 0, sizeof(buf)); fprintf(buf, "Subject: %s\r\n", subject);
                send(sockfd, buf, strlen(buf), 0);
                memset(buf, 0, sizeof(buf)); fprintf(buf, ".\r\n");
                send(sockfd, buf, strlen(buf), 0);

                memset(buf, 0, sizeof(buf));
                while (1) {
                    char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                    n = recv(sockfd, temp_buf, MAX_BUFF, 0);
                    strcat(buf, temp_buf);
                    if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                        break;
                    }
                }
                if (buf[0] == '5' && buf[1] == '0' && (buf[2] == '0' || buf[2] == '1')) {
                    printf("Error in sending mail: %s", buf);
                    exit(0);
                }
                if (buf[0] != '2' || buf[1] != '5' || buf[2] != '0') {
                    printf("Error in connection\n");
                    exit(0);
                }
                memset(buf, 0, sizeof(buf)); fprintf(buf, "QUIT\r\n");
                send(sockfd, buf, strlen(buf), 0);

                memset(buf, 0, sizeof(buf));
                while (1) {
                    char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                    n = recv(sockfd, temp_buf, MAX_BUFF, 0);
                    strcat(buf, temp_buf);
                    if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                        break;
                    }
                }
                if (buf[0] == '5' && buf[1] == '0' && (buf[2] == '0' || buf[2] == '1')) {
                    printf("Error in sending mail: %s", buf);
                    exit(0);
                }
                if (buf[0] != '2' || buf[1] != '2' || buf[2] != '1') {
                    printf("Error in connection\n");
                    exit(0);
                }                
                close(sockfd);
                break;
            }
            case 3: {
                exit(0);
            }
        }

    } 
}