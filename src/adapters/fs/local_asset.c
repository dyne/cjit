#include "adapters/fs/local_asset.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "cjit.h"
#include "muntar.h"

static CJITResult make_result(CJITResultCode code, int exit_status, bool ok, const char *message)
{
    CJITResult result;
    result.code = code;
    result.exit_status = exit_status;
    result.ok = ok;
    result.message = message;
    return result;
}

static CJITResult extract_runtime_assets_impl(void *context, RuntimeSession *session,
                                              const char *destination_path, char **resolved_path)
{
    CJITState *cjit = (CJITState *)context;
    (void)session;
    if (!extract_assets(cjit, destination_path)) {
        return make_result(CJIT_RESULT_IO_ERROR, 1, false, "Failed to extract runtime assets");
    }
    if (resolved_path) {
        *resolved_path = cjit->tmpdir;
    }
    return make_result(CJIT_RESULT_OK, 0, true, NULL);
}

static CJITResult extract_archive_to_path_impl(void *context, const char *archive_path,
                                               const char *destination_path)
{
    unsigned int len = 0;
    const uint8_t *targz;
    (void)context;

    targz = (const uint8_t *)file_load(archive_path, &len);
    if (!targz || !len) {
        return make_result(CJIT_RESULT_IO_ERROR, 1, false, "Failed to load archive");
    }
    if (muntargz_to_path(destination_path, targz, len) != 0) {
        return make_result(CJIT_RESULT_IO_ERROR, 1, false, "Failed to extract archive");
    }
    return make_result(CJIT_RESULT_OK, 0, true, NULL);
}

const AssetPort local_asset_port = {
    .context = NULL,
    .extract_runtime_assets = extract_runtime_assets_impl,
    .extract_archive_to_path = extract_archive_to_path_impl
};
