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
#ifndef O_BINARY
# define O_BINARY 0
#endif
#endif

#include <cjit.h>
extern void _err(const char *fmt, ...);

// from exec-headers.c
extern bool gen_exec_headers(char *tmpdir);

int detect_bom(const char *filename) {
	uint8_t bom[3];
	int res;
	int fd = open(filename, O_RDONLY | O_BINARY);
	res = read(fd,bom,3);
	if (res!=3) {
		_err("read error on %s: %s",filename, strerror(errno));
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
#ifdef LIBC_MINGW32
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

// cross-platform creation of any CJIT extension requested
// also fills CJIT->tmpdir on success
bool cjit_extensions_mkdtemp(CJITState *CJIT, const char *tmpdir) {
	// dmon.h
	if(CJIT->dmon) {
		if(!write_to_file(tmpdir,"dmon.h",
				  (char*)&lib_dmon_dmon_h,
				  lib_dmon_dmon_h_len)) return(false);
	}
	CJIT->tmpdir = malloc(strlen(tmpdir)+1);
	strcpy(CJIT->tmpdir, tmpdir);
	return(true);
}

#ifdef LIBC_MINGW32
///////////////
// WINDOWS SHIT
bool dir_exists(CJITState *CJIT, const char *path) {
	DWORD attributes = GetFileAttributes(path);
	if (attributes == INVALID_FILE_ATTRIBUTES) {
		// The path does not exist
		return false;
	} else if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
		// The path exists and is a directory
		CJIT->tmpdir = malloc(strlen(path)+1);
		strcpy(CJIT->tmpdir, path);
		return true;
	} else {
		_err("Temp dir is a file, cannot overwrite: %s",path);
		// The path exists but is not a directory
		return false;
	}
}

bool win32_mkdtemp(CJITState *CJIT) {
    static char tempDir[MAX_PATH];
    char sysTempDir[MAX_PATH];
    char secTempDir[MAX_PATH];
    char tccTempDir[MAX_PATH];
    char sysSecTempDir[MAX_PATH];
    char winTempDir[MAX_PATH];
    char tempPath[MAX_PATH];
    char filename [64];
    snprintf(filename,63,"CJIT-%s",VERSION);
    // Get the temporary path
    if (GetTempPath(MAX_PATH, tempPath) == 0) {
        _err("Failed to get temporary path");
        return false;
    }
    PathCombine(tempDir, tempPath, filename);
    // return already if found existing
    if(dir_exists(CJIT, tempDir)) return(true);
    // Create the temporary directory
    if (CreateDirectory(tempDir, NULL) == 0) {
        _err("Failed to create temporary dir: %s",tempDir);
        return false;
    }
    PathCombine(sysTempDir, tempDir, "sys");
    if (CreateDirectory(sysTempDir, NULL) == 0) {
        _err("Failed to create sys dir in temporary dir: %s",sysTempDir);
        return false;
    }
    PathCombine(secTempDir, tempDir, "sec_api");
    if (CreateDirectory(secTempDir, NULL) == 0) {
        _err("Failed to create sec_api dir in temporary dir: %s",secTempDir);
        return false;
    }
    PathCombine(sysSecTempDir, secTempDir, "sys");
    if (CreateDirectory(sysSecTempDir, NULL) == 0) {
        _err("Failed to create sec_api/sys dir in temporary dir: %s",sysSecTempDir);
        return false;
    }
    PathCombine(tccTempDir, tempDir, "tcc");
    if (CreateDirectory(tccTempDir, NULL) == 0) {
        _err("Failed to create tcc dir in temporary dir: %s",tccTempDir);
        return false;
    }
    PathCombine(winTempDir, tempDir, "winapi");
    if (CreateDirectory(winTempDir, NULL) == 0) {
        _err("Failed to create winapi dir in temporary dir: %s",winTempDir);
        return false;
    }
    if(!gen_exec_headers(tempDir)) return(false);
    if(!cjit_extensions_mkdtemp(CJIT, tempDir)) return(false);
    return(true);
}
#else // POSIX
bool dir_exists(CJITState *CJIT, const char *path) {
	struct stat info;
	if (stat(path, &info) != 0) {
		// stat() failed; the path does not exist
		return false;
	} else if (info.st_mode & S_IFDIR) {
		// The path exists and is a directory
		CJIT->tmpdir = malloc(strlen(path)+1);
		strcpy(CJIT->tmpdir, path);
		return true;
	} else {
		_err("Temp dir is a file, cannot overwrite: %s",path);
		// The path exists but is not a directory
		return false;
	}
}

bool posix_mkdtemp(CJITState *CJIT) {
	char tpath[260];
	snprintf(tpath,259,"/tmp/cjit-%s",VERSION);
	if(dir_exists(CJIT, tpath)) return(tpath);
	mkdir(tpath,0755);
	if(!gen_exec_headers(tpath) ) {
		_err("Cannot create temporary TCC headers");
		return(false);
	}
	if(!cjit_extensions_mkdtemp(CJIT, tpath)) {
		_err("Cannot create temporary CJIT headers");
		return(false);
	}
	return(true);
}
#endif
