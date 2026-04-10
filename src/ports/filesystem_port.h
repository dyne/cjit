#ifndef CJIT_PORTS_FILESYSTEM_PORT_H
#define CJIT_PORTS_FILESYSTEM_PORT_H

#include <stdbool.h>
#include <stddef.h>

#include "domain/error.h"

/**
 * Small source encoding classification used when inspecting input files.
 */
typedef enum SourceEncoding {
    SOURCE_ENCODING_UNKNOWN = 0,
    SOURCE_ENCODING_PLAIN,
    SOURCE_ENCODING_UTF8_BOM,
    SOURCE_ENCODING_UTF16_LE,
    SOURCE_ENCODING_UTF16_BE
} SourceEncoding;

/**
 * Filesystem and path operations needed by application slices.
 */
typedef struct FilesystemPort {
    void *context;
    CJITResult (*read_file)(void *context, const char *path, char **contents, size_t *length);
    CJITResult (*read_stdin)(void *context, char **contents, size_t *length);
    CJITResult (*detect_source_encoding)(void *context, const char *path,
                                         SourceEncoding *encoding, size_t *length);
    CJITResult (*make_absolute_path)(void *context, const char *path, char **absolute_path);
    CJITResult (*write_file)(void *context, const char *dir, const char *filename,
                             const char *contents, unsigned int length);
    CJITResult (*ensure_tempdir)(void *context, const char *requested_path,
                                 char **resolved_path, bool *is_fresh);
} FilesystemPort;

#endif
