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
#define MAX_COMMAND 10
#define MAX_NO_MAIL 100
#define MAX_LINE_LENGTH 200

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
    char command[MAX_COMMAND];

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
                int n;
                char buf[MAX_BUFF]; memset(buf, 0, sizeof(buf));
                int i, j;

                int	sockfd ;
                struct sockaddr_in	serv_addr;
                if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                    perror("Unable to create socket\n");
                    exit(0);
                }
                serv_addr.sin_family	= AF_INET;
                inet_aton(server_ip, &serv_addr.sin_addr);
                serv_addr.sin_port	= htons(pop3_port);
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

                if (buf[0] != '+' || buf[1] != 'O' || buf[2] != 'K') {
                    printf("Error in connection %s", buf);
                    exit(0);
                }
                
                memset(buf, 0, sizeof(buf)); sprintf(buf, "USER %s\r\n", username);
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
                
                if (buf[0] != '+' || buf[1] != 'O' || buf[2] != 'K') {
                    // extract everything other than -ERR from buf
                    printf("Error: %s", buf);
                    close(sockfd); exit(0);
                }

                memset(buf, 0, sizeof(buf)); sprintf(buf, "PASS %s\r\n", password);
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
                if (buf[0] != '+' || buf[1] != 'O' || buf[2] != 'K') {
                    // extract everything other than -ERR from buf
                    printf("Error: %s", buf);
                    close(sockfd); exit(0);
                }

                memset(buf, 0, sizeof(buf)); sprintf(buf, "STAT\r\n");
                send(sockfd, buf, strlen(buf), 0);

                // printf("%s\n",buf);
                //         fflush(stdout);
                memset(buf, 0, sizeof(buf));
                while (1) {
                    char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                    n = recv(sockfd, temp_buf, MAX_BUFF, 0);
                    strcat(buf, temp_buf);
                    if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                        break;
                    }
                }
                // printf("%s\n", buf);
                // fflush(stdout);
                if (buf[0] != '+' || buf[1] != 'O' || buf[2] != 'K') {
                    printf("Error : %s", buf);
                    close(sockfd); exit(0);
                }
                i = 3; j = 0;
                int mail_count = 0;
                char mail_count_str[MAX_NO_MAIL]; memset(mail_count_str, 0, sizeof(mail_count_str));
                while (buf[i] == ' ' || buf[i] == '\t') i++;
                while (buf[i] != ' ' && buf[i] != '\t') {
                    mail_count_str[j] = buf[i];
                    j++;
                    i++;
                }   mail_count_str[j] = '\0';
                mail_count = atoi(mail_count_str);

                int mail_number = 0;
                int deleted = 0;
                do {
                    for ( int i_loop = 1; i_loop <= mail_count - deleted; i_loop++){
                        memset(buf, 0, sizeof(buf)); sprintf(buf, "RETR %d\r\n", i_loop);
                        send(sockfd, buf, strlen(buf), 0);

                        // printf("%s\n",buf);
                        // fflush(stdout);

                        memset(buf, 0, sizeof(buf));
                        char mail[MAX_MAIL]; memset(mail, 0, sizeof(mail));
                        while (1) {
                            char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                            n = recv(sockfd, temp_buf, MAX_BUFF, 0);
                            strcat(mail, temp_buf);
                            if (mail[strlen(mail)-5] == '\r' && mail[strlen(mail)-4] == '\n' && mail[strlen(mail)-3] == '.' && mail[strlen(mail)-2] == '\r' && mail[strlen(mail)-1] == '\n') {
                                break;
                            }
                        }
                        char temp_sender[MAX_LINE_LENGTH]; memset(temp_sender, 0, sizeof(temp_sender));
                        char temp_received[MAX_LINE_LENGTH]; memset(temp_received, 0, sizeof(temp_received));
                        char temp_subject[MAX_LINE_LENGTH];  memset(temp_subject, 0, sizeof(temp_subject));
                        char temp_receiver[MAX_LINE_LENGTH]; memset(temp_receiver, 0, sizeof(temp_receiver));
                        char *line;
                        int check_body = 0;
                        line = strtok(mail, "\r\n");
                        line = strtok(NULL, "\r\n");

                        while(line != NULL){
                            if(strstr(line, "From") && check_body < 4){
                                strcpy(temp_sender, line);
                                temp_sender[strlen(temp_sender)] = '\0';
                                check_body++;
                            }else if(strstr(line, "Received") && check_body < 4){
                                strcpy(temp_received, line);
                                temp_received[strlen(temp_received)] = '\0';
                                check_body++;
                            }else if(strstr(line, "Subject") && check_body < 4){
                                strcpy(temp_subject, line);
                                temp_subject[strlen(temp_subject)] = '\0';
                                check_body++;
                            }else if(strstr(line, "To") && check_body < 4){
                                strcpy(temp_receiver, line);
                                temp_receiver[strlen(temp_receiver)] = '\0';
                                check_body++;
                            }else if(strcmp(line, ".") == 0){
                            }
                            line = strtok(NULL, "\r\n");
                        }
                        char sender[MAX_LINE_LENGTH]; memset(sender, 0, sizeof(sender));
                        char received[MAX_LINE_LENGTH]; memset(received, 0, sizeof(received));
                        char subject[MAX_LINE_LENGTH];  memset(subject, 0, sizeof(subject));
                        char receiver[MAX_LINE_LENGTH]; memset(receiver, 0, sizeof(receiver));
                        int i = 0;
                        int j = 0;
                        while (temp_sender[i] != ':') i++; i++;
                        while (temp_sender[i] == ' ' || temp_sender[i] == '\t') i++;
                        while (temp_sender[i] != ' ' && temp_sender[i] != '\t' && temp_sender[i] != '\0') sender[j++] = temp_sender[i++];
                        i = 0; j = 0;
                        while (temp_received[i] != ':') i++; i++;
                        while (temp_received[i] == ' ' || temp_received[i] == '\t') i++;
                        while (i < strlen(temp_received)) received[j++] = temp_received[i++];
                        i = 0; j = 0;
                        while (temp_subject[i] != ':') i++; i++;
                        while (temp_subject[i] == ' ' || temp_subject[i] == '\t') i++;
                        while (i < strlen(temp_subject)) subject[j++] = temp_subject[i++];
                        i = 0; j = 0;
                        while (temp_receiver[i] != ':') i++; i++;
                        while (temp_receiver[i] == ' ' || temp_receiver[i] == '\t') i++;
                        while (temp_receiver[i] != ' ' && temp_receiver[i] != '\t' && temp_receiver[i] != '\0') receiver[j++] = temp_receiver[i++];
                        printf("%d\t\t%s\t\t%s\t\t%s\n", i_loop, sender, received, subject);
                    }
                    printf("Enter the mail number to see (-1 to exit this option): ");
                    scanf("%d", &mail_number);
                    char ch; while ( (ch = getchar()) != '\n' && ch != EOF) { /* discard characters */ } ch = '\0';
                    if (mail_number == -1) {
                        break;
                    }
                    if (mail_number > mail_count - deleted || mail_number < 1) {
                        printf("Mail number out of range, give again\n");
                        continue;
                    }
                    memset(buf, 0, sizeof(buf)); sprintf(buf, "RETR %d\r\n", mail_number);
                    send(sockfd, buf, strlen(buf), 0);
                    memset(buf, 0, sizeof(buf));
                    char mail[MAX_MAIL]; memset(mail, 0, sizeof(mail));
                    while (1) {
                        char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                        n = recv(sockfd, temp_buf, MAX_BUFF, 0);
                        strcat(mail, temp_buf);
                        if (mail[strlen(mail)-5] == '\r' && mail[strlen(mail)-4] == '\n' && mail[strlen(mail)-3] == '.' && mail[strlen(mail)-2] == '\r' && mail[strlen(mail)-1] == '\n') {
                            break;
                        }
                    }
                    printf("%s\n", mail);
                    printf("Enter a character (d to delete and any other to print the mails again): ");
                    ch = getchar();
                    if (ch == 'd'){
                        memset(buf, 0, sizeof(buf)); sprintf(buf, "DELE %d\r\n", mail_number);
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
                        if (buf[0] != '+' || buf[1] != 'O' || buf[2] != 'K') {
                            printf("Error: %s", buf);
                            close(sockfd); exit(0);
                        }
                        deleted++;
                    }
                    while ( (ch = getchar()) != '\n' && ch != EOF) { /* discard characters */ } ch = '\0';
                } while (mail_number != -1);

                // printf("%s\n", buf);
                // fflush(stdout);

                memset(buf, 0, sizeof(buf)); sprintf(buf, "QUIT\r\n");
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
                if (buf[0] != '+' || buf[1] != 'O' || buf[2] != 'K') {
                    printf("Error: %s", buf);
                    close(sockfd); exit(0);
                }
                printf("%s\n", buf);
                close(sockfd);
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
                //check that the subject is not empty
                if(strlen(subject)==0 ){
                    printf("Incorrect format, Subject should not be empty\n");
                    exit(0);
                }
                if(strlen(subject)>50){
                    printf("Incorrect format, Subject should not exceed 50 characters\n");
                    exit(0);
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
                memset(buf, 0, sizeof(buf)); sprintf(buf, "HELO %s\r\n", domain);
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

                memset(buf, 0, sizeof(buf)); sprintf(buf, "MAIL FROM: <%s@%s>\r\n", sender, send_domain);
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

                // printf("Enter the receiver's username: ");
                memset(buf, 0, sizeof(buf)); sprintf(buf, "RCPT TO:%s@%s\r\n", receiver, recv_domain);
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
                    // if (buf[strlen(buf)-3] == '.' && buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                    //     printf("CLRF found\n");
                    // }
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
                printf("Mail sent successfully\n");              
                close(sockfd);
                break;
            }
            case 3: {
                exit(0);
            }
        }
    } 
}