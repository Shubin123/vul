#include <stdio.h>
#include <stdbool.h>
#include <string.h>

bool isWhitespace(int ch) {
    return ch == ' ' || ch == '\t';
}

bool isIncludeDirective(const char *line) {
    return strncmp(line, "#include", 8) == 0;
}

void writeCompressed(FILE *dest, const char *line, bool *isFirstCodeLineWritten) {
    char buffer[1024];
    int bufferIndex = 0;
    bool inComment = false, lastWasSpace = true;

    for (int i = 0; line[i] != '\0'; ++i) {
        if (line[i] == '/' && line[i + 1] == '/') {
            inComment = true;
            break; // Ignore rest of the line (inline comment)
        }

        if (!isWhitespace(line[i])) {
            buffer[bufferIndex++] = line[i];
            lastWasSpace = false;
        } else if (!lastWasSpace) {
            buffer[bufferIndex++] = ' '; // Add a single space
            lastWasSpace = true;
        }
    }

    if (!inComment && bufferIndex > 0 && !lastWasSpace) {
        // Trim trailing space
        bufferIndex--;
    }
    
    if (bufferIndex > 0) {
        buffer[bufferIndex] = '\0';
        if (!*isFirstCodeLineWritten) {
            fputs(buffer, dest);
            *isFirstCodeLineWritten = true;
        } else {
            fputc(' ', dest); // Add a space before appending new content
            fputs(buffer, dest);
        }
    }
}

int main() {
    FILE *fp, *tmp;
    char line[1024];
    bool isFirstCodeLineWritten = false;

    fp = fopen("e.txt", "r+");
    if (fp == NULL) {
        printf("Error opening the file.\n");
        return 1;
    }

    tmp = fopen("temp.txt", "w");
    if (tmp == NULL) {
        printf("Error opening the temporary file.\n");
        fclose(fp);
        return 1;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (isIncludeDirective(line)) {
            fputs(line, tmp); // Preserve #include lines
        } else {
            writeCompressed(tmp, line, &isFirstCodeLineWritten);
        }
    }

    fclose(fp);
    fclose(tmp);

    // Overwrite the original file
    fp = fopen("e.txt", "w");
    tmp = fopen("temp.txt", "r");

    while (fgets(line, sizeof(line), tmp)) {
        fputs(line, fp);
    }

    fclose(fp);
    fclose(tmp);

    remove("temp.txt");

    return 0;
}
