#include "adapters/fs/local_filesystem.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "cwalk.h"

#include "cjit.h"

extern char *load_stdin();
extern char *new_abspath(const char *path);
extern bool write_to_file(const char *path, const char *filename, const char *buf, unsigned int len);

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
#if defined(WINDOWS)
        char temp_path[MAX_PATH];
        char dirname[64];

        snprintf(dirname, 63, "CJIT-%s", VERSION);
        if (GetTempPath(MAX_PATH, temp_path) == 0) {
            _err("Failed to get temporary path");
            return false;
        }
        cwk_path_join(temp_path, dirname, temp_dir, MAX_PATH);
        DWORD attributes = GetFileAttributes(temp_dir);
        if (attributes == INVALID_FILE_ATTRIBUTES) {
            cjit->fresh = true;
        } else if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
            cjit->fresh = false;
        } else {
            _err("Temp dir is a file, cannot overwrite: %s", temp_dir);
            return false;
        }
        if (cjit->fresh && CreateDirectory(temp_dir, NULL) == 0) {
            _err("Failed to create temporary dir: %s", temp_dir);
            return false;
        }
#else
        struct stat info;

        snprintf(temp_dir, 259, "/tmp/cjit-%s", VERSION);
        if (stat(temp_dir, &info) != 0) {
            cjit->fresh = true;
        } else if (info.st_mode & S_IFDIR) {
            cjit->fresh = false;
        } else {
            _err("Temp dir is a file, cannot overwrite: %s", temp_dir);
            return false;
        }
        if (cjit->fresh) {
            mkdir(temp_dir, 0755);
        }
#endif
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
