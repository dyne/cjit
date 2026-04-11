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
    if (!extract_assets(cjit, destination_path)) {
        return cjit_result_error(CJIT_RESULT_IO_ERROR, 1, "Failed to extract runtime assets");
    }
    if (resolved_path) {
        *resolved_path = cjit->tmpdir;
    }
    return cjit_result_ok();
}

static CJITResult extract_archive_to_path_impl(void *context, const char *archive_path,
                                               const char *destination_path)
{
    unsigned int len = 0;
    const uint8_t *targz;
    (void)context;

    targz = (const uint8_t *)file_load(archive_path, &len);
    if (!targz || !len) {
        return cjit_result_error(CJIT_RESULT_IO_ERROR, 1, "Failed to load archive");
    }
    if (muntarfs_extract_targz_to_path(destination_path, targz, len) != 0) {
        return cjit_result_error(CJIT_RESULT_IO_ERROR, 1, "Failed to extract archive");
    }
    return cjit_result_ok();
}

const AssetPort local_asset_port = {
    .context = NULL,
    .extract_runtime_assets = extract_runtime_assets_impl,
    .extract_archive_to_path = extract_archive_to_path_impl
};
