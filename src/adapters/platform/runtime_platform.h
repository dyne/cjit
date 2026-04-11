#ifndef CJIT_ADAPTERS_PLATFORM_RUNTIME_PLATFORM_H
#define CJIT_ADAPTERS_PLATFORM_RUNTIME_PLATFORM_H

#include "cjit.h"

/**
 * Apply platform-specific runtime setup such as compatibility symbols,
 * default library search paths, and host include paths.
 */
void cjit_platform_setup_runtime(CJITState *cjit);

/**
 * Executes the compiled entrypoint using the host platform process model.
 */
int cjit_platform_exec(CJITState *cjit, int (*entrypoint)(int, char **),
                       int argc, char **argv);

/**
 * Records a user-provided library path using the host platform rules.
 */
void cjit_platform_add_library_path(CJITState *cjit, const char *path);

#endif
