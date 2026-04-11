#include "adapters/platform/library_resolver_windows.h"

#include <stdbool.h>

#include "cjit.h"

#if defined(WINDOWS)
int windows_resolve_libs(CJITState *cjit);
#endif

static CJITResult resolve_impl(void *context, const LibraryResolverRequest *request,
                               LibraryResolverResponse *response)
{
#if defined(WINDOWS)
    CJITState *cjit = (CJITState *)context;
    int found;
    (void)request;
    found = windows_resolve_libs(cjit);
    response->resolved_count = found;
    response->resolved_paths = NULL;
    return cjit_result_ok();
#else
    (void)context;
    (void)request;
    response->resolved_count = 0;
    response->resolved_paths = NULL;
    return cjit_result_error(CJIT_RESULT_PLATFORM_ERROR, 1,
                             "Windows library resolver unavailable on this platform");
#endif
}

const LibraryResolverPort windows_library_resolver_port = {
    .context = NULL,
    .resolve = resolve_impl
};
