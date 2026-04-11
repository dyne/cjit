#include "adapters/platform/library_resolver_posix.h"

#include <stdbool.h>

#include "cjit.h"
#include "elflinker.h"

static CJITResult make_result(CJITResultCode code, int exit_status, bool ok, const char *message)
{
    CJITResult result;
    result.code = code;
    result.exit_status = exit_status;
    result.ok = ok;
    result.message = message;
    return result;
}

static CJITResult resolve_impl(void *context, const LibraryResolverRequest *request,
                               LibraryResolverResponse *response)
{
    CJITState *cjit = (CJITState *)context;
    int found;
    (void)request;
    found = posix_resolve_libs(cjit);
    response->resolved_count = found;
    response->resolved_paths = NULL;
    return make_result(CJIT_RESULT_OK, 0, true, NULL);
}

const LibraryResolverPort posix_library_resolver_port = {
    .context = NULL,
    .resolve = resolve_impl
};
