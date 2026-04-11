#include "app/extract_assets.h"

#include "adapters/fs/local_asset.h"

ExtractAssetsResponse extract_assets_route(CJITState *cjit, const ExtractAssetsRequest *request)
{
    ExtractAssetsResponse response;
    RuntimeSession session;
    AssetPort assets = local_asset_port;
    char *resolved_path = NULL;
    assets.context = cjit;
    session.compiler_handle = cjit->TCC;
    session.tmpdir = cjit->tmpdir;
    session.tempdir_is_fresh = cjit->fresh;
    session.setup_complete = cjit->done_setup;
    session.execution_complete = cjit->done_exec;
    if (!assets.extract_runtime_assets(assets.context, &session,
                                       request->destination_path, &resolved_path).ok) {
        response.result = cjit_result_error(CJIT_RESULT_IO_ERROR, 1,
                                            "Failed to extract runtime assets");
        response.destination_path = NULL;
        return response;
    }
    response.result = cjit_result_ok();
    response.destination_path = resolved_path;
    return response;
}
