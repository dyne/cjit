#ifndef CJIT_ADAPTERS_PLATFORM_LIBRARY_RESOLVER_POSIX_H
#define CJIT_ADAPTERS_PLATFORM_LIBRARY_RESOLVER_POSIX_H

#include <stdbool.h>

#include "ports/library_resolver_port.h"

typedef struct StringList StringList;

extern const LibraryResolverPort posix_library_resolver_port;

bool read_ldsoconf(StringList *dest, char *path);
bool read_ldsoconf_dir(StringList *dest, const char *directory);

#endif
