#ifndef CJIT_ADAPTERS_PLATFORM_LIBRARY_RESOLVER_WINDOWS_H
#define CJIT_ADAPTERS_PLATFORM_LIBRARY_RESOLVER_WINDOWS_H

#include "ports/library_resolver_port.h"

#include <stddef.h>

typedef struct StringList StringList;

extern const LibraryResolverPort windows_library_resolver_port;

/* Resolve logical DLL names using only supplied lists. `resolved` owns copies. */
int windows_resolve_library_lists(const StringList *libraries,
                                  const StringList *library_paths,
                                  StringList *resolved);

#endif
