/*
 *  Taken from ELF file handling in TCC
 *
 *  Copyright (c) 2001-2004 Fabrice Bellard
 *  Copyright (c) 2024-2025 Dyne.org
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// interpret a subset of GNU ldscripts to handle dummy .so files

#include <platforms.h>
#if defined(POSIX)
#define MAX_PATH 512
#include <cjit.h>
#include <cwalk.h>
#include <elflinker.h>

#define CH_EOF   (-1) //end of file
#define LD_TOK_NAME 256
#define LD_TOK_EOF  (-1)

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

// tinyCC internals used here
extern void dynarray_add(void *ptab, int *nb_ptr, void *data);
extern void dynarray_reset(void *pp, int *n);
extern char *tcc_strdup(const char *str);
extern char *pstrcpy(char *buf, size_t buf_size, const char *s);

// implemented further down
static int resolve_ldscript(LDState *s1, char *path);
static int find_library(xarray_t *results, const char *path);

bool read_ldsoconf(xarray_t *dest, char *path) {
	FILE *file = fopen(path, "r");
	if (!file) {
		fail(path);
		return false;
	}
	// TODO: optimize by allocating the buffer inside a xarray data
	// struct to avoid double memcpy
	char line[MAX_PATH];
	while (fgets(line, MAX_PATH, file) != NULL) {
		// Skip lines that aren't paths
		if (line[0] != '/') continue;
		size_t len = strlen(line);
		if(line[len-1]=='\n') line[len-1]=0x0;
		// add the line to the result array
		XArray_AddData(dest,line,len+1);
	}
	fclose(file);
    return true;
}

bool read_ldsoconf_dir(xarray_t *dest, const char *directory) {
    struct dirent **namelist;
    char path[MAX_PATH];
    int n;

    // Open the directory and read its sorted contents
    n = scandir(directory, &namelist, NULL, alphasort);
    if (n < 0) {
        fail(directory);
        return(false);
    }
    // go through sorted filenames
    for (int i = 0; i < n; i++) {
        if (namelist[i]->d_type == DT_REG) {
			// Regular files only
			//  || namelist[i]->d_type == DT_LNK
			cwk_path_join(directory,namelist[i]->d_name,path,MAX_PATH);
			if(! read_ldsoconf(dest,path) ) {
				fail(directory);
				continue;
			}
        }
        free(namelist[i]);
    }
    free(namelist);
    return true;
}

int resolve_libs(CJITState *cjit) {
	char tryfile[PATH_MAX];
	int i,ii;
	int libpaths_num, libnames_num;
	char *lpath, *lname;
	int found;
	// search in all paths if lib%s.so exists
	// TODO: support --static here
	libpaths_num = XArray_Used(cjit->libpaths);
	libnames_num = XArray_Used(cjit->libs);
	for(i=0;i<libnames_num;i++) {
		lname = XArray_GetData(cjit->libs,i);
		for(ii=0;ii<libpaths_num;ii++) {
			lpath = XArray_GetData(cjit->libpaths,ii);
			snprintf(tryfile,PATH_MAX-2,"%s/lib%s.so",lpath,lname);
			// _err("%s: try: %s",__func__,tryfile);
			found = find_library(cjit->reallibs, tryfile);
			if(found==0) {
				// _err("library found: %s",tryfile);
				break;
			}
		}
	}
	return(XArray_Used(cjit->reallibs));
}

static int find_library(xarray_t *results, const char *path) {
    struct stat statbuf;
	char reallib[PATH_MAX];

	if (lstat(path, &statbuf) == -1) {
		// fail(path);
        return -1;
    }
    if (S_ISLNK(statbuf.st_mode)) {
        ssize_t len = readlink(path, reallib, PATH_MAX - 1);
        if (len == -1) {
            fail(path);
			return -1;
        }
        reallib[len] = 0x0;
		// _err("symlink to: %s",reallib);
    } else if (S_ISREG(statbuf.st_mode)) {
		strcpy(reallib,path);
    }
	FILE *fd = fopen(reallib,"r");
	if(!fd) {
		// fail(reallib);
		return -2;
	}
	int ch;
	int i = 0;
	char elf[4];
	bool is_ldscript = true;
	while ((ch = fgetc(fd)) != EOF) {

		// Check for ELF binary signature
		if(i<4) elf[i]=ch;
		if(i==4
		   && elf[0] == 0x7f
		   && elf[1] == 'E'
		   && elf[2] == 'L'
		   && elf[3] == 'F') {
			is_ldscript = false;
			break;
		}

		// check for ASCII
		if (ch < 0 || ch > 127) {
			is_ldscript = false;
			break;
		}
		if(i++ > 64) break; // just the first 64 bytes should be
							// enough to know if file is a gnu ld
							// script or not ?
	}
	fclose(fd);
	if(is_ldscript) {
		LDState s1;
		s1.libs = results;
		resolve_ldscript(&s1, reallib);
	} else {
		XArray_AddData(results,reallib,strlen(reallib));
	}
	return 0;
}

static int ld_inp(LDState *s1)
{
    char b;
    if (s1->cc != -1) {
        int c = s1->cc;
        s1->cc = -1;
        return c;
    }
    if (1 == read(s1->fd, &b, 1))
        return b;
    return CH_EOF;
}

/* return next ld script token */
static int ld_next(LDState *s1, char *name, int name_size)
{
    int c, d, ch;
    char *q;

 redo:
    ch = ld_inp(s1);
    switch(ch) {
    case ' ':
    case '\t':
    case '\f':
    case '\v':
    case '\r':
    case '\n':
        goto redo;
    case '/':
        ch = ld_inp(s1);
        if (ch == '*') { /* comment */
            for (d = 0;; d = ch) {
                ch = ld_inp(s1);
                if (ch == CH_EOF || (ch == '/' && d == '*'))
                    break;
            }
            goto redo;
        } else {
            q = name;
            *q++ = '/';
            goto parse_name;
        }
        break;
    case '\\':
    /* case 'a' ... 'z': */
    case 'a':
       case 'b':
       case 'c':
       case 'd':
       case 'e':
       case 'f':
       case 'g':
       case 'h':
       case 'i':
       case 'j':
       case 'k':
       case 'l':
       case 'm':
       case 'n':
       case 'o':
       case 'p':
       case 'q':
       case 'r':
       case 's':
       case 't':
       case 'u':
       case 'v':
       case 'w':
       case 'x':
       case 'y':
       case 'z':
    /* case 'A' ... 'z': */
    case 'A':
       case 'B':
       case 'C':
       case 'D':
       case 'E':
       case 'F':
       case 'G':
       case 'H':
       case 'I':
       case 'J':
       case 'K':
       case 'L':
       case 'M':
       case 'N':
       case 'O':
       case 'P':
       case 'Q':
       case 'R':
       case 'S':
       case 'T':
       case 'U':
       case 'V':
       case 'W':
       case 'X':
       case 'Y':
       case 'Z':
    case '_':
    case '.':
    case '$':
    case '~':
        q = name;
    parse_name:
        for(;;) {
            if (!((ch >= 'a' && ch <= 'z') ||
                  (ch >= 'A' && ch <= 'Z') ||
                  (ch >= '0' && ch <= '9') ||
                  strchr("/.-_+=$:\\,~", ch)))
                break;
            if ((q - name) < name_size - 1) {
                *q++ = ch;
            }
            ch = ld_inp(s1);
        }
        s1->cc = ch;
        *q = '\0';
        c = LD_TOK_NAME;
        break;
    case CH_EOF:
        c = LD_TOK_EOF;
        break;
    default:
        c = ch;
        break;
    }
    return c;
}

