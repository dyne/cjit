#ifndef CJIT_PORTS_LIBRARY_RESOLVER_PORT_H
#define CJIT_PORTS_LIBRARY_RESOLVER_PORT_H

#include "domain/error.h"

/**
 * Input collection for library resolution.
 */
typedef struct LibraryResolverRequest {
    int library_count;
    const char **libraries;
    int search_path_count;
    const char **search_paths;
} LibraryResolverRequest;

/**
 * Output collection for resolved libraries.
 */
typedef struct LibraryResolverResponse {
    int resolved_count;
    const char **resolved_paths;
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
