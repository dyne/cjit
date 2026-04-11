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
        response.result = cjit_result_error(CJIT_RESULT_IO_ERROR, 1,
                                            "Failed to extract archive");
        return response;
    }
    response.result = cjit_result_ok();
    return response;
}
