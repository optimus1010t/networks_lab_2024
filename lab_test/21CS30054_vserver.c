#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/select.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>


int main()
{
    struct sockaddr_in servaddr;
    int sockfd;
    socklen_t serv_len;
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(60000);
    inet_aton("127.0.0.1", &servaddr.sin_addr); // changed and corrected
    serv_len = sizeof(servaddr);

    if (bind(sockfd,(struct sockaddr*)&servaddr,serv_len) < 0) {
        perror("bind : ");
        exit(0);
    }
    char entity_names[10][201];
    memset(entity_names,0,2010);

    int clisock[5];
    for (int i = 0; i < 5; i++) clisock[i]=-1;

    int vote_info_table[10];
    for (int i = 0; i < 10; i++) vote_info_table[i] = -1;
    // when it is assigned then make it 0

    printf("Enter choice : \n");
    printf("1) Print list and votes\n");
    printf("2) Add new entities\n");
    printf("3) Delete an entity\n");
    while (1) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(0,&fds);
        FD_SET(sockfd,&fds);
        long int nfds = sockfd;
        for (int i = 0; i < 5; i++){
            if (clisock[i] != -1)
                FD_SET(clisock[i],&fds);
                if (clisock[i] > nfds) nfds = clisock[i];
        }
        nfds += 1;
        select(nfds,&fds,0,0,0);

        if (FD_ISSET(0,&fds)){
            int choice;
            scanf("%d",&choice);
            char ch;
            fflush(stdout);
            while (1) {
                ch = getchar();
                if (ch == '\n' || ch == EOF) break;
            }
            if (choice == 1) {
                printf("Entity names\t\tVotes\n");  // brought this inside this loop (changed)
                for (int i = 0; i < 10; i++) {
                    if (vote_info_table[i]!= -1){
                        printf("%s\t\t%d\n",entity_names[i],vote_info_table[i]);
                    }
                }
            }
            else if (choice == 2) {
                char entity[201];
                memset(entity,0,201);
                int flag = 0; int index = 0;
                for (int i = 0; i<10; i++) {
                    if (vote_info_table[i] == -1){
                        flag = 1;
                        index = i;
                        break;
                    }
                }
                if (flag == 0) {
                    printf("No more entities can be added\n");
                    continue;
                }
                printf("Enter the entity name to add : ");  // changed spelling to entity
                fgets(entity,201,stdin);
                if (entity[strlen(entity)-1] == '\n') entity[strlen(entity)-1] = '\0';

                strcpy(entity_names[index],entity);
                vote_info_table[index] = 0;
                continue;
            }
            else if (choice == 3) {
                char entity[201];
                memset(entity,0,201);
                printf("Enter the entity name to delete : ");
                fgets(entity,201,stdin);
                if (entity[strlen(entity)-1] == '\n') entity[strlen(entity)-1] = '\0';
                int flag = 0; int index = 0;
                for (int i = 0; i<10; i++) {
                    if (vote_info_table[i] != -1){
                        if (strcmp(entity_names[i],entity)==0) {
                            flag = 1;
                            index = i;
                            break;
                        }
                    }
                }
                if (flag == 0) {
                    printf("No such entity\n");
                    continue;
                }
                memset(entity_names[index],0,201);
                vote_info_table[index]=-1;
            }
        }
        
        if (FD_ISSET(sockfd,&fds)) {
            struct sockaddr_in cli_addr;
            socklen_t clilen;
            int cli_fd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
            int flag = 0;
            for (int i = 0; i < 5; i++){
                if (clisock[i] == -1){
                    flag = 1;
                    clisock[i]=cli_fd;
                }
            }
            if (flag == 0){
                close(cli_fd);
                continue;
            }
            int count = 0;
            for (int i = 0; i < 10; i++) {
                if (vote_info_table[i] != -1) count++;
            }
            long buff = htonl(count);
            send(cli_fd,&buff,sizeof(long),0);
            
            char buf[2020];
            memset(buf,0,2020);
            for (int i = 0; i < 10; i++){
                if (vote_info_table[i] != -1) {
                    for (int j = 0; j < 201; j++){
                        if (j=='\0') break;
                        buf[count]=entity_names[i][j];
                        count++;
                    }
                    buf[count] = '\0';
                    count++;
                }
            }
            buf[count] == '\n';
            if (count != 0) {
                send(cli_fd,buf,strlen(buf),0);
            }            
        }

        for (int k = 0; k < 5; k++) {
            if (clisock[k] != -1 && FD_ISSET(clisock[k],&fds)) {
                char buf[203]; memset(buf,0,203);
                int n = -237;
                while (1) {
                    char temp_buf[203]; memset(temp_buf,0,203);
                    n = recv (clisock[k],temp_buf,203,0);
                    if (n <= 0) break;
                    strcat(buf,temp_buf);
                    if (buf[strlen(buf)-1] == '\n') break;                                                            
                }

                if (n == 0) {
                    close(clisock[k]);
                    clisock[k] = -1;
                }
                buf[strlen(buf)-1] = '\0';

                int flag = 0;

                for (int i = 0; i < 10; i++) {
                    if (vote_info_table[i] != -1) {
                        if (strcmp(buf,entity_names[i]) == 0) {
                            flag = 1;
                            vote_info_table[i]++;
                            break;
                        }
                    }
                }

                if (flag == 1) {
                    char *buff = "Vote Registered Successfully\0";
                    send(clisock[k],buff,strlen(buff),0);
                }
                else {
                    char *buff = "Problem in Voting. Try again later\0";
                    send(clisock[k],buff,strlen(buff),0);
                }                
            }
        }
    }


}