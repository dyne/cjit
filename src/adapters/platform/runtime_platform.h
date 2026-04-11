#ifndef CJIT_ADAPTERS_PLATFORM_RUNTIME_PLATFORM_H
#define CJIT_ADAPTERS_PLATFORM_RUNTIME_PLATFORM_H

#include "cjit.h"

/**
 * Apply platform-specific runtime setup such as compatibility symbols,
 * default library search paths, and host include paths.
 */
void cjit_platform_setup_runtime(CJITState *cjit);

#endif
