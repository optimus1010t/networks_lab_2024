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

// In accordance wirh RFC 5321 and RFC 1939
int main(int argc, char*argv[])
{
    char filename[] = "a.out";
    FILE* file;

    // Attempt to open the file for reading
    file = fopen(filename, "r"); // if it is "w" then would just create the file if it doesnt exist

    if (file != NULL) {
        printf("File exists!\n");
        // Close the file if it was successfully opened
        fclose(file);
    } else {
        printf("File does not exist!\n");
    }
}

