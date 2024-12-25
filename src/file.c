/* CJIT https://dyne.org/cjit
 *
 * Copyright (C) 2024 Dyne.org foundation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <cjit.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>

#include <ftw.h> // _GNU_SOURCE

int detect_bom(const char *filename) {
	uint8_t bom[3];
	int res;
	int fd = open(filename, O_RDONLY | O_BINARY);
	res = read(fd,bom,3);
	if (res!=3) {
		_err("read error: %s",strerror(errno));
		return -1;
	}
	close(fd);
	// _err("%s bom: %x %x %x",filename,bom[0],bom[1],bom[2]);
	if (bom[0] == 0xFF && bom[1] == 0xFE) {
		return 1; // UTF-16 LE
	} else if (bom[0] == 0xFE && bom[1] == 0xFF) {
		return 2; // UTF-16 BE
	} else if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
		return 3; // UTF-8
	} else {
		return 0; // No BOM
	}
}

bool append_path(char **stored_path, const char *new_path) {
  // TODO: sanitize input checking only path chars are there
  // support both / and \ for windows
  if (*stored_path == NULL) {
    // If stored_path is NULL, allocate memory and copy new_path
    *stored_path = malloc(strlen(new_path) + 1);
    if (*stored_path == NULL) {
      _err("Memory allocation failed");
      return(false);
    }
    strcpy(*stored_path, new_path);
  } else {
    // If stored_path is not NULL, append new_path separated by ':'
    size_t new_length = strlen(*stored_path) + strlen(new_path) + 2;
    char *temp = realloc(*stored_path, new_length);
    if (temp == NULL) {
      _err("Memory allocation failed");
      return(false);
    }
    *stored_path = temp;
    strcat(*stored_path, ":");
    strcat(*stored_path, new_path);
  }
  return(true);
}

bool prepend_path(char **stored_path, const char *new_path) {
  if (*stored_path == NULL) {
    // If stored_path is NULL, allocate memory and copy new_path
    *stored_path = malloc(strlen(new_path) + 1);
    if (*stored_path == NULL) {
      _err("Memory allocation failed");
      return(false);
    }
    strcpy(*stored_path, new_path);
  } else {
    // If stored_path is not NULL, prepend new_path separated by ':'
    size_t new_length = strlen(*stored_path) + strlen(new_path) + 2;
    char *temp = malloc(new_length);
    if (temp == NULL) {
      _err("Memory allocation failed");
      return(false);
    }
    strcpy(temp, new_path);
    strcat(temp, ":");
    strcat(temp, *stored_path);
    free(*stored_path);
    *stored_path = temp;
  }
  return(true);
}

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

char* file_load(const char *filename, unsigned int *len) {
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
    *len = length;
    fclose(file);

    return contents;
}

char *load_stdin() {
#if defined(WINDOWS)
  return NULL;
#else
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
#endif
}

bool write_to_file(const char *path, const char *filename, const char *buf, unsigned int len) {
  FILE *fd;
  size_t written;
  char fullpath[256];
#if defined(WINDOWS)
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
#if !defined(WINDOWS)
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

#if !defined(WINDOWS)

static char *full_content = NULL;


static int file_load_ftw(const char *pathname,
                         const struct stat *sbuf,
                         int type, struct FTW *ftwb) {
    FILE *fd;
    char *content = NULL;
    unsigned int len;
    if (type == FTW_F) {
        size_t pathlen = strlen(pathname);
        if (pathname[pathlen-1] == 'c' &&
            pathname[pathlen-2] == '.') {
            content = file_load(pathname, &len);
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
