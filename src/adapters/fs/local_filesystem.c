#include "adapters/fs/local_filesystem.h"

#include <stdbool.h>
#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <ftw.h>
#include <unistd.h>
#endif

#include "support/cwalk.h"

#include "cjit.h"

extern char *load_stdin();
extern char *new_abspath(const char *path);
extern bool write_to_file(const char *path, const char *filename, const char *buf, unsigned int len);

/**
 * Creates one directory level when it is missing and rejects file collisions.
 */
static bool ensure_directory(const char *path)
{
    struct stat info;

    if (stat(path, &info) == 0) {
        if (info.st_mode & S_IFDIR) {
            return true;
        }
        _err("Temp dir is a file, cannot overwrite: %s", path);
        return false;
    }
#if defined(WINDOWS)
    if (CreateDirectory(path, NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS) {
#else
    if (mkdir(path, 0755) == 0 || errno == EEXIST) {
#endif
        return true;
    }
    fail(path);
    return false;
}

/**
 * Checks whether the cached embedded runtime contains the minimum files
 * required to compile simple sources.
 */
static bool runtime_cache_is_complete(const char *root)
{
    struct stat info;
    char path[MAX_PATH];
    struct RuntimeCacheEntry {
        const char *path;
        off_t min_size;
    };
    const struct RuntimeCacheEntry required[] = {
        { "libtcc1.a", 1024 },
        { "include/stdarg.h", 64 },
        { "include/stddef.h", 64 },
        { "include/tccdefs.h", 1024 },
    };
    size_t i;

#if defined(WINDOWS)
    const struct RuntimeCacheEntry windows_required[] = {
        { "tinycc_win32/stdio.h", 1024 },
        { "tinycc_win32/_mingw.h", 256 },
        { "win32ports/unistd.h", 256 },
    };
#endif

    for (i = 0; i < sizeof(required) / sizeof(required[0]); ++i) {
        cwk_path_join(root, required[i].path, path, sizeof(path));
        if (stat(path, &info) != 0 || !(info.st_mode & S_IFREG) || info.st_size < required[i].min_size) {
            return false;
        }
    }

#if defined(WINDOWS)
    for (i = 0; i < sizeof(windows_required) / sizeof(windows_required[0]); ++i) {
        cwk_path_join(root, windows_required[i].path, path, sizeof(path));
        if (stat(path, &info) != 0 || !(info.st_mode & S_IFREG) || info.st_size < windows_required[i].min_size) {
            return false;
        }
    }
#endif

    return true;
}

#if defined(WINDOWS)
/**
 * Removes one cached runtime tree recursively so broken files do not survive
 * a refresh attempt.
 */
static bool remove_tree(const char *path)
{
    WIN32_FIND_DATA find_data;
    HANDLE handle;
    char pattern[MAX_PATH];
    char child[MAX_PATH];

    cwk_path_join(path, "*", pattern, sizeof(pattern));
    handle = FindFirstFile(pattern, &find_data);
    if (handle != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
                continue;
            }
            cwk_path_join(path, find_data.cFileName, child, sizeof(child));
            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (!remove_tree(child)) {
                    FindClose(handle);
                    return false;
                }
            } else if (DeleteFile(child) == 0) {
                fail(child);
                FindClose(handle);
                return false;
            }
        } while (FindNextFile(handle, &find_data) != 0);
        FindClose(handle);
    }

    if (RemoveDirectory(path) == 0) {
        fail(path);
        return false;
    }
    return true;
}
#else
static int remove_tree_entry(const char *fpath, const struct stat *sb,
                             int typeflag, struct FTW *ftwbuf)
{
    (void)sb;
    (void)typeflag;
    (void)ftwbuf;
    return remove(fpath);
}

/**
 * Removes one cached runtime tree recursively so broken files do not survive
 * a refresh attempt.
 */
static bool remove_tree(const char *path)
{
    if (nftw(path, remove_tree_entry, 16, FTW_DEPTH | FTW_PHYS) != 0) {
        fail(path);
        return false;
    }
    return true;
}
#endif

/**
 * Creates or reuses the runtime temp directory and records whether it was
 * freshly created for this session.
 */
