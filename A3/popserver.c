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
#define MAX_LINE_LENGTH 200
#define MAX_NO_MAIL 100

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

            // printf("%s\n",buf);
            // fflush(stdout);
            
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

            // printf("%s\n",buf);
            // fflush(stdout);

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
            

            FILE* user_file = fopen("./user.txt", "r");
            if (user_file == NULL) {
                printf("Error opening file.\n");
                exit(0);
            }
            int flag = 0;
            char temp_username[MAX_USERNAME]; memset(temp_username, 0, sizeof(temp_username));
            char temp_password[MAX_PASSWORD]; memset(temp_password, 0, sizeof(temp_password));

            char line[MAX_LINE_LENGTH];
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

            // printf("%s\n",buf);
            // fflush(stdout);

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

            FILE *mail_file;
            memset(line, 0, sizeof(line));

            // Open the file
            strcat(path, "/mymailbox");
            mail_file = fopen(path, "r");
            if (mail_file == NULL) {
                memset(buf, 0, sizeof(buf)); sprintf(buf, "-ERR Opening mailbox\r\n");
                send(newsockfd, buf, strlen(buf), 0);
                close(newsockfd);
                exit(0);
            }
            fclose(mail_file);
            memset(buf, 0, sizeof(buf)); sprintf(buf, "+OK maildrop ready\r\n");
            send(newsockfd, buf, strlen(buf), 0);

            // printf("%s\n",buf);
            // fflush(stdout);     

            memset(buf, 0, sizeof(buf));
			while (1) {
                char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                n = recv(newsockfd, temp_buf, MAX_BUFF, 0);
                strcat(buf, temp_buf);
                if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                    break;
                }
            }

            // printf("%s\n",buf);
            // fflush(stdout);

            char sender[MAX_LINE_LENGTH]; memset(sender, 0, sizeof(sender));
            char receiver[MAX_LINE_LENGTH]; memset(receiver, 0, sizeof(receiver));
            char received[MAX_LINE_LENGTH]; memset(received, 0, sizeof(received));
            char subject[MAX_LINE_LENGTH];  memset(subject, 0, sizeof(subject));
            char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
            // Read the file line by line
            int serial_number = 0;
            int loop_in=0;
            int check_inbody = 0;
            mail_file = fopen(path, "r");
            while (fgets(line,sizeof(line),mail_file)!=NULL) {
                // line[strcspn(line, "\n")] = 0;
                if (strcmp(line, ".\r\n") == 0 /* || (strcmp(line, ".\n")==0 && strlen(line)==2) */) {
                    if(strlen(sender)>0 && strlen(received)>0 && strlen(subject)>0 && strlen(receiver)>0){
                        serial_number++;                       
                    }
                    check_inbody = 0;
                    memset(sender, 0, sizeof(sender));
                    memset(receiver, 0, sizeof(receiver));
                    memset(received, 0, sizeof(received));
                    memset(subject, 0, sizeof(subject));
                    memset(temp_buf, 0, sizeof(temp_buf));
                } else {
                    if (strncmp(line, "To", 2) == 0 && check_inbody < 4) {
                        strcpy(receiver, line);
                        check_inbody++;                                
                    } else if (strncmp(line, "From", 4) == 0 && check_inbody < 4) {
                        strcpy(sender, line);
                        check_inbody++;                                
                    }
                    else if (strncmp(line, "Received", 8) == 0 && check_inbody < 4) {
                        strcpy(received, line);
                        check_inbody++;
                    }
                    else if (strncmp(line, "Subject", 7) == 0 && check_inbody < 4) {
                        strcpy(subject, line);
                        check_inbody++;
                    }
                    else {
                        strcat(temp_buf, line);
                    }
                }
                memset(line, 0, sizeof(line));
                loop_in++;
            }
            fclose(mail_file);
            int deleted_msgs[serial_number+1];
            for (int i = 0; i <= serial_number; i++) {
                deleted_msgs[i] = 0;
            }
            int total_mails = serial_number;

            // printf("%d\n",total_mails);
            // fflush(stdout);

            int body_length[total_mails+1]; for (int i = 0; i <= serial_number; i++) body_length[i] = 0;
            int from_length[total_mails+1]; for (int i = 0; i <= serial_number; i++) from_length[i] = 0;
            int to_length[total_mails+1]; for (int i = 0; i <= serial_number; i++) to_length[i] = 0;
            int subject_length[total_mails+1]; for (int i = 0; i <= serial_number; i++) subject_length[i] = 0;
            int received_length[total_mails+1]; for (int i = 0; i <= serial_number; i++) received_length[i] = 0;
            long int total_length[total_mails+1]; for (int i = 0; i <= serial_number; i++) total_length[i] = 0;            

            memset(sender, 0, sizeof(sender));
            memset(receiver, 0, sizeof(receiver));
            memset(received, 0, sizeof(received));
            memset(subject, 0, sizeof(subject));
            memset(temp_buf, 0, sizeof(temp_buf));
            check_inbody = 0;
            long int totalcharacterCount = 0;
            long int indv_count = 0;
            serial_number = 0;
            
            mail_file = fopen(path, "r");
            while (fgets(line,sizeof(line),mail_file)!=NULL) {
                // line[strcspn(line, "\n")] = 0;
                if (strcmp(line, ".\r\n") == 0  /*|| (strcmp(line, ".\n")==0 && strlen(line)==2 )*/) {
                    if(strlen(sender)>0 && strlen(received)>0 && strlen(subject)>0 && strlen(receiver)>0){
                        serial_number++;
                        total_length[serial_number] = indv_count+3;
                        totalcharacterCount += indv_count+3;                        
                    }
                    check_inbody = 0;
                    memset(sender, 0, sizeof(sender));
                    memset(receiver, 0, sizeof(receiver));
                    memset(received, 0, sizeof(received));
                    memset(subject, 0, sizeof(subject));
                    memset(temp_buf, 0, sizeof(temp_buf));
                    indv_count = 0;
                } else {
                    if (strncmp(line, "To", 2) == 0 && check_inbody < 4) {
                        strcpy(receiver, line);
                        to_length[serial_number+1] = strlen(line);
                        indv_count += strlen(line);
                        check_inbody++;                                
                    } else if (strncmp(line, "From", 4) == 0 && check_inbody < 4) {
                        strcpy(sender, line);
                        from_length[serial_number+1] = strlen(line);
                        indv_count += strlen(line);
                        check_inbody++;                                
                    }
                    else if (strncmp(line, "Received", 8) == 0 && check_inbody < 4) {
                        strcpy(received, line);
                        received_length[serial_number+1] = strlen(line);
                        indv_count += strlen(line);
                        check_inbody++;
                    }
                    else if (strncmp(line, "Subject", 7) == 0 && check_inbody < 4) {
                        strcpy(subject, line);
                        subject_length[serial_number+1] = strlen(line);
                        indv_count += strlen(line);
                        check_inbody++;
                    }
                    else {
                        body_length[serial_number+1] += strlen(line);
                        indv_count += strlen(line);
                        strcat(temp_buf, line);
                    }
                }
                memset(line, 0, sizeof(line));
                loop_in++;
            }
            fclose(mail_file);

            while (strncmp(buf, "QUIT", 4) != 0){
                if (strncmp(buf, "STAT", 4) == 0) {
                    int count = 0;
                    long int total_char_deleted = 0;
                    for (int i = 1; i <= total_mails; i++) {
                        if (deleted_msgs[i] != 0) {
                            total_char_deleted += total_length[i];
                            count++;
                        }
                    }
                    memset(buf, 0, sizeof(buf)); sprintf(buf, "+OK %d %ld\r\n", total_mails - count, totalcharacterCount - total_char_deleted);
                    send(newsockfd, buf, strlen(buf), 0);

                    // printf("%s\n",buf);
                    // fflush(stdout);
                }
                else if (strncmp(buf, "LIST", 4) == 0) {
                    i = 4;

                    // printf("%s\n",buf);
                    // fflush(stdout);

                    while (i < strlen(buf) && (buf[i] == ' ' || buf[i] == '\t')) i++;
                    if (buf[i] == '\r' && buf[i+1] == '\n'){
                        
                        int count = 0;
                        long int total_char_deleted = 0;
                        for (int i = 1; i <= total_mails; i++) {
                            if (deleted_msgs[i] != 0) {
                                total_char_deleted += total_length[i];
                                count++;
                            }
                        }
                        memset(buf, 0, sizeof(buf)); sprintf(buf, "+OK %d messages (%ld octets)\r\n", total_mails - count, totalcharacterCount - total_char_deleted);
                        send(newsockfd, buf, strlen(buf), 0);

                        // printf("%s\n",buf);
                        // fflush(stdout);

                        int non_deleted_msgs = 0;
                        for (int i = 1; i <= total_mails; i++) {
                            if (deleted_msgs[i] == 0) {
                                non_deleted_msgs++;
                                memset(buf, 0, sizeof(buf)); sprintf(buf, "%d %ld\r\n", non_deleted_msgs, total_length[i]);
                                send(newsockfd, buf, strlen(buf), 0);

                                // printf("%s\n",buf);
                                // fflush(stdout);

                            }
                        }
                        memset(buf, 0, sizeof(buf)); sprintf(buf, ".\r\n");
                        send(newsockfd, buf, strlen(buf), 0);

                        // printf("%s\n",buf);
                        // fflush(stdout);
                    }
                    else {
                        int msg_number = 0; j = 0 ;
                        char msg_number_str[MAX_NO_MAIL]; memset(msg_number_str, 0, sizeof(msg_number_str));
                        while (i < strlen(buf) && buf[i] != '\r' && buf[i] != '\n' && buf[i] != ' ' && buf[i] != '\t') {
                            msg_number_str[j++] = buf[i];
                            i++;
                        }   msg_number_str[j] = '\0';
                        msg_number = atoi(msg_number_str);

                        // printf("message no. : %d", msg_number);
                        // fflush(stdout);

                        int non_deleted_msgs = 0;
                        for (int i = 1; i <= total_mails; i++) {
                            if (deleted_msgs[i] == 0) {
                                non_deleted_msgs++;
                                if (non_deleted_msgs == msg_number) {
                                    memset(buf, 0, sizeof(buf)); sprintf(buf, "+OK %d %ld\r\n", msg_number, total_length[i]);
                                    send(newsockfd, buf, strlen(buf), 0);
                                    break;
                                }
                            }
                        }
                        if (msg_number < 1 || msg_number > non_deleted_msgs) {
                            memset(buf, 0, sizeof(buf)); sprintf(buf, "-ERR no such message, only %d messages in maildrop starting from 1 (if there is one)\r\n", non_deleted_msgs);
                        }
                        // printf("%s\n",buf);
                        // fflush(stdout);
                    }
                }
                else if (strncmp(buf, "RETR", 4) == 0) {
                    i = 4;
                    while (i < strlen(buf) && (buf[i] == ' ' || buf[i] == '\t')) i++;
                    if (i >= strlen(buf) || (buf[i] == '\r' && buf[i+1] == '\n')) {
                        memset(buf, 0, sizeof(buf)); sprintf(buf, "-ERR Syntax error: command unrecognized\r\n");
                        send(newsockfd, buf, strlen(buf), 0);
                        exit(0);
                    }
                    else {
                        int msg_number = 0; j = 0 ;
                        char msg_number_str[MAX_NO_MAIL]; memset(msg_number_str, 0, sizeof(msg_number_str));
                        while (buf[i] != '\r' && buf[i] != '\n' && buf[i] != ' ' && buf[i] != '\t') {
                            msg_number_str[j++] = buf[i];
                            i++;
                        }   msg_number_str[j] = '\0';
                        msg_number = atoi(msg_number_str);
                        int non_deleted_msgs = 0;
                        for (int i = 1; i <= total_mails; i++) {
                            if (deleted_msgs[i] == 0) {
                                non_deleted_msgs++;
                                if (non_deleted_msgs == msg_number) {
                                    memset(buf, 0, sizeof(buf)); sprintf(buf, "+OK %ld octets\r\n", total_length[i]);
                                    send(newsockfd, buf, strlen(buf), 0);
                                    break;
                                }
                            }
                        }
                        if (msg_number < 1 || msg_number > non_deleted_msgs) {
                            memset(buf, 0, sizeof(buf)); sprintf(buf, "-ERR no such message, only %d messages in maildrop starting from 1 (if there is one)\r\n", non_deleted_msgs);
                            send(newsockfd, buf, strlen(buf), 0);
                            exit(0);
                        }
                        memset(sender, 0, sizeof(sender));
                        memset(receiver, 0, sizeof(receiver));
                        memset(received, 0, sizeof(received));
                        memset(subject, 0, sizeof(subject));
                        memset(temp_buf, 0, sizeof(temp_buf));
                        serial_number = 0;
                        check_inbody = 0;
                        non_deleted_msgs = 0;
                        mail_file = fopen(path, "r");
                        while (fgets(line,sizeof(line),mail_file)!=NULL) {
                            // line[strcspn(line, "\n")] = 0;
                            if (strcmp(line, ".\r\n") == 0) {
                                if(strlen(sender)>0 && strlen(received)>0 && strlen(subject)>0 && strlen(receiver)>0){
                                    serial_number++;
                                    strcat (temp_buf, ".\r\n");
                                    if (deleted_msgs[serial_number] == 0) {
                                        non_deleted_msgs++;
                                        if (non_deleted_msgs == msg_number) {
                                            break;
                                        }                                       
                                    }
                                }
                                check_inbody = 0;
                                memset(sender, 0, sizeof(sender));
                                memset(receiver, 0, sizeof(receiver));
                                memset(received, 0, sizeof(received));
                                memset(subject, 0, sizeof(subject));
                                memset(temp_buf, 0, sizeof(temp_buf));
                            } else {
                                if (strncmp(line, "To", 2) == 0 && check_inbody < 4) {
                                    strcpy(receiver, line);
                                    // strcat(receiver, "\r\n");
                                    check_inbody++;                                
                                } else if (strncmp(line, "From", 4) == 0 && check_inbody < 4) {
                                    strcpy(sender, line);
                                    // strcat(sender, "\r\n");
                                    check_inbody++;                                
                                }
                                else if (strncmp(line, "Received", 8) == 0 && check_inbody < 4) {
                                    strcpy(received, line);
                                    // strcat(received, "\r\n");
                                    check_inbody++;
                                }
                                else if (strncmp(line, "Subject", 7) == 0 && check_inbody < 4) {
                                    strcpy(subject, line);
                                    // strcat(subject, "\r\n");
                                    check_inbody++;
                                }
                                else {
                                    strcat(temp_buf, line);
                                    // strcat(temp_buf, "\r\n");
                                }
                            }
                            memset(line, 0, sizeof(line));
                            loop_in++;
                        }
                        fclose(mail_file);
                        memset(buf, 0, sizeof(buf)); sprintf(buf, "%s", sender);
                        send(newsockfd, buf, strlen(buf), 0);
                        memset(buf, 0, sizeof(buf)); sprintf(buf, "%s", receiver);
                        send(newsockfd, buf, strlen(buf), 0);
                        memset(buf, 0, sizeof(buf)); sprintf(buf, "%s", subject);
                        send(newsockfd, buf, strlen(buf), 0);
                        memset(buf, 0, sizeof(buf)); sprintf(buf, "%s", received);
                        send(newsockfd, buf, strlen(buf), 0);
                        memset(buf, 0, sizeof(buf)); sprintf(buf, "%s", temp_buf);
                        send(newsockfd, buf, strlen(buf), 0);
                    }
                }
                else if (strncmp(buf, "DELE", 4) == 0) {
                    i = 4;
                    while (i < strlen(buf) && (buf[i] == ' ' || buf[i] == '\t')) i++;
                    if (i >= strlen(buf) || (buf[i] == '\r' && buf[i+1] == '\n')) {
                        memset(buf, 0, sizeof(buf)); sprintf(buf, "-ERR Syntax error: command unrecognized\r\n");
                        send(newsockfd, buf, strlen(buf), 0);
                        exit(0);
                    }
                    else {
                        int msg_number = 0; j = 0 ;
                        char msg_number_str[MAX_NO_MAIL]; memset(msg_number_str, 0, sizeof(msg_number_str));
                        while (buf[i] != '\r' && buf[i] != '\n' && buf[i] != ' ' && buf[i] != '\t') {
                            msg_number_str[j++] = buf[i];
                            i++;
                        }   msg_number_str[j] = '\0';
                        msg_number = atoi(msg_number_str);
                        int non_deleted_msgs = 0;
                        for (int i = 1; i <= total_mails; i++) {
                            if (deleted_msgs[i] == 0) {
                                non_deleted_msgs++;
                                if (non_deleted_msgs == msg_number) {
                                    deleted_msgs[i] = 1;
                                    memset(buf, 0, sizeof(buf)); sprintf(buf, "+OK message %d deleted\r\n", msg_number);
                                    send(newsockfd, buf, strlen(buf), 0);
                                    break;
                                }
                            }
                        }
                        if (msg_number < 1 || msg_number > non_deleted_msgs) {
                            memset(buf, 0, sizeof(buf)); sprintf(buf, "-ERR no such message, only %d messages in maildrop starting from 1 (if there is one)\r\n", non_deleted_msgs);
                            send(newsockfd, buf, strlen(buf), 0);
                            exit(0);
                        }
                    }
                }
                else if (strncmp(buf, "RSET", 4) == 0) {
                    i = 1;
                    for (int i = 0; i <= total_mails; i++) {
                        deleted_msgs[i] = 0;
                    }
                    memset(buf, 0, sizeof(buf)); sprintf(buf, "+OK maildrop has %d messages (%ld octets)\r\n", total_mails, totalcharacterCount);
                    send(newsockfd, buf, strlen(buf), 0);
                }
                else if (strncmp(buf, "NOOP", 4) == 0) {
                    memset(buf, 0, sizeof(buf)); sprintf(buf, "+OK\r\n");
                    send(newsockfd, buf, strlen(buf), 0);
                }
                memset(buf, 0, sizeof(buf));
                while (1) {
                    char temp_buf[MAX_BUFF]; memset(temp_buf, 0, sizeof(temp_buf));
                    n = recv(newsockfd, temp_buf, MAX_BUFF, 0);
                    strcat(buf, temp_buf);
                    if (buf[strlen(buf)-2] == '\r' && buf[strlen(buf)-1] == '\n') {
                        break;
                    }
                }
            }
            int deleted_msgs_count = 0;
            for (int i = 1; i <= total_mails; i++) {
                if (deleted_msgs[i] != 0) {
                    deleted_msgs_count++;
                }
            }
            if (deleted_msgs_count > 0)
            {
                char path_temp[MAX_PATH]; memset(path_temp, 0, sizeof(path_temp));
                strcat(path_temp, "./");
                strcat(path_temp, username);
                strcat(path_temp, "/temp");
                FILE *tempFile = fopen(path_temp, "w");
                memset(sender, 0, sizeof(sender));
                memset(receiver, 0, sizeof(receiver));
                memset(received, 0, sizeof(received));
                memset(subject, 0, sizeof(subject));
                memset(temp_buf, 0, sizeof(temp_buf));
                check_inbody = 0;
                serial_number = 0;
                mail_file = fopen(path, "r");
                while (fgets(line,sizeof(line),mail_file)!=NULL) {
                    // line[strcspn(line, "\n")] = 0;
                    if (strcmp(line, ".\r\n") == 0) {
                        if(strlen(sender)>0 && strlen(received)>0 && strlen(subject)>0 && strlen(receiver)>0){
                            serial_number++;                       
                        }
                        if (deleted_msgs[serial_number] == 0) {
                            fprintf(tempFile, "%s", sender);
                            fprintf(tempFile, "%s", receiver);
                            fprintf(tempFile, "%s", received);
                            fprintf(tempFile, "%s", subject);
                            fprintf(tempFile, "%s", temp_buf);
                            fprintf(tempFile, ".\r\n");
                        }
                        check_inbody = 0;
                        memset(sender, 0, sizeof(sender));
                        memset(receiver, 0, sizeof(receiver));
                        memset(subject, 0, sizeof(subject));
                        memset(received, 0, sizeof(received));
                        memset(temp_buf, 0, sizeof(temp_buf));
                    } else {
                        if (strncmp(line, "To", 2) == 0 && check_inbody < 4) {
                            strcpy(receiver, line);
                            // strcat(receiver, "\r\n");
                            check_inbody++;                                
                        } else if (strncmp(line, "From", 4) == 0 && check_inbody < 4) {
                            strcpy(sender, line);
                            // strcat(sender, "\r\n");
                            check_inbody++;                                
                        }
                        else if (strncmp(line, "Received", 8) == 0 && check_inbody < 4) {
                            strcpy(received, line);
                            // strcat(received, "\r\n");
                            check_inbody++;
                        }
                        else if (strncmp(line, "Subject", 7) == 0 && check_inbody < 4) {
                            strcpy(subject, line);
                            // strcat(subject, "\r\n");
                            check_inbody++;
                        }
                        else {
                            strcat(temp_buf, line);
                            // strcat(temp_buf, "\r\n");
                        }
                    }
                    memset(line, 0, sizeof(line));
                    loop_in++;
                }
                fclose(mail_file);
                fclose(tempFile);
                remove(path);
                rename(path_temp, path);
            }
            memset(buf, 0, sizeof(buf)); sprintf(buf, "+OK POP3 server signing off\r\n"); 
            send(newsockfd, buf, strlen(buf), 0);           
            close(newsockfd);
			exit(0);
		}
		close(newsockfd);
	}
	return 0;
}
