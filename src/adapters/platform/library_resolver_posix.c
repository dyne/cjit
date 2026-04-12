/*
 *  Taken from ELF file handling in TCC
 *
 *  Copyright (c) 2001-2004 Fabrice Bellard
 *  Copyright (c) 2024-2026 Dyne.org
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

// Interpret a subset of GNU ld scripts to resolve shared libraries.

#include "adapters/platform/library_resolver_posix.h"

#include "adapters/platform/build_platform.h"

#if defined(POSIX)

#define MAX_PATH 512
#define CH_EOF (-1)
#define LD_TOK_NAME 256
#define LD_TOK_EOF (-1)

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "cjit.h"
#include "support/cwalk.h"
#include "support/string_list.h"

typedef struct LDState {
    int cc;
    int fd;
    int new_undef_sym;
    int static_link;
    StringList *libs;
    StringList *libpaths;
} LDState;

// TinyCC internals used by the ldscript parser.
extern void dynarray_add(void *ptab, int *nb_ptr, void *data);
extern void dynarray_reset(void *pp, int *n);
extern char *tcc_strdup(const char *str);
extern char *pstrcpy(char *buf, size_t buf_size, const char *s);

static int resolve_ldscript(LDState *state, char *path);
static int find_library(CJITState *cjit, const char *path);
static int posix_resolve_libs(CJITState *cjit);
static int ld_inp(LDState *state);
static int ld_next(LDState *state, char *name, int name_size);
static int ld_add_file(LDState *state, const char filename[]);
static int ld_add_file_list(LDState *state, const char *cmd, int as_needed);

bool read_ldsoconf(StringList *dest, char *path)
{
    FILE *file;
    char line[MAX_PATH];

    file = fopen(path, "r");
    if (!file) {
        fail(path);
        return false;
    }

    while (fgets(line, MAX_PATH, file) != NULL) {
        size_t len;

        if (line[0] != '/') {
            continue;
        }
        len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = 0x0;
        }
        string_list_add(dest, line);
    }
    fclose(file);
    return true;
}

bool read_ldsoconf_dir(StringList *dest, const char *directory)
{
    struct dirent **namelist;
    char path[MAX_PATH];
    int n;
    int i;

    n = scandir(directory, &namelist, NULL, alphasort);
    if (n < 0) {
        fail(directory);
        return false;
    }
    for (i = 0; i < n; i++) {
        if (namelist[i]->d_type == DT_REG) {
            cwk_path_join(directory, namelist[i]->d_name, path, MAX_PATH);
            if (!read_ldsoconf(dest, path)) {
                fail(directory);
            }
        }
        free(namelist[i]);
    }
    free(namelist);
    return true;
}

static CJITResult resolve_impl(void *context,
                               const LibraryResolverRequest *request,
                               LibraryResolverResponse *response)
{
    CJITState *cjit;

    cjit = (CJITState *)context;
    (void)request;
    response->resolved_count = posix_resolve_libs(cjit);
    response->resolved_paths = NULL;
    return cjit_result_ok();
}

const LibraryResolverPort posix_library_resolver_port = {
    .context = NULL,
    .resolve = resolve_impl
};

static int posix_resolve_libs(CJITState *cjit)
{
    char tryfile[PATH_MAX];
    int found;
    int i;
    int ii;
    int libnames_num;
    int libpaths_num;
    char *lname;
    char *lpath;

    libpaths_num = (int)string_list_count(cjit->libpaths);
    libnames_num = (int)string_list_count(cjit->libs);
    found = -1;
    for (i = 0; i < libnames_num; i++) {
        lname = string_list_get(cjit->libs, i);
        found = -1;
        for (ii = 0; ii < libpaths_num; ii++) {
            lpath = string_list_get(cjit->libpaths, ii);
            snprintf(tryfile, PATH_MAX - 2, "%s/lib%s.so", lpath, lname);
            found = find_library(cjit, tryfile);
            if (found == 0) {
                break;
            }
        }
        if (found != 0) {
            _err("Library not found: lib%s.so", lname);
        }
    }
    return (int)string_list_count(cjit->reallibs);
}

static char *new_solve_symlink(const char *path)
{
    struct stat statbuf;
    char *reallib;

    reallib = NULL;
    if (lstat(path, &statbuf) == -1) {
        return NULL;
    }
    if (S_ISLNK(statbuf.st_mode)) {
        int tlen;
        char *tpath;
        ssize_t len;
        size_t rlen;

        tlen = strlen(path);
        tpath = malloc(tlen);
        len = readlink(path, tpath, tlen - 1);
        if (len == -1) {
            free(tpath);
            fail(path);
            return NULL;
        }
        tpath[len] = 0x0;
        rlen = tlen + len;
        reallib = malloc(rlen + 1);
        cwk_path_get_dirname(path, &rlen);
        memcpy(reallib, path, rlen);
        strcpy(&reallib[rlen], tpath);
        free(tpath);
    } else if (S_ISREG(statbuf.st_mode)) {
        reallib = malloc(strlen(path) + 1);
        strcpy(reallib, path);
    }
    if (!reallib) {
        _err("%s: internal error: %s", __func__, path);
        return NULL;
    }
    return reallib;
}

static int find_library(CJITState *cjit, const char *path)
{
    FILE *fd;
    int ch;
    int i;
    int rr;
    bool is_ldscript;
    char elf[4];
    char *reallib;
    LDState state;

    reallib = new_solve_symlink(path);
    if (!reallib) {
        return -1;
    }
    fd = fopen(reallib, "r");
    if (!fd) {
        free(reallib);
        return -2;
    }
    i = 0;
    is_ldscript = true;
    while ((ch = fgetc(fd)) != EOF) {
        if (i < 4) {
            elf[i] = ch;
        }
        if (i == 4 &&
            elf[0] == 0x7f &&
            elf[1] == 'E' &&
            elf[2] == 'L' &&
            elf[3] == 'F') {
            is_ldscript = false;
            break;
        }
        if (ch < 0 || ch > 127) {
            is_ldscript = false;
            break;
        }
        if (i++ > 64) {
            break;
        }
    }
    fclose(fd);
    if (is_ldscript) {
        state.libs = cjit->reallibs;
        state.libpaths = cjit->libpaths;
        rr = resolve_ldscript(&state, reallib);
        if (rr < 1) {
            _err("Library not found: %s", reallib);
        }
    } else {
        string_list_add(cjit->reallibs, reallib);
    }
    free(reallib);
    return 0;
}

static int ld_inp(LDState *state)
{
    char byte;

    if (state->cc != -1) {
        int ch;

        ch = state->cc;
        state->cc = -1;
        return ch;
    }
    if (1 == read(state->fd, &byte, 1)) {
        return byte;
    }
    return CH_EOF;
}

static int ld_next(LDState *state, char *name, int name_size)
{
    int c;
    int d;
    int ch;
    char *q;

redo:
    ch = ld_inp(state);
    switch (ch) {
    case ' ':
    case '\t':
    case '\f':
    case '\v':
    case '\r':
    case '\n':
        goto redo;
    case '/':
        ch = ld_inp(state);
        if (ch == '*') {
            for (d = 0;; d = ch) {
                ch = ld_inp(state);
                if (ch == CH_EOF || (ch == '/' && d == '*')) {
                    break;
                }
            }
            goto redo;
        }
        q = name;
        *q++ = '/';
        goto parse_name;
    case '\\':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
    case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
    case 'Y': case 'Z':
    case '_':
    case '.':
    case '$':
    case '~':
        q = name;
parse_name:
        for (;;) {
            if (!((ch >= 'a' && ch <= 'z') ||
                  (ch >= 'A' && ch <= 'Z') ||
                  (ch >= '0' && ch <= '9') ||
                  strchr("/.-_+=$:\\,~", ch))) {
                break;
            }
            if ((q - name) < name_size - 1) {
                *q++ = ch;
            }
            ch = ld_inp(state);
        }
        state->cc = ch;
        *q = '\0';
        return LD_TOK_NAME;
    case CH_EOF:
        return LD_TOK_EOF;
    default:
        return ch;
    }
    c = ch;
    return c;
}

static int ld_add_file(LDState *state, const char filename[])
{
    char tryfile[PATH_MAX];
    char *lpath;
    int ii;
    int libpaths_num;
    struct stat statbuf;

    if (cwk_path_is_absolute(filename)) {
        if (lstat(filename, &statbuf) == -1) {
            fail(filename);
            return 0;
        }
        if (S_ISREG(statbuf.st_mode)) {
            strcpy(tryfile, filename);
        } else if (S_ISLNK(statbuf.st_mode)) {
            ssize_t len;

            len = readlink(filename, tryfile, PATH_MAX - 1);
            if (len == -1) {
                fail(filename);
                return 0;
            }
            tryfile[len] = 0x0;
        } else {
            _err("Library file not recognized: %s", filename);
            return 0;
        }
    } else {
        libpaths_num = (int)string_list_count(state->libpaths);
        for (ii = 0; ii < libpaths_num; ii++) {
            lpath = string_list_get(state->libpaths, ii);
            cwk_path_join(lpath, filename, tryfile, PATH_MAX);
            if (lstat(tryfile, &statbuf) == -1) {
                continue;
            }
            if (S_ISREG(statbuf.st_mode)) {
                break;
            }
            if (S_ISLNK(statbuf.st_mode)) {
                int tlen;
                char *tpath;
                ssize_t len;

                tlen = strlen(tryfile) + 64;
                tpath = malloc(tlen);
                len = readlink(tryfile, tpath, tlen - 1);
                if (len == -1) {
                    free(tpath);
                    fail(tryfile);
                    continue;
                }
                tpath[len] = 0x0;
                cwk_path_join(lpath, tpath, tryfile, PATH_MAX);
                free(tpath);
                break;
            }
            _err("Library file not recognized: %s", tryfile);
        }
    }
    string_list_add(state->libs, tryfile);
    return 1;
}

static int tcc_error_noabort(char *msg)
{
    _err("Error in ldscript parser: %s", msg);
    return 1;
}

static int ld_add_file_list(LDState *state, const char *cmd, int as_needed)
{
    char filename[MAX_PATH];
    char libname[MAX_PATH - 8];
    char **libs;
    int group;
    int i;
    int nblibs;
    int ret;
    int t;

    group = !strcmp(cmd, "GROUP");
    libs = NULL;
    nblibs = 0;
    ret = 0;
    if (!as_needed) {
        state->new_undef_sym = 0;
    }
    t = ld_next(state, filename, sizeof(filename));
    if (t != '(') {
        ret = tcc_error_noabort("( expected");
        goto lib_parse_error;
    }
    t = ld_next(state, filename, sizeof(filename));
    for (;;) {
        libname[0] = '\0';
        if (t == LD_TOK_EOF) {
            ret = tcc_error_noabort("unexpected end of file");
            goto lib_parse_error;
        } else if (t == ')') {
            break;
        } else if (t == '-') {
            t = ld_next(state, filename, sizeof(filename));
            if ((t != LD_TOK_NAME) || (filename[0] != 'l')) {
                ret = tcc_error_noabort("library name expected");
                goto lib_parse_error;
            }
            pstrcpy(libname, sizeof libname, &filename[1]);
            if (state->static_link) {
                snprintf(filename, sizeof filename, "lib%s.a", libname);
            } else {
                snprintf(filename, sizeof filename, "lib%s.so", libname);
            }
        } else if (t != LD_TOK_NAME) {
            ret = tcc_error_noabort("filename expected");
            goto lib_parse_error;
        }
        if (!strcmp(filename, "AS_NEEDED")) {
            ret = ld_add_file_list(state, cmd, 1);
            if (ret) {
                goto lib_parse_error;
            }
        } else {
            ret = ld_add_file(state, filename);
            if (ret) {
                goto lib_parse_error;
            }
            if (group) {
                dynarray_add(&libs, &nblibs, tcc_strdup(filename));
                if (libname[0] != '\0') {
                    dynarray_add(&libs, &nblibs, tcc_strdup(libname));
                }
            }
        }
        t = ld_next(state, filename, sizeof(filename));
        if (t == ',') {
            t = ld_next(state, filename, sizeof(filename));
        }
    }
    if (group && !as_needed) {
        while (state->new_undef_sym) {
            state->new_undef_sym = 0;
            for (i = 0; i < nblibs; i++) {
                ld_add_file(state, libs[i]);
            }
        }
    }

lib_parse_error:
    dynarray_reset(&libs, &nblibs);
    return ret;
}

static int resolve_ldscript(LDState *state, char *path)
{
    char cmd[64];
    char filename[1024];
    int fd;
    int ret;
    int t;

    fd = open(path, O_RDONLY | O_BINARY);
    if (fd < 0) {
        fail(path);
        return -1;
    }
    state->fd = fd;
    state->cc = -1;
    for (;;) {
        t = ld_next(state, cmd, sizeof(cmd));
        if (t == LD_TOK_EOF) {
            close(fd);
            return 0;
        }
        if (t != LD_TOK_NAME) {
            close(fd);
            return -1;
        }
        if (!strcmp(cmd, "INPUT") || !strcmp(cmd, "GROUP")) {
            ret = ld_add_file_list(state, cmd, 0);
            if (ret) {
                close(fd);
                break;
            }
        } else if (!strcmp(cmd, "OUTPUT_FORMAT") || !strcmp(cmd, "TARGET")) {
            t = ld_next(state, cmd, sizeof(cmd));
            if (t != '(') {
                _err("%s: ( expected while parsing %s", __func__, path);
                close(fd);
                break;
            }
            for (;;) {
                t = ld_next(state, filename, sizeof(filename));
                if (t == LD_TOK_EOF) {
                    _err("%s: unexpected end of file in %s", __func__, path);
                    close(fd);
                    return 0;
                }
                if (t == ')') {
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

#else

bool read_ldsoconf(StringList *dest, char *path)
{
    (void)dest;
    (void)path;
    return false;
}

bool read_ldsoconf_dir(StringList *dest, const char *directory)
{
    (void)dest;
    (void)directory;
    return false;
}

static CJITResult resolve_impl(void *context,
                               const LibraryResolverRequest *request,
                               LibraryResolverResponse *response)
{
    (void)context;
    (void)request;
    response->resolved_count = 0;
    response->resolved_paths = NULL;
    return cjit_result_error(CJIT_RESULT_PLATFORM_ERROR, 1,
                             "POSIX library resolver unavailable on this platform");
}

const LibraryResolverPort posix_library_resolver_port = {
    .context = NULL,
    .resolve = resolve_impl
};

#endif
