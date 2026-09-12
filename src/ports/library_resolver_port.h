#ifndef CJIT_PORTS_LIBRARY_RESOLVER_PORT_H
#define CJIT_PORTS_LIBRARY_RESOLVER_PORT_H

#include "domain/error.h"

typedef struct StringList StringList;

/**
 * Input collection for library resolution.
 */
typedef struct LibraryResolverRequest {
    int library_count;
    const StringList *libraries;
    int search_path_count;
    const StringList *search_paths;
} LibraryResolverRequest;

/**
 * Output collection for resolved libraries.
 */
typedef struct LibraryResolverResponse {
    int resolved_count;
    /* Borrowed from the resolver context; the context owns the list. */
    const StringList *resolved_paths;
} LibraryResolverResponse;

/**
 * Platform-specific shared library resolution port.
 */
typedef struct LibraryResolverPort {
    void *context;
    CJITResult (*resolve)(void *context, const LibraryResolverRequest *request,
                          LibraryResolverResponse *response);
} LibraryResolverPort;

#endif
