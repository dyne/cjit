/* CJIT https://dyne.org/cjit
 *
 * Copyright (C) 2024 Dyne.org foundation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <ftw.h> // _GNU_SOURCE

#ifdef LIBC_MINGW32
#include <windows.h>
#include <shlwapi.h>
#include <rpc.h>
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "shlwapi.lib")
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#endif
extern void _err(const char *fmt, ...);

// Function to get the length of a file in bytes
long file_size(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fclose(file);
    return length;
}

char* file_load(const char *filename) {
    long length = file_size(filename);
    if (length == -1) {
        return NULL;
    }

    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    char *contents = (char*)malloc((length + 1) * sizeof(char));
    if (contents == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    // _err("Loading source file %s",filename);

    fread(contents, 1, length, file);
    contents[length] = '\0'; // Null-terminate the string
    fclose(file);

    return contents;
}

char *load_stdin() {
  char *code = NULL;
  char *line = NULL;
  size_t len = 0;
  ssize_t rd;
  fflush(stdout);
  fflush(stderr);
  while(1) {
    rd = getline(&line, &len, stdin);
    if(rd == -1) { // ctrl+d
      free(line);
      break;
    }
    code = realloc(code, (code?strlen(code):0) + len + 1);
    if (!code) {
      _err("Memory allocation error");
      free(line);
      return NULL;
    }
    strcat(code, line);
    free(line);
    line = NULL;
  }
  return(code);
}

bool write_to_file(char *path, char *filename, char *buf, unsigned int len) {
  FILE *fd;
  size_t written;
  char fullpath[256];
#if defined(LIBC_MINGW32)
  snprintf(fullpath,255,"%s\\%s",path,filename);
#else
  snprintf(fullpath,255,"%s/%s",path,filename);
#endif
  fd = fopen(fullpath,"wb");
  if(!fd) {
    _err("Error: fopen file %s",fullpath);
    _err("%s",strerror(errno));
    return false;
  }
  written = fwrite(buf,1,len,fd);
  fclose(fd);
  if(written != len) {
    _err("Error: fwrite file %s",fullpath);
    _err("%s",strerror(errno));
    return false;
  }
  return true;
}

static int rm_ftw(const char *pathname,
                  const struct stat *sbuf,
                  int type, struct FTW *ftwb) {
#ifndef LIBC_MINGW32
  if(remove(pathname) < 0) {
    _err("Error: remove path %s",pathname);
    _err("%s",strerror(errno));
    return -1;
  }
#else
  if (type == FTW_F) {
    if(DeleteFile(pathname) == 0) {
      _err("Error: DeleteFile path %s",pathname);
      _err("%s",strerror(errno));
      return -1;
    }
  } else if (type == FTW_D) {
    if(RemoveDirectory(pathname) == 0) {
      _err("Error: RemoveDirectory path %s",pathname);
      _err("%s",strerror(errno));
      return -1;
    }
  }
#endif
  return 0;
}
bool rm_recursive(char *path) {
  if (nftw(path, rm_ftw, 10, FTW_DEPTH|FTW_MOUNT|FTW_PHYS) < 0) {
    _err("Error: nftw path %s",path);
    _err("%s",strerror(errno));
        return false;
  }
  return true;
}

#ifndef LIBC_MINGW32

static char *full_content = NULL;


static int file_load_ftw(const char *pathname,
                         const struct stat *sbuf,
                         int type, struct FTW *ftwb) {
    FILE *fd;
    char *content = NULL;
    if (type == FTW_F) {
        size_t pathlen = strlen(pathname);
        if (pathname[pathlen-1] == 'c' &&
            pathname[pathlen-2] == '.') {
            content = file_load(pathname);
            if (content == NULL) {
                _err("Error: file_load %s",pathname);
                return -1;
            }
            if (full_content == NULL) {
                full_content = content;
            } else {
                full_content = realloc(full_content, strlen(full_content) + strlen(content) + 1);
                if (full_content == NULL) {
                    _err("Error: realloc full_content");
                    return -1;
                }
                strcat(full_content, content);
            }
        }
    }
    return 0;
}

/* dir_load: nftw version */
char *dir_load(const char *path)
{
    struct stat sb;
    FILE *fd;
    char *content = NULL;

    if (stat(path, &sb) != 0) {
        _err("Error: %s",path);
        _err("%s",strerror(errno));
        return NULL;
    }
    if (!S_ISDIR(sb.st_mode)) {
        _err("Error: %s is not a directory",path);
        return NULL;
    }
    if (nftw(path, file_load_ftw, 10, FTW_DEPTH|FTW_MOUNT|FTW_PHYS) < 0) {
        _err("Error: nftw path %s",path);
        _err("%s",strerror(errno));
        return NULL;
    }
    return full_content;
}


#else
char *dir_load(const char *path)
{
    /* FIXME */
    _err("Error: dir_load not implemented on Windows");
    return NULL;
}


#endif


#ifdef LIBC_MINGW32
///////////////
// WINDOWS SHIT
char *win32_mkdtemp() {
    static char tempDir[MAX_PATH];
    static char sysTempDir[MAX_PATH];
    static char secTempDir[MAX_PATH];
    static char winTempDir[MAX_PATH];
    char tempPath[MAX_PATH];
    char filename [] = "CJIT-XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    // Get the temporary path
    if (GetTempPath(MAX_PATH, tempPath) == 0) {
        _err("Failed to get temporary path");
        return NULL;
    }
    // else
    //   _err("Temporary path is %s",tempPath);
    // randomize the temp dir name
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t charset_size = sizeof(charset) - 1;
    char *str = &filename[5];
    // Seed the random number generator
    srand((unsigned int)time(NULL));
    for (size_t i = 0; i < 16; i++) {
        int key = rand() % charset_size;
        *str = charset[key];
        str++;
    }
    *str = '\0'; // Null-terminate the string
    PathCombine(tempDir, tempPath, filename);
    // Create the temporary directory
    if (CreateDirectory(tempDir, NULL) == 0) {
        _err("Failed to create temporary dir: %s",tempDir);
        return NULL;
    }
    PathCombine(sysTempDir, tempDir, "sys");
    if (CreateDirectory(sysTempDir, NULL) == 0) {
        _err("Failed to create sys dir in temporary dir: %s",sysTempDir);
        return NULL;
    }
    PathCombine(secTempDir, tempDir, "sec_api");
    if (CreateDirectory(secTempDir, NULL) == 0) {
        _err("Failed to create sec_api dir in temporary dir: %s",secTempDir);
        return NULL;
    }
    PathCombine(winTempDir, tempDir, "winapi");
    if (CreateDirectory(winTempDir, NULL) == 0) {
        _err("Failed to create winapi dir in temporary dir: %s",winTempDir);
        return NULL;
    }
    return(tempDir);
}
#endif
