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
#define OFFSET 0
#define NO_OF_CLIENTS 2
#define TABLE_SIZE 3
#define MAX_FRIEND_NAME 100

int main(int argc, char*argv[])
{
    int ports[TABLE_SIZE];
    ports[0]=50000+OFFSET;
    ports[1]=50001+OFFSET;
    ports[2]=50002+OFFSET;

    char *names[TABLE_SIZE];
    names[0]="A"; // changes
    names[1]="B";
    names[2]="C";

    char *ip = "127.0.0.1";

    // find the index of port that matches the ports
    int my_index = -1;
    for (int i = 0; i < 3; i++) {
        if (ports[i] == atoi(argv[1])) {
            my_index = i;
            break;
        }
    }

    int my_port;
    if ( argc != 2) {
        printf("Usage: ./peerserver <my_port>\n");
        exit(0);
    }
    else {
        my_port = atoi(argv[1]);
    }

	int	sockfd; /* Socket descriptors */
    int newsockfd[NO_OF_CLIENTS];
	int	clilen[NO_OF_CLIENTS+1], clilen_buf;
	struct sockaddr_in	cli_addr[NO_OF_CLIENTS+1], serv_addr, cli_addr_buf;
    clilen_buf = sizeof(cli_addr_buf);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		    = htons(ports[my_index]);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}

    if(listen(sockfd, 5) < 0) {
        printf("Listen error\n");
        exit(0);
    }

    int    i = 0, num = 0;
    char   buf[MAX_BUFF];


    for (i = 0; i < NO_OF_CLIENTS; i++) {
        newsockfd[i] = socket(AF_INET, SOCK_STREAM, 0);
    }
    int client_isthere[NO_OF_CLIENTS];
    for (i = 0; i < NO_OF_CLIENTS; i++) {
        client_isthere[i] = -1;
    }

    for (i = 0; i < NO_OF_CLIENTS+1; i++) {
        cli_addr[i].sin_family = AF_INET;
        cli_addr[i].sin_port = htons(ports[i]);
        cli_addr[i].sin_addr.s_addr = inet_addr(ip);
        clilen[i] = sizeof(cli_addr[i]);
    }
    for (i = 0; i < NO_OF_CLIENTS+1; i++) {
        clilen[i] = sizeof(cli_addr[i]);
    }
    int cli_indices[NO_OF_CLIENTS+1];
    for (i = 0; i < NO_OF_CLIENTS+1; i++) {
        cli_indices[i] = -1;
    }

    fd_set fds_og, fds;
    FD_ZERO (&fds);
    // printf("Server started\n");
    // fflush(stdout);
    while(1) {
        FD_ZERO (&fds);
        FD_SET(0, &fds);
        FD_SET(sockfd, &fds);
        for (i = 0; i < NO_OF_CLIENTS; i++) {
            if (client_isthere[i] != -1) FD_SET(newsockfd[i], &fds);
        }
        size_t nfds = 0;
        for (i = 0; i < NO_OF_CLIENTS; i++) {
            if (newsockfd[i] > nfds && client_isthere[i] != -1) {
                nfds = newsockfd[i];
            }
        }
        if (nfds < sockfd) nfds = sockfd;
        nfds++;
        select(nfds, &fds, 0, 0, 0);
        for (i = 0; i < NO_OF_CLIENTS; i++) {
            if (client_isthere[i] != -1 && FD_ISSET(newsockfd[i], &fds)) {
                memset(buf, 0, MAX_BUFF); char temp_buff[MAX_BUFF]; memset(temp_buff, 0, MAX_BUFF);
                while (1) {
                    if ((num = recv(newsockfd[i], temp_buff, MAX_BUFF, 0)) < 0) {
                        printf("Error in receiving message\n");
                        exit(0);
                    }
                    strcat(buf, temp_buff);
                    if (temp_buff[strlen(temp_buff)-1] == '\n') break;
                    memset(temp_buff, 0, MAX_BUFF);
                }
                // if (num == 0) {
                //     FD_CLR(newsockfd[i], &fds);
                //     newsockfd[i] = -1;
                //     close(newsockfd[i]);
                //     continue;            
                // }
                char name[MAX_FRIEND_NAME]; memset(name, 0, MAX_FRIEND_NAME);
                int k = 0;
                int count = 0;
                int j = 0;
                while(count != 2){
                    if (buf[j] == ' ' || buf[j] == '\t') {
                        count++;
                    }
                    j++;
                }
                while(buf[j] != ' ' && buf[j] != '\t' && buf[j] != ':' && buf[j] != '\n') {
                    name[k] = buf[j];
                    k++; j++;
                }name[k] = '\0';
                for (int j = 0; j < TABLE_SIZE; j++) {
                    if (strcmp(name, names[j]) == 0) {
                        // printf("Message from %s", names[j]);
                        // fflush(stdout);
                        cli_indices[j] = i;
                        break;
                    }
                }
                printf("%s", buf);
                fflush(stdout);
                memset(buf, 0, MAX_BUFF);
                break;
            }
        }
        if (FD_ISSET(sockfd, &fds)) {
            for (i = 0; i < NO_OF_CLIENTS; i++) {
                if (client_isthere[i] == -1) {
                    newsockfd[i] = accept(sockfd, (struct sockaddr *) &cli_addr_buf, &clilen_buf);
                    if (newsockfd[i] < 0) {
                        printf("Accept error\n");
                        exit(0);
                    }
                    client_isthere[i] = 1;
                    // printf("Connection established with %s from %s\n", names[i], names[my_index]);
                    // fflush(stdout);
                    break;
                }
            }
        }
        else if (FD_ISSET(0, &fds)) {
            memset(buf, 0, MAX_BUFF);
            char message[MAX_BUFF]; memset (message, 0, MAX_BUFF);
            fgets(message, MAX_BUFF, stdin);
            char final_message[MAX_BUFF]; memset (final_message, 0, MAX_BUFF);
            i = 0; int j =0;
            // printf("Enter the name of the friend: %s",message);
            // fflush(stdout);
            int count = 0;
            while (message[i] != '\n') {
                if (count != 0) {final_message[j] = message[i]; j++;}
                if (message[i] == '/') {
                    if (count == 0) count++;
                }
                i++;
            } final_message[j] = '\n';
            char *friend_name = strtok(message, "/");
            memset(buf, 0, MAX_BUFF);
            sprintf(buf, "Message from %s: %s",names[my_index], final_message);

            // printf("%s\n", friend_name);
            // fflush(stdout);
            for (int j = 0; j < TABLE_SIZE; j++) {
                if (strcmp(friend_name, names[j]) == 0){
                    // printf("Sending message to %s\n", names[j]);
                    // fflush(stdout);
                    if (cli_indices[j]!=-1 && client_isthere[cli_indices[j]] != -1) {
                        if (send(newsockfd[cli_indices[j]], buf, strlen(buf), 0) < 0) {
                            printf("Error in sending message\n");
                            exit(0);
                        }
                    }
                    else {
                        for (int k = 0; k < NO_OF_CLIENTS; k++) {
                            // printf("Trying to connect to %s\n", names[j]);
                            // fflush(stdout);
                            if (client_isthere[k] == -1){
                                if (connect(newsockfd[k], (struct sockaddr *) &cli_addr[j], clilen[j]) < 0) {
                                    printf("Connection error\n");
                                    exit(0);
                                }
                                client_isthere[k] = 1;
                                cli_indices[j] = k;
                                if (send(newsockfd[k], buf, strlen(buf), 0) < 0) {
                                    printf("Error in sending message\n");
                                    exit(0);
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
	return 0;
}

