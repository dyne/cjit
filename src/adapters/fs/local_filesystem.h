#ifndef CJIT_ADAPTERS_FS_LOCAL_FILESYSTEM_H
#define CJIT_ADAPTERS_FS_LOCAL_FILESYSTEM_H

#include <stdbool.h>

#include "ports/filesystem_port.h"

extern const FilesystemPort local_filesystem_port;

/**
 * Return whether a versioned runtime cache has every minimum required file.
 * This performs no mutation and is kept separate from cache refresh policy.
 */
bool cjit_runtime_cache_is_complete(const char *root);

#endif