static int ld_add_file(LDState *s1, const char filename[]) {
	XArray_AddData(s1->libs, (char*)filename, strlen(filename));
	return 1;
}

static int tcc_error_noabort(char *msg) {
	_err("Error in ldscript parser: %s",msg);
	return 1;
}

static int ld_add_file_list(LDState *s1, const char *cmd, int as_needed) {
    char filename[MAX_PATH], libname[MAX_PATH-8];
    int t, group, nblibs = 0, ret = 0;
    char **libs = NULL;
    group = !strcmp(cmd, "GROUP");
    if (!as_needed)
        s1->new_undef_sym = 0;
    t = ld_next(s1, filename, sizeof(filename));
    if (t != '(') {
        ret = tcc_error_noabort("( expected");
        goto lib_parse_error;
    }
    t = ld_next(s1, filename, sizeof(filename));
    for(;;) {
        libname[0] = '\0';
        if (t == LD_TOK_EOF) {
            ret = tcc_error_noabort("unexpected end of file");
            goto lib_parse_error;
        } else if (t == ')') {
            break;
        } else if (t == '-') {
            t = ld_next(s1, filename, sizeof(filename));
            if ((t != LD_TOK_NAME) || (filename[0] != 'l')) {
                ret = tcc_error_noabort("library name expected");
                goto lib_parse_error;
            }
            pstrcpy(libname, sizeof libname, &filename[1]);
            if (s1->static_link) {
                snprintf(filename, sizeof filename, "lib%s.a", libname);
            } else {
                snprintf(filename, sizeof filename, "lib%s.so", libname);
            }
        } else if (t != LD_TOK_NAME) {
            ret = tcc_error_noabort("filename expected");
            goto lib_parse_error;
        }
        if (!strcmp(filename, "AS_NEEDED")) {
            ret = ld_add_file_list(s1, cmd, 1);
            if (ret)
                goto lib_parse_error;
        } else {
			ret = ld_add_file(s1, filename);
			if (ret)
				goto lib_parse_error;
			if (group) {
				/* Add the filename *and* the libname to avoid future conversions */
				dynarray_add(&libs, &nblibs, tcc_strdup(filename));
				if (libname[0] != '\0')
					dynarray_add(&libs, &nblibs, tcc_strdup(libname));
			}

        }
        t = ld_next(s1, filename, sizeof(filename));
        if (t == ',') {
            t = ld_next(s1, filename, sizeof(filename));
        }
    }
    if (group && !as_needed) {
        while (s1->new_undef_sym) {
            int i;
            s1->new_undef_sym = 0;
            for (i = 0; i < nblibs; i ++)
                ld_add_file(s1, libs[i]);
        }
    }
 lib_parse_error:
    dynarray_reset(&libs, &nblibs);
    return ret;
}

