#ifndef CJIT_DOMAIN_RUNTIME_SESSION_H
#define CJIT_DOMAIN_RUNTIME_SESSION_H

#include <stdbool.h>

/**
 * Mutable runtime/compiler session state that should eventually replace
 * the transport-facing parts of CJITState.
 *
 * The handle stays opaque here as `void *` so the current TinyCC-backed
 * implementation can be migrated incrementally.
 */
typedef struct RuntimeSession {
    void *compiler_handle;
    const char *tmpdir;
    bool tempdir_is_fresh;
    bool setup_complete;
    bool execution_complete;
} RuntimeSession;

#endif
