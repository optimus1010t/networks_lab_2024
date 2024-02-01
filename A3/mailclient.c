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
#include <stdbool.h>

#define MAX_BUFF 4000
#define MAX_DOMAIN 512
#define MAX_USERNAME 100
#define MAX_PASSWORD 100
#define MAX_MAIL 6000
#define MAX_LINE 100
#define MAX_MAILID 650

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
        printf("1. Manage Mail\n2. Send Mail\n3. Quit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        switch(choice) 
        {
            case 1: {
                break;
            }
            case 2: {
                int n;
                char buf[MAX_BUFF]; memset(buf, 0, sizeof(buf));

                char mail_id_send[MAX_MAILID]; memset(mail_id_send, 0, sizeof(mail_id_send));
                char mail_id_recv[MAX_MAILID]; memset(mail_id_recv, 0, sizeof(mail_id_recv));
                char sender[MAX_USERNAME]; memset(sender, 0, sizeof(sender));
                char receiver[MAX_USERNAME]; memset(receiver, 0, sizeof(receiver));
                char subject[MAX_LINE]; memset(subject, 0, sizeof(subject));
                char send_domain[MAX_DOMAIN]; memset(send_domain, 0, sizeof(send_domain));
                char recv_domain[MAX_DOMAIN]; memset(recv_domain, 0, sizeof(recv_domain));
                char mail_body_line[MAX_LINE]; memset(mail_body_line, 0, sizeof(mail_body_line));
                char mail_body[MAX_MAIL]; memset(mail_body, 0, sizeof(mail_body));
                int line_count = 0;

                char ch; while ( (ch = getchar()) != '\n' && ch != EOF) { /* discard characters */ } ch = '\0';
                printf("From: ");
                fgets(mail_id_send, sizeof(mail_id_send), stdin);
                if (mail_id_send[strlen(mail_id_send) - 1] == '\n') {
                    mail_id_send[strlen(mail_id_send) - 1] = '\0';
                }
                // while ( (ch = getchar()) != '\n' && ch != EOF) { /* discard characters */ } ch = '\0';
                printf("To: ");
                fgets(mail_id_recv, sizeof(mail_id_recv), stdin);
                if (mail_id_recv[strlen(mail_id_recv) - 1] == '\n') {
                    mail_id_recv[strlen(mail_id_recv) - 1] = '\0';
                }
                printf("Subject: ");
                // while ((ch = getchar()) != '\n' && ch != EOF) { /* discard characters */ } ch = '\0';
                fgets(subject, sizeof(subject), stdin);
                if (subject[strlen(subject) - 1] == '\n') {
                    subject[strlen(subject) - 1] = '\0';
                }
                printf("Body: ");
                // while ((ch = getchar()) != '\n' && ch != EOF) { /* discard characters */ } ch = '\0';
                int flag = 1;
                while(1) {
                    // while ((ch = getchar()) != '\n' && ch != EOF) { /* discard characters */ } ch = '\0';
                    fgets(mail_body_line, sizeof(mail_body_line), stdin);
                    line_count++;
                    if (mail_body_line[strlen(mail_body_line) - 1] == '\n') {
                        mail_body_line[strlen(mail_body_line) - 1] = '\0';
                    }
                    // check that each line must not exceed 80 characters
                    if (strlen(mail_body_line) > 80) {
                        printf("Incorrect Format: Line too long, each line must not exceed 80 characters\n");
                        flag = 0;
                        break;
                    }
                    if (line_count >= 51){
                        printf("Incorrect Format: Body too long, it cannot contain more than 50 lines\n");
                        flag = 0;
                        break;
                    }
                    if (strcmp(mail_body_line, ".") == 0) {
                        strcat(mail_body, ".\r\n");
                        break;
                    }
                    strcat(mail_body, mail_body_line);
                    strcat(mail_body, "\r\n");
                }
                if (flag == 0) {
                    break;
                }
                int atCount = 0;
                int length = strlen(mail_id_send);
                int atIndex = -1;
                // Count @ and remember its position
                // Skip initial whites spaces
                int i = 0; flag = 1;
                while (mail_id_send[i] == ' ' || mail_id_send[i] == '\t') i++;
                int start = i;
                for (; i < length; ++i) {
                    // Cant have whitespaces in between
                    if (mail_id_send[i] == ' ' || mail_id_send[i] == '\t') {
                        printf("Incorrect Format: Whitespaces in between the senders mail id\n");
                        flag = 0;
                        break;
                    }
                    if (mail_id_send[i] == '@') {
                        atCount++;
                        atIndex = i;
                    }
                }
                if (flag == 0) {
                    break;
                }
                if (!((atCount == 1) && (atIndex > start) && (atIndex < length - 1)))
                {
                    printf("Incorrect Format: Incorrect senders domain\n");
                    break;
                }

                atCount = 0;
                length = strlen(mail_id_recv);
                atIndex = -1;

                i = 0; flag = 1;
                while (mail_id_recv[i] == ' ' || mail_id_recv[i] == '\t') i++;
                start = i;
                for (; i < length; ++i) {
                    if (mail_id_recv[i] == ' ' || mail_id_recv[i] == '\t') {
                        printf("Incorrect Format: Whitespaces in between the receivers mail id\n");
                        flag = 0;
                        break;
                    }
                    if (mail_id_recv[i] == '@') {
                        atCount++;
                        atIndex = i;
                    }
                }
                if (flag == 0) {
                    break;
                }
                if (!((atCount == 1) && (atIndex > start) && (atIndex < length - 1)))
                {
                    printf("Incorrect Format: Incorrect receivers domain\n");
                    break;
                }
                // extract sender and send_domain from mail_id_send
                i = 0;
                // ignore initial spaces
                while (mail_id_send[i] == ' ' || mail_id_send[i] == '\t') i++;
                int j = 0;
                while (mail_id_send[i] != '@') {
                    sender[j] = mail_id_send[i];
                    i++; j++;
                }   sender[j] = '\0';
                i++; j=0;
                while (mail_id_send[i] != ' ' && mail_id_send[i] != '\t' && i < strlen(mail_id_send) && mail_id_send[i] != '\0'){
                    send_domain[j] = mail_id_send[i];
                    i++;
                    j++;
                }   send_domain[j] = '\0';
                // extract receiver and recv_domain from mail_id_recv
                i = 0; j=0;
                // ignore initial spaces
                while (mail_id_recv[i] == ' ' || mail_id_recv[i] == '\t') i++;
                while (mail_id_recv[i] != '@') {
                    receiver[j] = mail_id_recv[i];
                    i++; j++;
                }   receiver[j] = '\0';
                i++;
                j = 0;
                while (mail_id_recv[i] != ' ' && mail_id_recv[i] != '\t' && i < strlen(mail_id_recv) && mail_id_recv[i] != '\0') {
                    recv_domain[j] = mail_id_recv[i];
                    i++;
                    j++;
                }   recv_domain[j] = '\0';

                printf("sender: %s\n", sender);
                printf("send_domain: %s\n", send_domain);
                printf("receiver: %s\n", receiver);
                printf("recv_domain: %s\n", recv_domain);
                printf("subject: %s\n", subject);
                printf("mail_body: %s\n", mail_body);

                
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
                    printf("\t%s\n", buf);
                if (buf[0] != '2' || buf[1] != '2' || buf[2] != '0') {
                    printf("Error in connection\n");
                    exit(0);
                }
                i = 0;
                while (buf[i] != '<') i++;
                i++; j = 0;
                char domain_serv_recv[MAX_DOMAIN];
                while (buf[i] != '>') {
                    if (buf[i] == ' ' || buf[i] == '\t') {
                        i++;
                        continue;
                    }
                    domain_serv_recv[j] = buf[i];
                    i++;
                    j++;
                }   domain_serv_recv[j] = '\0';
                memset(buf, 0, sizeof(buf)); sprintf(buf, "HELO <%s>\r\n", domain);
                send(sockfd, buf, strlen(buf), 0);
                    printf("\t%s\n", buf);
                memset(buf, 0, sizeof(buf));
                while (1) {
                    char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                    n = recv(sockfd, temp_buf, MAX_BUFF, 0);
                    strcat(buf, temp_buf);
                    if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                        break;
                    }
                }
                    printf("\t%s\n", buf);
                if (buf[0] == '5' && buf[1] == '0' && (buf[2] == '0' || buf[2] == '1')) {
                    printf("Error in sending mail: %s", buf);
                    exit(0);
                }
                if (buf[0] != '2' || buf[1] != '5' || buf[2] != '0') {
                    printf("Error in connection\n");
                    exit(0);
                }
                memset(buf, 0, sizeof(buf)); sprintf(buf, "MAIL FROM: <%s@%s>\r\n", sender, send_domain);
                send(sockfd, buf, strlen(buf), 0);
                    printf("\t%s\n", buf);
                memset(buf, 0, sizeof(buf));
                while (1) {
                    char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                    n = recv(sockfd, temp_buf, MAX_BUFF, 0);
                    strcat(buf, temp_buf);
                    if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                        break;
                    }
                }
                    printf("\t%s\n", buf);
                if (buf[0] == '5' && buf[1] == '0' && (buf[2] == '0' || buf[2] == '1')) {
                    printf("Error in sending mail: %s", buf);
                    exit(0);
                }
                if (buf[0] != '2' || buf[1] != '5' || buf[2] != '0') {
                    printf("Error in connection\n");
                    exit(0);
                }
                // printf("Enter the receiver's username: ");
                memset(buf, 0, sizeof(buf)); sprintf(buf, "RCPT TO: <%s@%s>\r\n", receiver, recv_domain);
                send(sockfd, buf, strlen(buf), 0);
                    printf("\t%s\n", buf);
                memset(buf, 0, sizeof(buf));
                while (1) {
                    char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                    n = recv(sockfd, temp_buf, MAX_BUFF, 0);
                    strcat(buf, temp_buf);
                    if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                        break;
                    }
                }
                    printf("\t%s\n", buf);
                if (buf[0] == '5' && buf[1] == '0' && (buf[2] == '0' || buf[2] == '1')) {
                    printf("Error in sending mail: %s", buf);
                    exit(0);
                }
                if (buf[0] == '5' && buf[1] == '5' && buf[2] == '0') {
                    printf("Error in sending mail: %s", buf);
                    exit(0);
                }
                if (buf[0] != '2' || buf[1] != '5' || buf[2] != '0') {
                    printf("Error in connection\n");
                    exit(0);
                }
                memset(buf, 0, sizeof(buf)); sprintf(buf, "DATA\r\n");
                send(sockfd, buf, strlen(buf), 0);
                    printf("\t%s\n", buf);
                memset(buf, 0, sizeof(buf));
                while (1) {
                    char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                    n = recv(sockfd, temp_buf, MAX_BUFF, 0);
                    strcat(buf, temp_buf);
                    if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                        break;
                    }
                }
                    printf("\t%s\n", buf);
                if (buf[0] == '5' && buf[1] == '0' && (buf[2] == '0' || buf[2] == '1')) {
                    printf("Error in sending mail: %s", buf);
                    exit(0);
                }
                if (buf[0] != '3' || buf[1] != '5' || buf[2] != '4') {
                    printf("Error in connection\n");
                    exit(0);
                }
                // printf("Enter the subject: ");
                memset(buf, 0, sizeof(buf)); sprintf(buf, "From: %s@%s\r\n", sender, send_domain);
                send(sockfd, buf, strlen(buf), 0);
                memset(buf, 0, sizeof(buf)); sprintf(buf, "To: %s@%s\r\n", receiver, recv_domain);
                send(sockfd, buf, strlen(buf), 0);
                memset(buf, 0, sizeof(buf)); sprintf(buf, "Subject: %s\r\n", subject);
                send(sockfd, buf, strlen(buf), 0);
                // start sending the mail body in chunks of size buf
                int len = strlen(mail_body);
                start = 0;
                while (start < len) {
                    memset(buf, 0, sizeof(buf));
                    int i = 0;
                    while (i < MAX_BUFF && start < len) {
                        buf[i] = mail_body[start];
                        i++;
                        start++;
                    }
                    send(sockfd, buf, strlen(buf), 0);
                    if (buf[strlen(buf)-3] == '.' && buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                        printf("CLRF found\n");
                    }
                    printf("\t%s\n", buf);
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
                    printf("\t%s\n", buf);
                if (buf[0] == '5' && buf[1] == '0' && (buf[2] == '0' || buf[2] == '1')) {
                    printf("Error in sending mail: %s", buf);
                    exit(0);
                }
                if (buf[0] != '2' || buf[1] != '5' || buf[2] != '0') {
                    printf("Error in connection\n");
                    exit(0);
                }
                memset(buf, 0, sizeof(buf)); sprintf(buf, "QUIT\r\n");
                send(sockfd, buf, strlen(buf), 0);
                    printf("\t%s\n", buf);
                memset(buf, 0, sizeof(buf));
                while (1) {
                    char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                    n = recv(sockfd, temp_buf, MAX_BUFF, 0);
                    strcat(buf, temp_buf);
                    if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                        break;
                    }
                }
                    printf("\t%s\n", buf);
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