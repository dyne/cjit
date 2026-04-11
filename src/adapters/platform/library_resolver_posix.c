#include "adapters/platform/library_resolver_posix.h"

#include <stdbool.h>

#include "cjit.h"
#include "elflinker.h"

static CJITResult resolve_impl(void *context, const LibraryResolverRequest *request,
                               LibraryResolverResponse *response)
{
    CJITState *cjit = (CJITState *)context;
    int found;
    (void)request;
    found = posix_resolve_libs(cjit);
    response->resolved_count = found;
    response->resolved_paths = NULL;
    return cjit_result_ok();
}

const LibraryResolverPort posix_library_resolver_port = {
    .context = NULL,
    .resolve = resolve_impl
};