bool cjit_mkdtemp(CJITState *cjit, const char *optional_path)
{
    char *temp_dir;

    if (optional_path) {
        temp_dir = malloc(strlen(optional_path) + 512);
        cwk_path_normalize(optional_path, temp_dir, strlen(optional_path) + 511);
#if defined(WINDOWS)
        if (CreateDirectory(temp_dir, NULL) == 0) {
            _err("Failed to create temporary dir: %s", temp_dir);
            return false;
        }
#else
        struct stat info;
        if (stat(temp_dir, &info) != 0) {
            cjit->fresh = true;
        } else if (info.st_mode & S_IFDIR) {
            cjit->fresh = false;
        } else {
            _err("Cannot overwrite runtime include dir: %s", temp_dir);
            return false;
        }
        if (cjit->fresh) {
            mkdir(temp_dir, 0755);
        }
#endif
    } else {
        temp_dir = malloc(MAX_PATH + 1);
        const char *tmp_root = getenv("TMPDIR");
        char cache_root[MAX_PATH];
        struct stat info;

#if defined(WINDOWS)
        char temp_path[MAX_PATH];

        if (!tmp_root || tmp_root[0] == '\0') {
            tmp_root = getenv("TEMP");
        }
        if (!tmp_root || tmp_root[0] == '\0') {
            tmp_root = getenv("TMP");
        }
        if ((!tmp_root || tmp_root[0] == '\0') && GetTempPath(MAX_PATH, temp_path) != 0) {
            tmp_root = temp_path;
        }
#endif
        if (!tmp_root || tmp_root[0] == '\0') {
            tmp_root = "/tmp";
        }
        cwk_path_join(tmp_root, "cjit", cache_root, sizeof(cache_root));
        if (!ensure_directory(cache_root)) {
            free(temp_dir);
            return false;
        }

        cwk_path_join(cache_root, VERSION, temp_dir, MAX_PATH);
        if (stat(temp_dir, &info) != 0) {
            cjit->fresh = true;
        } else if (info.st_mode & S_IFDIR) {
            cjit->fresh = !runtime_cache_is_complete(temp_dir);
            if (cjit->fresh) {
                if (!remove_tree(temp_dir) || !ensure_directory(temp_dir)) {
                    free(temp_dir);
                    return false;
                }
            }
        } else {
            _err("Temp dir is a file, cannot overwrite: %s", temp_dir);
            free(temp_dir);
            return false;
        }
        if (cjit->fresh && !ensure_directory(temp_dir)) {
            free(temp_dir);
            return false;
        }
    }

    cjit->tmpdir = malloc(strlen(temp_dir) + 1);
    strcpy(cjit->tmpdir, temp_dir);
    free(temp_dir);
    return true;
}

static CJITResult read_file_impl(void *context, const char *path, char **contents, size_t *length)
{
    unsigned int len = 0;
    (void)context;
    *contents = file_load(path, &len);
    if (!*contents) {
        return cjit_result_error(CJIT_RESULT_IO_ERROR, 1, "Failed to read file");
    }
    if (length) {
        *length = len;
    }
    return cjit_result_ok();
}

static CJITResult read_stdin_impl(void *context, char **contents, size_t *length)
{
    (void)context;
    *contents = load_stdin();
    if (!*contents) {
        return cjit_result_error(CJIT_RESULT_IO_ERROR, 1, "Error reading from standard input");
    }
    if (length) {
        *length = *contents ? strlen(*contents) : 0;
    }
    return cjit_result_ok();
}

static CJITResult detect_source_encoding_impl(void *context, const char *path,
                                              SourceEncoding *encoding, size_t *length)
{
    unsigned int len = 0;
    char *contents;
    (void)context;
    contents = file_load(path, &len);
    if (!contents) {
        return cjit_result_error(CJIT_RESULT_IO_ERROR, 1, "Failed to inspect source encoding");
    }
    free(contents);
    if (encoding) {
        *encoding = SOURCE_ENCODING_UNKNOWN;
    }
    if (length) {
        *length = len;
    }
    return cjit_result_ok();
}

static CJITResult make_absolute_path_impl(void *context, const char *path, char **absolute_path)
{
    (void)context;
    *absolute_path = new_abspath(path);
    if (!*absolute_path) {
        return cjit_result_error(CJIT_RESULT_IO_ERROR, 1, "Failed to resolve absolute path");
    }
    return cjit_result_ok();
}

static CJITResult write_file_impl(void *context, const char *dir, const char *filename,
                                  const char *contents, unsigned int length)
{
    (void)context;
    if (!write_to_file(dir, filename, contents, length)) {
        return cjit_result_error(CJIT_RESULT_IO_ERROR, 1, "Failed to write file");
    }
    return cjit_result_ok();
}

static CJITResult ensure_tempdir_impl(void *context, const char *requested_path,
                                      char **resolved_path, bool *is_fresh)
{
    CJITState *cjit = (CJITState *)context;
    if (!cjit_mkdtemp(cjit, requested_path)) {
        return cjit_result_error(CJIT_RESULT_IO_ERROR, 1, "Failed to create tempdir");
    }
    if (resolved_path) {
        *resolved_path = cjit->tmpdir;
    }
    if (is_fresh) {
        *is_fresh = cjit->fresh;
    }
    return cjit_result_ok();
}

const FilesystemPort local_filesystem_port = {
    .context = NULL,
    .read_file = read_file_impl,
    .read_stdin = read_stdin_impl,
    .detect_source_encoding = detect_source_encoding_impl,
    .make_absolute_path = make_absolute_path_impl,
    .write_file = write_file_impl,
    .ensure_tempdir = ensure_tempdir_impl
};
