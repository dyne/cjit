#ifndef CJIT_DOMAIN_ERROR_H
#define CJIT_DOMAIN_ERROR_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Result codes shared by application slices and adapters.
 *
 * The goal is to make intent explicit without forcing callers to
 * interpret a mix of booleans, negative integers, and side effects.
 */
typedef enum CJITResultCode {
    CJIT_RESULT_OK = 0,
    CJIT_RESULT_INVALID_REQUEST = 1,
    CJIT_RESULT_IO_ERROR = 2,
    CJIT_RESULT_COMPILER_ERROR = 3,
    CJIT_RESULT_LINK_ERROR = 4,
    CJIT_RESULT_EXEC_ERROR = 5,
    CJIT_RESULT_PLATFORM_ERROR = 6
} CJITResultCode;

/**
 * Describes one operation outcome in a transport-neutral way.
 */
typedef struct CJITResult {
    CJITResultCode code;
    int exit_status;
    bool ok;
    const char *message;
} CJITResult;

#endif
