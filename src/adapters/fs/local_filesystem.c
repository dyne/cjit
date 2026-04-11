#include "adapters/fs/local_filesystem.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "cjit.h"

extern char *load_stdin();
extern char *new_abspath(const char *path);
extern bool write_to_file(const char *path, const char *filename, const char *buf, unsigned int len);

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
