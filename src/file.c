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

#include <ftw.h> // POSIX

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

    fread(contents, 1, length, file);
    contents[length] = '\0'; // Null-terminate the string
    fclose(file);

    return contents;
}

bool write_to_file(char *path, char *filename, char *buf, unsigned int len) {
  FILE *fd;
  size_t written;
  char fullpath[256];
  snprintf(fullpath,255,"%s/%s",path,filename);
  fd = fopen(fullpath,"w");
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
  if(remove(pathname) < 0) {
        _err("Error: remove path %s",pathname);
        _err("%s",strerror(errno));
        return -1;
  }
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
