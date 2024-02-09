#include <stdio.h>

int main() {
    FILE* fp;
    fp = fopen("e.txt", "r+");

    if (fp == NULL) {
        printf("Error opening the file.\n");
        return 1;
    }

    // Move file pointer to the beginning of the file
    fseek(fp, 0, SEEK_SET);

    FILE* tmp = fopen("temp.txt", "w"); // Temporary file to store non-whitespace characters

    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (ch != ' ' && ch != '\n') {
            fputc(ch, tmp); // Write non-whitespace characters to temporary file
        }
    }

    fclose(fp);
    fclose(tmp);

    // Reopen the original file in write mode to truncate it
    fp = fopen("e.txt", "w");
    tmp = fopen("temp.txt", "r");

    // Copy content from temporary file to original file
    while ((ch = fgetc(tmp)) != EOF) {
        fputc(ch, fp);
    }

    fclose(fp);
    fclose(tmp);

    // Remove the temporary file
    remove("temp.txt");

    return 0;
}
