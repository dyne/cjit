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
#include <cwalk.h>

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

// Function to get the length of a file in bytes
static long file_size(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
		fail(filename);
        return -1;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fclose(file);
    return length;
}

char* file_load(const char *filename, unsigned int *len) {
    size_t length = file_size(filename);
    if (length < 1) {
        return NULL;
    }

    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
		fail(filename);
        return NULL;
    }

    char *contents = (char*)malloc((length + 1) * sizeof(char));
    if (contents == NULL) {
		fail(filename);
        fclose(file);
        return NULL;
    }

    // _err("Loading source file %s",filename);

    if(fread(contents, 1, length, file)<length) {
		fail(file);
		return NULL;
	}

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
			fail("malloc error");
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

char *new_abspath(const char *path) {
	char tpath[MAX_PATH];
	char *res = NULL;
	size_t len;
	if(path[0]=='.' && path[1]==0x0) {
		// argument is just .
		if( getcwd(tpath,MAX_PATH) ) {
			res = malloc(strlen(tpath)+1);
			strcpy(res,tpath);
			return(res);
		}
	}
	if(path[0]=='.' && path[1]=='/') {
		if( !getcwd(tpath,MAX_PATH) ) {
			fail(path);
			return(NULL);
		}
		res = malloc(strlen(tpath)+1);
		strcpy(res,tpath);
		len = cwk_path_get_absolute(res,path,tpath,MAX_PATH);
		res = realloc(res, len+1);
		strcpy(res,tpath);
		return(res);
	}
	if(path[0]!='/') {
		if( !getcwd(tpath,MAX_PATH) ) {
			fail(path);
			return(NULL);
		}
		res = malloc(strlen(tpath)+strlen(path)+16);
		strcpy(res,tpath);
		len = cwk_path_get_absolute(res,path,tpath,MAX_PATH);
		res = realloc(res,len+1);
		strcpy(res,tpath);
		return(res);
	}
	len = cwk_path_normalize(path,tpath,MAX_PATH);
	res = malloc(len+1);
	strcpy(res,tpath);
	return(res);
}

bool write_to_file(const char *path, const char *filename, const char *buf, unsigned int len) {
	FILE *fd;
	size_t written;
	char fullpath[MAX_PATH];
	cwk_path_join(path,filename,fullpath,MAX_PATH);
	fd = fopen(fullpath,"wb");
	if(!fd) {
		fail(fullpath);
		return false;
	}
	written = fwrite(buf,1,len,fd);
	fclose(fd);
	if(written != len) {
		fail(fullpath);
		return false;
	}
	return true;
}

#if 0 // unused for now, dangerous to have if unnecessary
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
#endif


#if 0
#if !defined(WINDOWS)

static char *full_content = NULL;


static int file_load_ftw(const char *pathname,
                         const struct stat *sbuf,
                         int type, struct FTW *ftwb) {
    unsigned int len;
    char *content = NULL;
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
#endif // 0
