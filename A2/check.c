#include <stdio.h>
#include <stdlib.h>

int areFilesEqual(FILE *fp1, FILE *fp2) {
    char ch1, ch2;
    do {
        ch1 = fgetc(fp1);
        ch2 = fgetc(fp2);

        if (ch1 != ch2)
            return 0; // Files are not equal
    } while (ch1 != EOF && ch2 != EOF);

    // Both files must reach EOF together for them to be equal
    if (ch1 == EOF && ch2 == EOF)
        return 1;
    else
        return 0;
}

int main() {
    FILE *fp1, *fp2;
    char file1[] = "smth.txt";
    char file2[] = "text.txt.enc";

    // Open the two files
    fp1 = fopen(file1, "r");
    fp2 = fopen(file2, "r");

    // Check if the files could be opened successfully
    if (fp1 == NULL || fp2 == NULL) {
        printf("Error in opening file(s)\n");
        exit(1);
    }

    if (areFilesEqual(fp1, fp2))
        printf("Files are equal.\n");
    else
        printf("Files are not equal.\n");

    // Close the files
    fclose(fp1);
    fclose(fp2);

    return 0;
}
