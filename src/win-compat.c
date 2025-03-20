#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#include <windows.h>
#include <tchar.h>

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

bool get_winsdkpath(char *dst, size_t destlen) {
    HKEY hKey;
    LPDWORD len = (LPDWORD)destlen; // unused
    LONG lRes = RegOpenKeyEx
	    (HKEY_LOCAL_MACHINE,
	     "SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots",
	     0, KEY_READ, &hKey);
    if (lRes != ERROR_SUCCESS) {
        fprintf(stderr,"Failed to open registry key. Error: %ld\n", lRes);
        return false;
    }
    lRes = RegQueryValueEx(hKey, "KitsRoot10",
			   NULL, NULL, dst, len);
    RegCloseKey(hKey);
    if (lRes != ERROR_SUCCESS) {
        fprintf(stderr,"Failed to query registry value. Error: %ld\n",lRes);
        RegCloseKey(hKey);
        return false;
    }
    // Append the Include path
    strcat(dst, "Include\\");
    // Find the correct SDK version directory
    WIN32_FIND_DATA findFileData;
    char *tmp = malloc(strlen(dst)+4);
    strcpy(tmp,dst); strcat(tmp,"*");
    HANDLE hFind = FindFirstFile(tmp, &findFileData);
    free(tmp);
    if (hFind == INVALID_HANDLE_VALUE) {
	    printf("Failed to find any SDK version directories.\n");
	    return false;
    }
    do {
	    if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		    if (_tcscmp(findFileData.cFileName, ".") != 0
			&& _tcscmp(findFileData.cFileName, "..") != 0) {
			    // Append the first valid directory found
			    strcat(dst, findFileData.cFileName);
			    break;
		    }
	    }
    } while (FindNextFile(hFind, &findFileData) != 0);
    FindClose(hFind);
    // fprintf(stderr,"Windows SDK Path: %s\n", dst);
    return(true);
}
