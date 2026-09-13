#ifndef CJIT_ADAPTERS_PLATFORM_LIBRARY_RESOLVER_POSIX_H
#define CJIT_ADAPTERS_PLATFORM_LIBRARY_RESOLVER_POSIX_H

#include <stdbool.h>
#include <stddef.h>

#include "ports/library_resolver_port.h"

typedef struct StringList StringList;

extern const LibraryResolverPort posix_library_resolver_port;

bool read_ldsoconf(StringList *dest, const char *path);
bool read_ldsoconf_dir(StringList *dest, const char *directory);

/* Resolve logical libraries using only the supplied lists.  `resolved` owns
 * copies of the discovered paths; repeated library requests remain repeated. */
int posix_resolve_library_lists(const StringList *libraries,
                                const StringList *library_paths,
                                StringList *resolved);

/* Parse ld-script syntax from bytes only; never opens files or resolves paths. */
int posix_ldscript_parse_buffer(const unsigned char *buffer, size_t length);

#endif
