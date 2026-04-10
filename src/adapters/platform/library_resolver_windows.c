#include "adapters/platform/library_resolver_windows.h"

#include <stdbool.h>

#include "cjit.h"
#include "array.h"

#if defined(WINDOWS)
int windows_resolve_libs(CJITState *cjit);
#endif

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
#if defined(WINDOWS)
    CJITState *cjit = (CJITState *)context;
    int found;
    (void)request;
    found = windows_resolve_libs(cjit);
    response->resolved_count = found;
    response->resolved_paths = (const char **)XArray_GetData((xarray_t *)cjit->reallibs, 0);
    return make_result(CJIT_RESULT_OK, 0, true, NULL);
#else
    (void)context;
    (void)request;
    response->resolved_count = 0;
    response->resolved_paths = NULL;
    return make_result(CJIT_RESULT_PLATFORM_ERROR, 1, false,
                       "Windows library resolver unavailable on this platform");
#endif
}

const LibraryResolverPort windows_library_resolver_port = {
    .context = NULL,
    .resolve = resolve_impl
};
