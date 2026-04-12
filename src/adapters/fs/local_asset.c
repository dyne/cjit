#include "adapters/fs/local_asset.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "cjit.h"
#include "muntarfs.h"

static CJITResult extract_runtime_assets_impl(void *context, RuntimeSession *session,
                                              const char *destination_path, char **resolved_path)
{
    CJITState *cjit = (CJITState *)context;
    (void)session;
#if defined(SHAREDTCC)
    (void)cjit;
    (void)destination_path;
    (void)resolved_path;
    return cjit_result_error(CJIT_RESULT_INVALID_REQUEST, 1,
                             "Runtime assets are unavailable with shared libtcc builds");
#else
    if (!extract_assets(cjit, destination_path)) {
        return cjit_result_error(CJIT_RESULT_IO_ERROR, 1, "Failed to extract runtime assets");
    }
    if (resolved_path) {
        *resolved_path = cjit->tmpdir;
    }
    return cjit_result_ok();
#endif
}

static CJITResult extract_archive_to_path_impl(void *context, const char *archive_path,
                                               const char *destination_path)
{
    CJITResult result;
    unsigned int len = 0;
    const uint8_t *targz;
    (void)context;

    targz = (const uint8_t *)file_load(archive_path, &len);
    if (!targz || !len) {
        return cjit_result_error(CJIT_RESULT_IO_ERROR, 1, "Failed to load archive");
    }
    if (muntarfs_extract_targz_to_path(destination_path, targz, len) != 0) {
        free((void *)targz);
        return cjit_result_error(CJIT_RESULT_IO_ERROR, 1, "Failed to extract archive");
    }
    free((void *)targz);
    result = cjit_result_ok();
    return result;
}

const AssetPort local_asset_port = {
    .context = NULL,
    .extract_runtime_assets = extract_runtime_assets_impl,
    .extract_archive_to_path = extract_archive_to_path_impl
};
