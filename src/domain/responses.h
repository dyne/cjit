#ifndef CJIT_DOMAIN_RESPONSES_H
#define CJIT_DOMAIN_RESPONSES_H

#include "domain/error.h"

/**
 * Response for routes that execute code.
 */
typedef struct ExecuteResponse {
    CJITResult result;
} ExecuteResponse;

/**
 * Response for compile-only object generation.
 */
typedef struct CompileObjectResponse {
    CJITResult result;
    const char *output_path;
} CompileObjectResponse;

/**
 * Response for executable build generation.
 */
typedef struct BuildExecutableResponse {
    CJITResult result;
    const char *output_path;
} BuildExecutableResponse;

/**
 * Response for asset extraction.
 */
typedef struct ExtractAssetsResponse {
    CJITResult result;
    const char *destination_path;
} ExtractAssetsResponse;

/**
 * Response for archive extraction.
 */
typedef struct ExtractArchiveResponse {
    CJITResult result;
} ExtractArchiveResponse;

/**
 * Response for runtime/compiler status output.
 */
typedef struct StatusResponse {
    CJITResult result;
} StatusResponse;

#endif
