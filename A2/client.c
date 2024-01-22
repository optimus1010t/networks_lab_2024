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

#define MAX_BUFF 100
#define FILE_MAX_BUFF 100

int main()
{
    while (1)
	{
        int flag_file = 0;
        char filename[FILE_MAX_BUFF];
        while (1) {
            memset(filename, 0, sizeof(filename));
            printf("Enter the filename: ");
            fgets(filename, sizeof(filename), stdin);
            if (filename[strlen(filename) - 1] == '\n') {
                filename[strlen(filename) - 1] = '\0';
            }
            struct stat s;
            if (stat(filename,&s) != 0){
                printf("File %s Not Found\n",filename);
                continue;
            }
            else {
                flag_file = 1;
                break;
            }
        }
        int file_d = open(filename, O_RDONLY);
        int k;
        // check if the file ois empty
        if (lseek(file_d, 0, SEEK_END) == 0) {
            printf("File is empty\n");
            continue;
        }
        file_d = open(filename, O_RDONLY);
        printf("Enter the key k: ");
        scanf("%d", &k);

        int	sockfd ;
        struct sockaddr_in	serv_addr;

        int i;
        char buf[MAX_BUFF];

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Unable to create socket\n");
            exit(0);
        }
        serv_addr.sin_family	= AF_INET;
        inet_aton("127.0.0.1", &serv_addr.sin_addr);
        serv_addr.sin_port	= htons(20001);

        if ((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) {
            perror("Unable to connect to server ");
            exit(0);
        }
        // sending k as key
        sprintf(buf, "%d", k);
        for (int i = strlen(buf); i < MAX_BUFF; i++) {
            buf[i] = '*';
        }
        send(sockfd, buf, strlen(buf), 0);
        // now reading the file and sending it
        memset(buf, 0, sizeof(buf));
        int last_size; char last_ch = '\0'; 
        while (read(file_d, buf, sizeof(buf)) > 0){
            last_size = strlen(buf);
            if (last_size < 100) {
                if (buf[last_size-1] != '\n')
                    buf[last_size] = '$';
            }
            last_ch = buf[last_size-1];
            send(sockfd, buf, strlen(buf), 0);
            memset(buf, 0, sizeof(buf));
        }
        if (last_ch != '\n' && last_ch != '$') { 
            buf[0] = '$';
            send(sockfd, buf, strlen(buf), 0);
        }
        /*
            The files given to check have a newline charcter at the last 
            so we are sending a newline character to indicate the end of the file
            Then I am checking against that newline character to break the loop
        */
        char filename_enc[FILE_MAX_BUFF];
        sprintf(filename_enc, "%s.enc", filename);
        int fd_enc = open(filename_enc, O_WRONLY | O_CREAT, 0666);
        int n;
        memset(buf, 0, sizeof(buf));
        while (1) {
            n = recv(sockfd, buf, MAX_BUFF, 0);
            ssize_t buf_len = 0;
            // if (buf[0] != '$') 
            // buf_len = write(fd_enc, buf, strlen(buf));
            if (buf[strlen(buf)-1] == '\n'){
                // buf[strlen(buf) - 1] = '\0';
					buf_len = write(fd_enc, buf, strlen(buf));
					break;
				}
                buf_len = write(fd_enc, buf, strlen(buf));
            if (buf_len == -1) {
                printf("Error in writing to file\n");
                continue;
            }
            // if (n < MAX_BUFF) {
            //     break;
            // }
            bzero(buf, MAX_BUFF);
        }
        printf("The file is encrypted!\n");
        printf("The name of the original file: %s\n", filename);
        printf("The name of the encrypted file: %s\n\n", filename_enc);
        close(sockfd);
        close(file_d);
        close(fd_enc);
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        close(sockfd);
    }    
}

