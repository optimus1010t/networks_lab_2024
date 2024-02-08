#include <stdio.h>
#include <string.h>

#define MAX_LINE_LENGTH 10000

void display_mail_details(int serial_number, const char* sender, const char* received, const char* subject) {
    printf("%d\t", serial_number);
    printf("<%s>\t", sender);
    printf("<%s>\t", received);
    printf("<%s>\t", subject);
    printf("\n");
}

int main() {
    FILE *file;
    char line[MAX_LINE_LENGTH];

    // Open the file
    file = fopen("mailbox", "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Initialize variables
    int serial_number = 0;
    char sender[MAX_LINE_LENGTH];
    char received[MAX_LINE_LENGTH];
    char subject[MAX_LINE_LENGTH];

    int loop_in=0;

    // Read the file line by line
    while (fgets(line,sizeof(line),file)!=NULL) {
        // Remove newline character at the end
        line[strcspn(line, "\n")] = 0;

        // Check if the line is a separator (".") 
        if (strcmp(line, ".") == 0) {
            // Display mail details
            if(strlen(sender)>0 && strlen(received)>0 && strlen(subject)>0){
                serial_number++;
                display_mail_details(serial_number, sender, received, subject);

            }
            // Reset variables for the next mail
            memset(sender, 0, sizeof(sender));
            memset(received, 0, sizeof(received));
            memset(subject, 0, sizeof(subject));
        } else {
            // Check if the line is a "From:" line
            if (strncmp(line, "From:", 5) == 0) {
                strcpy(sender, line + 6);
            }
            // Check if the line is a "Received:" line
            else if (strncmp(line, "Received:", 9) == 0) {
                strcpy(received, line + 10);
            }
            // Check if the line is a "Subject:" line
            else if (strncmp(line, "Subject:", 8) == 0) {
                strcpy(subject, line + 9);
            }
        }
        memset(line, 0, sizeof(line));
        loop_in++;
    }
    // Close the file
    fclose(file);
    return 0;
}
