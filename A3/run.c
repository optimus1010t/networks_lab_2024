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
    int i; int j;
    char recv_time[100]; memset(recv_time, 0, sizeof(recv_time));
    char mail[6000] = "From: tho@ed\r\nTo: tan@gd\r\nSubject: Hello\r\nHi\r\n.\r\n";
    int c = 0; 
    char new_mail[strlen(mail) + 100]; memset(new_mail, 0, sizeof(new_mail));
    j=0;
    for (i = 0; i < strlen(mail); i++)
    {
        if (c < 3 && mail[i] == '\r' && mail[i + 1] == '\n')
        {
            if ( c < 2) c++;
            else if (c == 2)
            {
                new_mail[j] = '\r'; new_mail[j+1] = '\n';
                i++;
                time_t t = time(NULL);
                struct tm tm = *localtime(&t);
                sprintf(recv_time, "Received: %d-%02d-%02d %02d:%02d:%02d\r\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                strcat(new_mail, recv_time);
                j=strlen(new_mail);
                c++;
                continue;
            }
        }        
        new_mail[j] = mail[i];   
        j++;     
    }
    // calculate number of \r and \n in new_mail
    int count_r = 0, count_n = 0;
    for (i=0; i < strlen(new_mail); i++)
    {
        if (new_mail[i]=='\r') count_r++;
        if (new_mail[i]=='\n') count_n++;
    }
    printf("%d %d\n", count_r,count_n);
    printf("%s..", new_mail);

	return 0;
}