// fd = open(filename, O_RDONLY | O_BINARY);
static int resolve_ldscript(LDState *s1, char *path) {
    char cmd[64];
    char filename[1024];
    int t, ret;
	int fd = open(path, O_RDONLY | O_BINARY);
	if(fd<0) {
		fail(path);
		return -1;
	}
    s1->fd = fd;
    s1->cc = -1;
    for(;;) {
        t = ld_next(s1, cmd, sizeof(cmd));
        if (t == LD_TOK_EOF) {
			close(fd);
            return 0;
		}
        else if (t != LD_TOK_NAME) {
			close(fd);
            return -1;
		}
        if (!strcmp(cmd, "INPUT") ||
            !strcmp(cmd, "GROUP")) {
            ret = ld_add_file_list(s1, cmd, 0);
            if (ret) {
				close(fd);
				break;
			}
        } else if (!strcmp(cmd, "OUTPUT_FORMAT") ||
                   !strcmp(cmd, "TARGET")) {
            /* ignore some commands */
            t = ld_next(s1, cmd, sizeof(cmd));
            if (t != '(') {
					_err("%s: ( expected while parsing %s",
						 __func__,path);
					close(fd);
					break;
			}
            for(;;) {
                t = ld_next(s1, filename, sizeof(filename));
                if (t == LD_TOK_EOF) {
					_err("%s: unexpected end of file in %s",
						 __func__,path);
					close(fd);
					return 0;
                } else if (t == ')') {
                    break;
                }
            }
        } else {
			close(fd);
            return -1;
        }
    }
	close(fd);
    return ret;
}

#endif // only GNU/Linux platform
