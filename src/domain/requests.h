#ifndef CJIT_DOMAIN_REQUESTS_H
#define CJIT_DOMAIN_REQUESTS_H

#include <stdbool.h>

/**
 * Shared execution-related options used by the main CLI routes.
 */
typedef struct CJITCommonOptions {
    bool quiet;
    bool verbose;
    bool print_status;
    const char *entry;
    const char *pid_file;
    const char *output_path;
} CJITCommonOptions;

/**
 * Request for the in-memory execute route.
 */
typedef struct ExecuteRequest {
    CJITCommonOptions options;
    int source_count;
    const char **sources;
    int app_argc;
    char **app_argv;
} ExecuteRequest;

/**
 * Request for compile-only object output.
 */
typedef struct CompileObjectRequest {
    CJITCommonOptions options;
    const char *source_path;
} CompileObjectRequest;

/**
 * Request for executable output without execution.
 */
typedef struct BuildExecutableRequest {
    CJITCommonOptions options;
    int source_count;
    const char **sources;
} BuildExecutableRequest;

/**
 * Request for asset extraction.
 */
typedef struct ExtractAssetsRequest {
    const char *destination_path;
} ExtractAssetsRequest;

/**
 * Request for tar.gz extraction.
 */
typedef struct ExtractArchiveRequest {
    const char *archive_path;
} ExtractArchiveRequest;

/**
 * Request for status printing without source execution.
 */
typedef struct StatusRequest {
    bool verbose;
} StatusRequest;

#endif
