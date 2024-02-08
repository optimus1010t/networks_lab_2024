// Online C compiler to run C program online
#include <stdio.h>
#include <string.h>

#define MAX_LINE_LENGTH 1000


void printBufInFormat(char *buf){
    char temp_sender[MAX_LINE_LENGTH]; memset(temp_sender, 0, sizeof(temp_sender));
    char temp_received[MAX_LINE_LENGTH]; memset(temp_received, 0, sizeof(temp_received));
    char temp_subject[MAX_LINE_LENGTH];  memset(temp_subject, 0, sizeof(temp_subject));
    char temp_receiver[MAX_LINE_LENGTH]; memset(temp_receiver, 0, sizeof(temp_receiver));
    char *line;
    int serial_number = 0;
    int check_body = 0;
    line = strtok(buf, "\r\n");
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
            serial_number++;
            printf("%d\t%s\t%s\t%s\n", serial_number, temp_sender, temp_received, temp_subject);
            // memset(temp_sender, 0, sizeof(temp_sender));
            // memset(temp_received, 0, sizeof(temp_received));
            // memset(temp_subject, 0, sizeof(temp_subject));
            // memset(temp_receiver, 0, sizeof(temp_receiver));
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
    while (temp_received[i] != ' ' && temp_received[i] != '\t' && temp_received[i] != '\0') received[j++] = temp_received[i++];
    i = 0; j = 0;
    while (temp_subject[i] != ':') i++; i++;
    while (temp_subject[i] == ' ' || temp_subject[i] == '\t') i++;
    while (temp_subject[i] != ' ' && temp_subject[i] != '\t' && temp_subject[i] != '\0') subject[j++] = temp_subject[i++];
    i = 0; j = 0;
    while (temp_receiver[i] != ':') i++; i++;
    while (temp_receiver[i] == ' ' || temp_receiver[i] == '\t') i++;
    while (temp_receiver[i] != ' ' && temp_receiver[i] != '\t' && temp_receiver[i] != '\0') receiver[j++] = temp_receiver[i++];
    printf("%d\t%s\t%s\t%s\n", serial_number, sender, received, subject);
}
int main(){
    char buf[MAX_LINE_LENGTH] = "dsdsdvsdvdsv\r\nFrom: tanishq@we\r\nTo:thoya@##\r\nSubject: sefsdf\r\nReceived:02/02/24:11:50\r\nsdfsdf\r\nsdfsdfsdf\r\nsdf\r\nsd.\r\n...\r\n.\r\n";
    printBufInFormat(buf);
    return 0;
}