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
#if defined(LINUX)
#define MAX_PATH 512
#include <cjit.h>
#include <elflinker.h>
#include <array.h>
#include <cwalk.h>

#define CH_EOF   (-1) //end of file
#define LD_TOK_NAME 256
#define LD_TOK_EOF  (-1)

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

// tinyCC internals used here
extern void dynarray_add(void *ptab, int *nb_ptr, void *data);
extern void dynarray_reset(void *pp, int *n);
extern char *tcc_strdup(const char *str);
extern char *pstrcpy(char *buf, size_t buf_size, const char *s);

bool read_ldsoconf(xarray_t *dest, const char *directory) {
    DIR *dir;
    struct dirent *entry;
    char path[MAX_PATH];
    dir = opendir(directory);
    if (dir == NULL) {
		_err("%s: error reading directory: %s",__func__,directory);
        return false;
    }
    while ((entry = readdir(dir)) != NULL) {
		struct stat st;
		cwk_path_join(directory,entry->d_name,path,MAX_PATH);
        // Check if it's a regular file
        if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
            FILE *file = fopen(path, "r");
            if (!file) {
				_err("%s: error opening file: %s",__func__,path);
                continue;
            }
            char line[512];
            while (fgets(line, sizeof(line), file) != NULL) {
                // Skip lines that are comments
                if (line[0] != '/') continue;
				size_t len = strlen(line);
				if(line[len-1]=='\n') line[len-1]=0x0;
                // add the line to the result array
				XArray_AddData(dest,line,len);
            }
            fclose(file);
        }
    }
    closedir(dir);
    return true;
}

void detect_file_type(const char *filename) {
    struct stat st;
    if (lstat(filename, &st) == -1) {
        perror("lstat");
        return;
    }

    if (S_ISLNK(st.st_mode)) {
        printf("%s is a symbolic link.\n", filename);
        return;
    }

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return;
    }

    unsigned char buffer[512];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
    if (bytes_read == -1) {
        perror("read");
        close(fd);
        return;
    }

    close(fd);

    // Check for plain text ASCII
    int is_ascii = 1;
    for (ssize_t i = 0; i < bytes_read; i++) {
        if (buffer[i] > 127 || (buffer[i] < 32 && buffer[i] != '\n' && buffer[i] != '\r' && buffer[i] != '\t')) {
            is_ascii = 0;
            break;
        }
    }

    if (is_ascii) {
        printf("%s is a plain text ASCII file.\n", filename);
        return;
    }

    // Check for ELF binary (common format for shared libraries)
    if (bytes_read >= 4 && buffer[0] == 0x7f && buffer[1] == 'E' && buffer[2] == 'L' && buffer[3] == 'F') {
        printf("%s is a binary shared library (ELF).\n", filename);
        return;
    }

    printf("%s is an unknown binary file.\n", filename);
}

#if defined(TEST_LINKER)
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }
    detect_file_type(argv[1]);
    return EXIT_SUCCESS;
}
#endif

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
	_err("LD: %s",filename);
	return 1;
}

static int tcc_error_noabort(char *msg) {
	_err("Error in ldscript parser: %s",msg);
	return 1;
}

static int ld_add_file_list(LDState *s1, const char *cmd, int as_needed)
{
    char filename[1024], libname[1024];
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
            /* TODO: Implement AS_NEEDED support. */
	    /*       DT_NEEDED is not used any more so ignore as_needed */
            if (1 || !as_needed) {
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
int cjit_load_ldscript(LDState *s1, char *path) {
    char cmd[64];
    char filename[1024];
    int t, ret;
	int fd = open(path, O_RDONLY | O_BINARY);

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
				return ret;
			}
        } else if (!strcmp(cmd, "OUTPUT_FORMAT") ||
                   !strcmp(cmd, "TARGET")) {
            /* ignore some commands */
            t = ld_next(s1, cmd, sizeof(cmd));
            if (t != '(') {
					_err("%s: ( expected while parsing %s",
						 __func__,path);
					close(fd);
					return 0;
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
    return 0;
}

#endif // only GNU/Linux platform
