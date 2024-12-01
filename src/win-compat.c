#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// Define the usleep function for Windows
void win_compat_usleep(unsigned int microseconds) {
    // Convert microseconds to milliseconds
    unsigned int milliseconds = microseconds >> 10; // /1024;
    Sleep(milliseconds);
}

ssize_t win_compat_getline(char **lineptr, size_t *n, FILE *stream) {
    size_t pos;
    int c;

    if (lineptr == NULL || n == NULL || stream == NULL) {
        return -1;
    }

    if (*lineptr == NULL) {
        *n = 128; // Initial buffer size
        *lineptr = malloc(*n);
        if (*lineptr == NULL) {
            return -1;
        }
    }

    pos = 0;
    while ((c = fgetc(stream)) != EOF) {
        if (pos + 1 >= *n) {
            size_t new_size = *n * 2;
            char *new_ptr = realloc(*lineptr, new_size);
            if (new_ptr == NULL) {
                return -1;
            }
            *lineptr = new_ptr;
            *n = new_size;
        }
        (*lineptr)[pos++] = c;
        if (c == '\n') {
            break;
        }
    }

    if (pos == 0 && c == EOF) {
        return -1;
    }

    (*lineptr)[pos] = '\0';
    return pos;
}
