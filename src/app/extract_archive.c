#include "app/extract_archive.h"

#include <stdint.h>

#include "adapters/fs/local_asset.h"
#include "cjit.h"

ExtractArchiveResponse extract_archive_route(const ExtractArchiveRequest *request)
{
    ExtractArchiveResponse response;
    AssetPort assets = local_asset_port;
    assets.context = NULL;
    if (!assets.extract_archive_to_path(assets.context, request->archive_path, ".").ok) {
        response.result.code = CJIT_RESULT_IO_ERROR;
        response.result.exit_status = 1;
        response.result.ok = false;
        response.result.message = "Failed to extract archive";
        return response;
    }
    response.result.code = CJIT_RESULT_OK;
    response.result.exit_status = 0;
    response.result.ok = true;
    response.result.message = NULL;
    return response;
}
